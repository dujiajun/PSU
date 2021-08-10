#include "MPOPRFReceiver.h"
#include <cryptoTools/Common/Log.h>

using namespace std;
using namespace osuCrypto;

void osuCrypto::MPOPRFReceiver::setTimer(Timer& timer)
{
	this->timer = &timer;
}

osuCrypto::MPOPRFReceiver::MPOPRFReceiver()
{
}

void osuCrypto::MPOPRFReceiver::setParams(const block& commonSeed, const u64& set_size, const u64& logHeight, const u64& width, const u64& hashLengthInBytes, const u64& h1LengthInBytes, const u64& bucket1, const u64& bucket2)
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

MPOPRFReceiver::MPOPRFReceiver(const block& commonSeed, const u64& set_size, const u64& logHeight, const u64& width, const u64& hashLengthInBytes, const u64& h1LengthInBytes, const u64& bucket1, const u64& bucket2)
	: commonSeed(commonSeed), set_size(set_size), logHeight(logHeight), width(width), hashLengthInBytes(hashLengthInBytes), h1LengthInBytes(h1LengthInBytes), bucket1(bucket1), bucket2(bucket2)
{
	height = 1ull << logHeight;
}

u8* MPOPRFReceiver::run(PRNG& prng, std::vector<Channel>& chls, const std::vector<block>& receiverSet)
{
	auto heightInBytes = (height + 7) / 8;
	auto widthInBytes = (width + 7) / 8;
	auto locationInBytes = (logHeight + 7) / 8;
	auto receiverSizeInBytes = (set_size + 7) / 8;
	auto shift = (1 << logHeight) - 1;
	auto widthBucket1 = sizeof(block) / locationInBytes;

	///////////////////// Base OTs ///////////////////////////

	IknpOtExtSender otExtSender;
	otExtSender.genBaseOts(prng, chls[0]);
	std::vector<std::array<block, 2>> otMessages(width);
	otExtSender.send(otMessages, prng, chls[0]);

	//////////// Initialization ///////////////////

	PRNG commonPrng(commonSeed);
	vector<vector<u8>> transHashInputs(width, vector<u8>(receiverSizeInBytes, 0));

	/////////// Transform input /////////////////////

	block commonKey = commonPrng.get<block>();

	vector<block> recvSet(set_size);
	vector<block> aesInput(set_size);
	vector<block> aesOutput(set_size);

	size_t num_threads = chls.size();
	vector<thread> thrds(num_threads);

	auto routine1 = [&](size_t tid)
	{
		AES aes(commonKey);
		size_t start_idx = set_size * tid / num_threads;
		size_t end_idx = set_size * (tid + 1) / num_threads;
		end_idx = end_idx > set_size ? set_size : end_idx;

		RandomOracle H1(h1LengthInBytes);
		vector<u8> h1Output(h1LengthInBytes);
		for (auto i = start_idx; i < end_idx; ++i)
		{
			H1.Reset();
			H1.Update((u8*)(receiverSet.data() + i), sizeof(block));
			H1.Final(h1Output.data());

			aesInput[i] = *(block*)h1Output.data();
			recvSet[i] = *(block*)(h1Output.data() + sizeof(block));
		}
		aes.ecbEncBlocks(aesInput.data() + start_idx, end_idx - start_idx, aesOutput.data() + start_idx);
		for (u64 i = start_idx; i < end_idx; ++i)
		{
			recvSet[i] = _mm_xor_si128(recvSet[i], aesOutput[i]);
		}
	};
	for (size_t i = 0; i < num_threads; i++)
		thrds[i] = thread(routine1, i);
	for (size_t i = 0; i < num_threads; i++)
		thrds[i].join();
	
	vector<block> commonKeys((width / widthBucket1) + 1);
	for (auto wLeft = 0; wLeft < width; wLeft += widthBucket1)
	{
		commonKeys[wLeft / widthBucket1] = commonPrng.get<block>();
	}
	// std::cout << "Receiver set transformed\n";
	//timer->setTimePoint("Receiver set transformed");

	size_t num_buckets = ceil(1.0 * width / widthBucket1);

	auto routine2 = [&](size_t tid)
	{
		AES aes;
		size_t start_idx = num_buckets * tid / num_threads * widthBucket1;
		size_t end_idx = num_buckets * (tid + 1) / num_threads * widthBucket1;
		end_idx = end_idx > width ? width : end_idx;

		vector<block> randomLocations(bucket1);
		vector<vector<u8>> matrixA(widthBucket1, vector<u8>(heightInBytes));
		vector<vector<u8>> matrixDelta(widthBucket1, vector<u8>(heightInBytes));
		vector<vector<u8>> transLocations(widthBucket1, vector<u8>(set_size * locationInBytes + sizeof(u32)));

		for (auto wLeft = start_idx; wLeft < end_idx; wLeft += widthBucket1)
		{
			auto wRight = wLeft + widthBucket1 < end_idx ? wLeft + widthBucket1 : end_idx;
			auto w = wRight - wLeft;

			//////////// Compute random locations (transposed) ////////////////

			//commonPrng.get((u8*)&commonKey, sizeof(block));
			aes.setKey(commonKeys[wLeft / widthBucket1]);

			for (auto low = 0; low < set_size; low += bucket1)
			{
				auto up = low + bucket1 < set_size ? low + bucket1 : set_size;
				aes.ecbEncBlocks(recvSet.data() + low, up - low, randomLocations.data());
				for (u64 i = 0; i < w; ++i)
				{
					for (u64 j = low; j < up; ++j)
					{
						memcpy(transLocations[i].data() + j * locationInBytes, (u8*)(randomLocations.data() + (j - low)) + i * locationInBytes, locationInBytes);
					}
				}
			}
			//////////// Compute matrix Delta /////////////////////////////////

			for (u64 i = 0; i < widthBucket1; ++i)
			{
				memset(matrixDelta[i].data(), 255, heightInBytes);
			}

			for (u64 i = 0; i < w; ++i)
			{
				for (u64 j = 0; j < set_size; ++j)
				{
					auto location = (*(u32*)(transLocations[i].data() + j * locationInBytes)) & shift;
					matrixDelta[i][location >> 3] &= ~(1 << (location & 7));
				}
			}

			//////////////// Compute matrix A & sent matrix ///////////////////////

			vector<u8> sentMatrix(w * heightInBytes);
			for (auto i = 0; i < w; ++i)
			{
				PRNG prng(otMessages[i + wLeft][0]);
				prng.get(matrixA[i].data(), heightInBytes);

				prng.SetSeed(otMessages[i + wLeft][1]);
				prng.get(sentMatrix.data() + heightInBytes * i, heightInBytes);

				for (auto j = 0; j < heightInBytes; ++j)
				{
					sentMatrix[i * heightInBytes + j] ^= matrixA[i][j] ^ matrixDelta[i][j];
				}
			}
			chls[tid].asyncSend(std::move(sentMatrix));

			///////////////// Compute hash inputs (transposed) /////////////////////

			for (u64 i = 0; i < w; ++i)
			{
				for (u64 j = 0; j < set_size; ++j)
				{
					auto location = (*(u32*)(transLocations[i].data() + j * locationInBytes)) & shift;

					transHashInputs[i + wLeft][j >> 3] |= (u8)((bool)(matrixA[i][location >> 3] & (1 << (location & 7)))) << (j & 7);
				}
			}
		}
	};
	for (size_t i = 0; i < num_threads; i++)
		thrds[i] = thread(routine2, i);
	for (size_t i = 0; i < num_threads; i++)
		thrds[i].join();
	//timer->setTimePoint("after Compute matrix");
	
	/////////////////// Compute hash outputs ///////////////////////////

	u8* results = new u8[hashLengthInBytes * set_size];

	auto routine = [&](size_t tid)
	{
		size_t start_idx = set_size * tid / num_threads;
		size_t end_idx = set_size * (tid + 1) / num_threads;
		end_idx = end_idx > set_size ? set_size : end_idx;

		RandomOracle H(hashLengthInBytes);
		u8 hashOutput[sizeof(block)];
		vector<vector<u8>> hashInputs(bucket2, vector<u8>(widthInBytes));

		for (auto low = start_idx; low < end_idx; low += bucket2)
		{
			auto up = low + bucket2 < end_idx ? low + bucket2 : end_idx;

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
				memcpy(results + j * hashLengthInBytes, hashOutput, hashLengthInBytes);
			}
		}

	};


	for (size_t i = 0; i < num_threads; i++)
		thrds[i] = thread(routine, i);
	for (size_t i = 0; i < num_threads; i++)
		thrds[i].join();
	//timer->setTimePoint("Compute hash outputs");

	return results;

}
