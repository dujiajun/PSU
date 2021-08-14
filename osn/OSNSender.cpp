#include "OSNSender.h"
#include "libOTe/Base/BaseOT.h"
#include "cryptoTools/Common/BitVector.h"
#include <cryptoTools/Crypto/AES.h>
#include <libOTe/TwoChooseOne/SilentOtExtReceiver.h>
#include <libOTe/TwoChooseOne/IknpOtExtReceiver.h>
#include "benes.h"
#include <iterator>

using namespace std;
using namespace osuCrypto;

std::vector<std::array<osuCrypto::block, 2>> OSNSender::gen_benes_server_osn(int values, std::vector<oc::Channel>& chls)
{
	osuCrypto::BitVector switches = benes.return_gen_benes_switches(values);

	std::vector<std::array<osuCrypto::block, 2>> recvMsg(switches.size());
	std::vector<std::array<osuCrypto::block, 2>> recvCorr(switches.size());
	Channel& chl = chls[0];
	if (ot_type == 0)
	{
		std::vector<osuCrypto::block> tmpMsg(switches.size());
		osuCrypto::BitVector choices(switches.size());

		silent_ot_recv(choices, tmpMsg, chls);
		AES aes(ZeroBlock);

		for (auto i = 0; i < recvMsg.size(); i++)
		{
			recvMsg[i] = { tmpMsg[i], aes.ecbEncBlock(tmpMsg[i]) };
		}
		osuCrypto::BitVector bit_correction = switches ^ choices;
		chl.send(bit_correction);
	}
	else
	{
		std::vector<osuCrypto::block> tmpMsg(switches.size());
		rand_ot_recv(switches, tmpMsg, chls);
		AES aes(ZeroBlock);
		for (auto i = 0; i < recvMsg.size(); i++)
			recvMsg[i] = { tmpMsg[i], aes.ecbEncBlock(tmpMsg[i]) };
	}
	chl.recv(recvCorr.data(), recvCorr.size());
	block temp_msg[2], temp_corr[2];
	for (int i = 0; i < recvMsg.size(); i++)
	{
		if (switches[i] == 1)
		{
			temp_msg[0] = recvCorr[i][0] ^ recvMsg[i][0];
			temp_msg[1] = recvCorr[i][1] ^ recvMsg[i][1];
			recvMsg[i] = { temp_msg[0], temp_msg[1] };
		}
	}
	return recvMsg;
}

std::vector<oc::block> OSNSender::run_osn(std::vector<oc::Channel>& chls)
{
	int values = size;
	int N = int(ceil(log2(values)));
	int levels = 2 * N - 1;

	std::vector<std::array<osuCrypto::block, 2>> ot_output = gen_benes_server_osn(values, chls);

	std::vector<block> input_vec(values);
	chls[0].recv(input_vec.data(), input_vec.size());

	std::vector<std::vector<std::array<osuCrypto::block, 2>>> matrix_ot_output(
		levels, std::vector<std::array<osuCrypto::block, 2>>(values));
	int ctr = 0;
	for (int i = 0; i < levels; ++i)
	{
		for (int j = 0; j < values / 2; ++j)
			matrix_ot_output[i][j] = ot_output[ctr++];
	}

	benes.gen_benes_masked_evaluate(N, 0, 0, input_vec, matrix_ot_output);
	return input_vec; //share
}

void OSNSender::setTimer(Timer& timer)
{
	this->timer = &timer;
}

void OSNSender::silent_ot_recv(osuCrypto::BitVector& choices,
	std::vector<osuCrypto::block>& recvMsg,
	std::vector<oc::Channel>& chls)
{
	//std::cout << "\n Silent OT receiver!!\n";
	size_t num_threads = chls.size();
	size_t total_len = choices.size();
	vector<BitVector> tmpChoices(num_threads);
	auto routine = [&](size_t tid)
	{
		size_t start_idx = total_len * tid / num_threads;
		size_t end_idx = total_len * (tid + 1) / num_threads;
		end_idx = ((end_idx <= total_len) ? end_idx : total_len);
		size_t size = end_idx - start_idx;

		osuCrypto::PRNG prng0(_mm_set_epi32(4253465, 3434565, 234435, 23987045));
		osuCrypto::u64 numOTs = size;

		osuCrypto::SilentOtExtReceiver recv;
		recv.configure(numOTs);

		tmpChoices[tid].copy(choices, start_idx, size);
		std::vector<oc::block> tmpMsg(size);
		recv.silentReceive(tmpChoices[tid], tmpMsg, prng0, chls[tid]);

		std::copy_n(tmpMsg.begin(), size, recvMsg.begin() + start_idx);
	};
	vector<thread> thrds(num_threads);
	for (size_t t = 0; t < num_threads; t++)
		thrds[t] = std::thread(routine, t);
	for (size_t t = 0; t < num_threads; t++)
		thrds[t].join();
	choices.resize(0);
	for (size_t t = 0; t < num_threads; t++)
		choices.append(tmpChoices[t]);

	/*osuCrypto::PRNG prng0(_mm_set_epi32(4253465, 3434565, 234435, 23987045));
	osuCrypto::u64 numOTs = choices.size();

	osuCrypto::SilentOtExtReceiver recv;
	recv.configure(numOTs, 2, num_threads);

	recv.silentReceive(choices, recvMsg, prng0, chls[0]);*/
}

