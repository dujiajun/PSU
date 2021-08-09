#include "MPOPRFSender.h"
#include <cryptoTools/Common/Log.h>

using namespace std;
using namespace osuCrypto;

void osuCrypto::MPOPRFSender::setTimer(Timer& timer)
{
	this->timer = &timer;
}

void osuCrypto::MPOPRFSender::run(PRNG& prng, std::vector<Channel>& chls)
{
	//timer.setTimePoint("Sender start");

	auto heightInBytes = (height + 7) / 8;
	auto widthInBytes = (width + 7) / 8;
	auto locationInBytes = (logHeight + 7) / 8;
	auto senderSizeInBytes = (set_size + 7) / 8;
	auto shift = (1 << logHeight) - 1;
	auto widthBucket1 = sizeof(block) / locationInBytes;

	//////////////////// Base OTs /////////////////////////////////

	IknpOtExtReceiver otExtReceiver;
	otExtReceiver.genBaseOts(prng, chls[0]);
	BitVector choices(width);
	std::vector<block> otMessages(width);
	prng.get(choices.data(), choices.sizeBytes());
	otExtReceiver.receive(choices, otMessages, prng, chls[0]);
	////////////// Initialization //////////////////////

	matrixC.resize(width);
	for (auto i = 0; i < width; i++)
		matrixC[i].resize(heightInBytes);

	int flag = 0;
	size_t num_threads = chls.size();
	vector<thread> thrds(num_threads);

	size_t num_buckets = width / widthBucket1;

	auto routine = [&](size_t tid)
	{
		size_t start_idx = num_buckets * tid / num_threads * width;
		size_t end_idx = num_buckets * (tid + 1) / num_threads * width;
		end_idx = end_idx > width ? width : end_idx;

		for (auto wLeft = start_idx; wLeft < end_idx; wLeft += widthBucket1)
		{
			auto wRight = wLeft + widthBucket1 < end_idx ? wLeft + widthBucket1 : end_idx;
			auto w = wRight - wLeft;

			//////////////// Extend OTs and compute matrix C ///////////////////

			std::vector<u8> recvMatrix(w * heightInBytes);
			chls[tid].recv(recvMatrix);
			for (auto i = 0; i < w; ++i)
			{
				PRNG prng(otMessages[i + wLeft]);
				prng.get(matrixC[i + wLeft].data(), heightInBytes);

				if (choices[i + wLeft])
				{
					for (auto j = 0; j < heightInBytes; ++j)
					{
						matrixC[i + wLeft][j] ^= recvMatrix[i * heightInBytes + j];
					}
				}
			}

		}
	};
	for (size_t i = 0; i < num_threads; i++)
		thrds[i] = thread(routine, i);
	for (size_t i = 0; i < num_threads; i++)
		thrds[i].join();
}

osuCrypto::MPOPRFSender::MPOPRFSender()
{
}

void osuCrypto::MPOPRFSender::setParams(const block& commonSeed, const u64& set_size, const u64& logHeight, const u64& width, const u64& hashLengthInBytes, const u64& h1LengthInBytes, const u64& bucket1, const u64& bucket2)
{
	this->commonSeed = commonSeed;
	this->set_size = set_size;
	this->logHeight = logHeight;
	this->width = width;
	this->hashLengthInBytes = hashLengthInBytes;
	this->h1LengthInBytes = h1LengthInBytes;
	this->bucket1 = bucket1;
	this->bucket2 = bucket2;
	height = 1ull << logHeight;
}

osuCrypto::MPOPRFSender::MPOPRFSender(const block& commonSeed, const u64& set_size, const u64& logHeight, const u64& width, const u64& hashLengthInBytes, const u64& h1LengthInBytes, const u64& bucket1, const u64& bucket2)
	: commonSeed(commonSeed), set_size(set_size), logHeight(logHeight), width(width), hashLengthInBytes(hashLengthInBytes), h1LengthInBytes(h1LengthInBytes), bucket1(bucket1), bucket2(bucket2)
{
	height = 1ull << logHeight;
}