void OSNSender::rand_ot_recv(osuCrypto::BitVector& choices,
	std::vector<osuCrypto::block>& recvMsg,
	std::vector<oc::Channel>& chls)
{
	//std::cout << "\n Ot receiver!!\n";

	size_t num_threads = chls.size();
	size_t total_len = choices.size();
	vector<BitVector> tmpChoices(num_threads);

	auto routine = [&](size_t tid)
	{

		size_t start_idx = total_len * tid / num_threads;
		size_t end_idx = total_len * (tid + 1) / num_threads;
		end_idx = ((end_idx <= total_len) ? end_idx : total_len);
		size_t size = end_idx - start_idx;

		osuCrypto::PRNG prng0(_mm_set_epi32(4253465, 3434565, 234435, 23987045));
		osuCrypto::u64 numOTs = size; // input.length();
		std::vector<osuCrypto::block> baseRecv(128);
		std::vector<std::array<osuCrypto::block, 2>> baseSend(128);
		osuCrypto::BitVector baseChoice(128);

		prng0.get((osuCrypto::u8*)baseSend.data()->data(), sizeof(osuCrypto::block) * 2 * baseSend.size());

		osuCrypto::DefaultBaseOT baseOTs;
		baseOTs.send(baseSend, prng0, chls[tid], 1);

		osuCrypto::IknpOtExtReceiver recv;
		recv.setBaseOts(baseSend);

		tmpChoices[tid].copy(choices, start_idx, size);
		std::vector<oc::block> tmpMsg(size);

		recv.receive(tmpChoices[tid], tmpMsg, prng0, chls[tid]);
		std::copy_n(tmpMsg.begin(), size, recvMsg.begin() + start_idx);
	};
	vector<thread> thrds(num_threads);
	for (size_t t = 0; t < num_threads; t++)
		thrds[t] = std::thread(routine, t);
	for (size_t t = 0; t < num_threads; t++)
		thrds[t].join();
	choices.resize(0);
	for (size_t t = 0; t < num_threads; t++)
		choices.append(tmpChoices[t]);

	/*osuCrypto::PRNG prng0(_mm_set_epi32(4253465, 3434565, 234435, 23987045));
	osuCrypto::u64 numOTs = choices.size(); // input.length();
	std::vector<osuCrypto::block> baseRecv(128);
	std::vector<std::array<osuCrypto::block, 2>> baseSend(128);
	osuCrypto::BitVector baseChoice(128);

	prng0.get((osuCrypto::u8*)baseSend.data()->data(), sizeof(osuCrypto::block) * 2 * baseSend.size());

	osuCrypto::DefaultBaseOT baseOTs;
	baseOTs.send(baseSend, prng0, chls[0], num_threads);

	osuCrypto::IknpOtExtReceiver recv;
	recv.setBaseOts(baseSend);

	recv.receive(choices, recvMsg, prng0, chls[0]);*/
}

OSNSender::OSNSender(size_t size, int ot_type) : size(size), ot_type(ot_type)
{

}

void OSNSender::init(size_t size, int ot_type, const string& osn_cache)
{
	this->size = size;
	this->ot_type = ot_type;

	int values = size;
	int N = int(ceil(log2(values)));
	int levels = 2 * N - 1;

	dest.resize(size);
	benes.initialize(values, levels);

	std::vector<int> src(values);
	for (int i = 0; i < src.size(); ++i)
		src[i] = dest[i] = i;

	osuCrypto::PRNG prng(_mm_set_epi32(4253233465, 334565, 0, 235)); // we need to modify this seed

	for (int i = size - 1; i > 0; i--)
	{
		int loc = prng.get<uint64_t>() % (i + 1); //  pick random location in the array
		std::swap(dest[i], dest[loc]);
	}
	if (osn_cache != "")
	{
		string file = osn_cache + "_" + to_string(size);
		if (!benes.load(file))
		{
			cout << "OSNSender is generating osn cache!" << endl;
			benes.gen_benes_route(N, 0, 0, src, dest);
			benes.dump(file);
		}
		else 
		{
			cout << "OSNSender is using osn cache!" << endl;
		}
	}
	else 
	{
		benes.gen_benes_route(N, 0, 0, src, dest);
	}
	

}