vector<u8> osuCrypto::MPOPRFSender::get_oprf(std::vector<block>& senderSet)
{
	PRNG commonPrng(commonSeed);
	auto set_size = senderSet.size();
	auto heightInBytes = (height + 7) / 8;
	auto widthInBytes = (width + 7) / 8;
	auto locationInBytes = (logHeight + 7) / 8;
	auto senderSizeInBytes = (set_size + 7) / 8;
	auto shift = (1 << logHeight) - 1;
	auto widthBucket1 = sizeof(block) / locationInBytes;

	vector<vector<u8>> transLocations(widthBucket1, vector<u8>(set_size * locationInBytes + sizeof(u32)));
	vector<block> randomLocations(bucket1);
	vector<vector<u8>> transHashInputs(width, vector<u8>(senderSizeInBytes, 0));

	/////////// Transform input /////////////////////
	block commonKey;
	commonPrng.get((u8*)&commonKey, sizeof(block));
	AES commonAes(commonKey);

	vector<block> sendSet(set_size);
	vector<block> aesInput(set_size);
	vector<block> aesOutput(set_size);

	RandomOracle H1(h1LengthInBytes);
	vector<u8> h1Output(h1LengthInBytes);

	for (auto i = 0; i < set_size; ++i)
	{
		H1.Reset();
		H1.Update((u8*)(senderSet.data() + i), sizeof(block));
		H1.Final(h1Output.data());

		aesInput[i] = *(block*)h1Output.data();
		sendSet[i] = *(block*)(h1Output.data() + sizeof(block));
	}

	commonAes.ecbEncBlocks(aesInput.data(), set_size, aesOutput.data());
	for (u64 i = 0; i < set_size; ++i)
	{
		sendSet[i] = _mm_xor_si128(sendSet[i], aesOutput[i]);
	}

	//std::cout << "Sender set transformed\n";
	//timer.setTimePoint("Sender set transformed");

	for (auto wLeft = 0; wLeft < width; wLeft += widthBucket1)
	{
		auto wRight = wLeft + widthBucket1 < width ? wLeft + widthBucket1 : width;
		auto w = wRight - wLeft;

		//////////// Compute random locations (transposed) ////////////////

		commonPrng.get((u8*)&commonKey, sizeof(block));
		commonAes.setKey(commonKey);
		for (auto low = 0; low < set_size; low += bucket1)
		{

			auto up = low + bucket1 < set_size ? low + bucket1 : set_size;

			commonAes.ecbEncBlocks(sendSet.data() + low, up - low, randomLocations.data());

			for (u64 i = 0; i < w; ++i)
			{
				for (u64 j = low; j < up; ++j)
				{
					memcpy(transLocations[i].data() + j * locationInBytes, (u8*)(randomLocations.data() + (j - low)) + i * locationInBytes, locationInBytes);
				}
			}
		}

		///////////////// Compute hash inputs (transposed) /////////////////////

		for (u64 i = 0; i < w; ++i)
		{
			for (u64 j = 0; j < set_size; ++j)
			{
				auto location = (*(u32*)(transLocations[i].data() + j * locationInBytes)) & shift;
				transHashInputs[i + wLeft][j >> 3] |= (u8)((bool)(matrixC[i + wLeft][location >> 3] & (1 << (location & 7)))) << (j & 7);
			}
		}
	}

	/////////////////// Compute hash outputs ///////////////////////////

	RandomOracle H(hashLengthInBytes);
	u8 hashOutput[sizeof(block)];

	vector<u8> results(hashLengthInBytes * set_size);
	vector<vector<u8>> hashInputs(bucket2, vector<u8>(widthInBytes));

	for (auto low = 0; low < set_size; low += bucket2)
	{
		auto up = low + bucket2 < set_size ? low + bucket2 : set_size;

		for (u64 j = low; j < up; ++j)
		{
			memset(hashInputs[j - low].data(), 0, widthInBytes);
		}

		for (auto i = 0; i < width; ++i)
		{
			for (auto j = low; j < up; ++j)
			{
				hashInputs[j - low][i >> 3] |= (u8)((bool)(transHashInputs[i][j >> 3] & (1 << (j & 7)))) << (i & 7);
			}
		}

		for (auto j = low; j < up; ++j)
		{
			H.Reset();
			H.Update(hashInputs[j - low].data(), widthInBytes);
			H.Final(hashOutput);
			memcpy(results.data() + j * hashLengthInBytes, hashOutput, hashLengthInBytes);
		}
	}
	return results;
}

osuCrypto::MPOPRFSender::~MPOPRFSender()
{
}
