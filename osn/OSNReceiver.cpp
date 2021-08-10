#include "OSNReceiver.h"
#include "libOTe/Base/BaseOT.h"
#include "cryptoTools/Common/BitVector.h"
#include <cryptoTools/Crypto/AES.h>
#include <libOTe/TwoChooseOne/SilentOtExtSender.h>
#include <libOTe/TwoChooseOne/IknpOtExtSender.h>
#include <iterator>
using namespace std;
using namespace oc;
void OSNReceiver::rand_ot_send(std::vector<std::array<osuCrypto::block, 2>>& sendMsg, std::vector<oc::Channel>& chls)
{
	//std::cout << "\n OT sender!! \n";

	size_t num_threads = chls.size();
	size_t total_len = sendMsg.size();
	auto routine = [&](size_t tid)
	{
	  size_t start_idx = total_len * tid / num_threads;
	  size_t end_idx = total_len * (tid + 1) / num_threads;
	  end_idx = ((end_idx <= total_len) ? end_idx : total_len);
	  size_t size = end_idx - start_idx;

	  osuCrypto::PRNG prng1(_mm_set_epi32(4253233465, 334565, 0, 235));

	  std::vector<osuCrypto::block> baseRecv(128);
	  osuCrypto::DefaultBaseOT baseOTs;
	  osuCrypto::BitVector baseChoice(128);
	  baseChoice.randomize(prng1);
	  osuCrypto::IknpOtExtSender sender;
	  baseOTs.receive(baseChoice, baseRecv, prng1, chls[tid], 1);
	  sender.setBaseOts(baseRecv, baseChoice);

	  std::vector<std::array<osuCrypto::block, 2>> tmpMsg(size);
	  sender.send(tmpMsg, prng1, chls[tid]);
	  std::copy_n(tmpMsg.begin(), size, sendMsg.begin() + start_idx);
	};
	vector<thread> thrds(num_threads);
	for (size_t t = 0; t < num_threads; t++)
		thrds[t] = std::thread(routine, t);
	for (size_t t = 0; t < num_threads; t++)
		thrds[t].join();
	
	/*osuCrypto::PRNG prng1(_mm_set_epi32(4253233465, 334565, 0, 235));

	std::vector<osuCrypto::block> baseRecv(128);
	osuCrypto::DefaultBaseOT baseOTs;
	osuCrypto::BitVector baseChoice(128);
	baseChoice.randomize(prng1);
	osuCrypto::IknpOtExtSender sender;
	baseOTs.receive(baseChoice, baseRecv, prng1, chls[0], num_threads);
	sender.setBaseOts(baseRecv, baseChoice);
	sender.send(sendMsg, prng1, chls[0]);*/
}

void OSNReceiver::silent_ot_send(std::vector<std::array<osuCrypto::block, 2>>& sendMsg, std::vector<oc::Channel>& chls)
{
	//std::cout << "\n Silent OT sender!! \n";

	size_t num_threads = chls.size();
	size_t total_len = sendMsg.size();
	auto routine = [&](size_t tid)
	{
	  size_t start_idx = total_len * tid / num_threads;
	  size_t end_idx = total_len * (tid + 1) / num_threads;
	  end_idx = ((end_idx <= total_len) ? end_idx : total_len);
	  size_t size = end_idx - start_idx;

	  osuCrypto::PRNG prng1(_mm_set_epi32(4253233465, 334565, 0, 235));
	  osuCrypto::u64 numOTs = size;

	  osuCrypto::SilentOtExtSender sender;
	  sender.configure(numOTs);
	  std::vector<std::array<osuCrypto::block, 2>> tmpMsg(size);
	  sender.silentSend(tmpMsg, prng1, chls[tid]);
	  std::copy_n(tmpMsg.begin(), size, sendMsg.begin() + start_idx);
	};

	vector<thread> thrds(num_threads);
	for (size_t t = 0; t < num_threads; t++)
		thrds[t] = std::thread(routine, t);
	for (size_t t = 0; t < num_threads; t++)
		thrds[t].join();

	/*osuCrypto::PRNG prng1(_mm_set_epi32(4253233465, 334565, 0, 235));
	osuCrypto::u64 numOTs = sendMsg.size();

	osuCrypto::SilentOtExtSender sender;
	sender.configure(numOTs, 2, num_threads);
	sender.silentSend(sendMsg, prng1, chls[0]);*/
}

std::vector<std::vector<block>> OSNReceiver::gen_benes_client_osn(int values, std::vector<oc::Channel>& chls)
{
	int N = int(ceil(log2(values)));

	int levels = 2 * N - 1;
	int switches = levels * (values / 2);
	block temp;
	std::vector<block> masks(values);
	std::vector<std::vector<block>> ret_masks(values);

	osuCrypto::PRNG prng(_mm_set_epi32(4253233465, 334565, 0, 235));

	for (int j = 0; j < values; j++)
	{ // we sample the input masks randomly
		temp = prng.get<block>();
		masks[j] = temp;
		ret_masks[j].push_back(temp);
	}
	//timer->setTimePoint("after sample masks");
	std::vector<std::array<std::array<osuCrypto::block, 2>, 2>> ot_messages(switches);
	Channel& chl = chls[0];
	if (ot_type == 0)
	{
		std::vector<std::array<osuCrypto::block, 2>> tmp_messages(switches);
		osuCrypto::BitVector bit_correction(switches);
		silent_ot_send(tmp_messages, chls); // sample random ot blocks
		//timer->setTimePoint("after silent_ot_send");
		chl.recv(bit_correction);
		osuCrypto::block tmp;
		for (int k = 0; k < tmp_messages.size(); k++)
		{
			if (bit_correction[k] == 1)
			{
				tmp = tmp_messages[k][0];
				tmp_messages[k][0] = tmp_messages[k][1];
				tmp_messages[k][1] = tmp;
			}
		}
		//timer->setTimePoint("after bit correction");
		AES aes(ZeroBlock);

		for (auto i = 0; i < ot_messages.size(); i++)
		{
			ot_messages[i][0] = { tmp_messages[i][0], aes.ecbEncBlock(tmp_messages[i][0]) };
			ot_messages[i][1] = { tmp_messages[i][1], aes.ecbEncBlock(tmp_messages[i][1]) };
		}
		//timer->setTimePoint("after aes");
	}
	else
	{
		std::vector<std::array<osuCrypto::block, 2>> tmp_messages(switches);
		rand_ot_send(tmp_messages, chls); // sample random ot blocks
		//timer->setTimePoint("after rand_ot_send");
		AES aes(ZeroBlock);
		for (auto i = 0; i < ot_messages.size(); i++)
		{
			ot_messages[i][0] = { tmp_messages[i][0], aes.ecbEncBlock(tmp_messages[i][0]) };
			ot_messages[i][1] = { tmp_messages[i][1], aes.ecbEncBlock(tmp_messages[i][1]) };
		}
		//timer->setTimePoint("after aes");
	}

	std::vector<std::array<osuCrypto::block, 2>> correction_blocks(switches);
	prepare_correction(N, values, 0, 0, masks, ot_messages, correction_blocks);
	//timer->setTimePoint("after prepare_correction");
	chl.send(correction_blocks);
	for (int i = 0; i < values; ++i)
	{
		ret_masks[i].push_back(masks[i]);
	}
	//timer->setTimePoint("after correction_blocks");
	return ret_masks;
}

OSNReceiver::OSNReceiver(size_t size, int ot_type) : size(size), ot_type(ot_type)
{
}

void OSNReceiver::init(size_t size, int ot_type)
{
	this->size = size;
	this->ot_type = ot_type;
}

std::vector<oc::block> OSNReceiver::run_osn(std::vector<oc::block> inputs, std::vector<oc::Channel>& chls)
{
	int values = size;
	std::vector<std::vector<block>> ret_masks = gen_benes_client_osn(values, chls);
	std::vector<block> output_masks, benes_input;

	for (int i = 0; i < values; ++i)
		ret_masks[i][0] = ret_masks[i][0] ^ inputs[i];
	for (int i = 0; i < values; ++i)
		benes_input.push_back(ret_masks[i][0]);
	chls[0].send(benes_input);
	for (int i = 0; i < values; ++i)
		output_masks.push_back(ret_masks[i][1]);
	return output_masks;
}

void OSNReceiver::setTimer(Timer& timer)
{
	this->timer = &timer;
}

void OSNReceiver::prepare_correction(int n, int Val, int lvl_p, int perm_idx, std::vector<oc::block>& src,
	std::vector<std::array<std::array<osuCrypto::block, 2>, 2>>& ot_output,
	std::vector<std::array<osuCrypto::block, 2>>& correction_blocks)
{
	// ot message M0 = m0 ^ w0 || m1 ^ w1
	//  for each switch: top wire m0 w0 - bottom wires m1, w1
	//  M1 = m0 ^ w1 || m1 ^ w0
	int levels, i, j, x, s;
	std::vector<block> bottom1;
	std::vector<block> top1;
	int values = src.size();
	block temp;

	block m0, m1, w0, w1, M0[2], M1[2], corr_mesg[2];
	std::array<oc::block, 2> corr_block, temp_block;

	if (values == 2)
	{
		if (n == 1)
		{
			m0 = src[0];
			m1 = src[1];
			temp_block = ot_output[lvl_p * (Val / 2) + perm_idx][0];
			memcpy(M0, temp_block.data(), sizeof(M0));
			w0 = M0[0] ^ m0;
			w1 = M0[1] ^ m1;
			temp_block = ot_output[lvl_p * (Val / 2) + perm_idx][1];
			memcpy(M1, temp_block.data(), sizeof(M1));
			corr_mesg[0] = M1[0] ^ m0 ^ w1;
			corr_mesg[1] = M1[1] ^ m1 ^ w0;
			correction_blocks[lvl_p * (Val / 2) + perm_idx] = { corr_mesg[0], corr_mesg[1] };
			M1[0] = m0 ^ w1;
			M1[1] = m1 ^ w0;
			ot_output[lvl_p * (Val / 2) + perm_idx][1] = { M1[0], M1[1] };
			src[0] = w0;
			src[1] = w1;
			// std::cout<<" base index: "<<(lvl_p)*(Val/2)+perm_idx
			//  <<" m0 = "<<m0<<" "<<" m1 = "<<m1<<" w0 = "<<w0<<" "<<" w1 = "<<w1<<std::endl;
		}
		else
		{
			m0 = src[0];
			m1 = src[1];
			temp_block = ot_output[(lvl_p + 1) * (Val / 2) + perm_idx][0];
			memcpy(M0, temp_block.data(), sizeof(M0));
			w0 = M0[0] ^ m0;
			w1 = M0[1] ^ m1;
			temp_block = ot_output[(lvl_p + 1) * (Val / 2) + perm_idx][1];
			memcpy(M1, temp_block.data(), sizeof(M1));
			corr_mesg[0] = M1[0] ^ m0 ^ w1;
			corr_mesg[1] = M1[1] ^ m1 ^ w0;
			correction_blocks[(lvl_p + 1) * (Val / 2) + perm_idx] = { corr_mesg[0], corr_mesg[1] };
			M1[0] = m0 ^ w1;
			M1[1] = m1 ^ w0;
			ot_output[(lvl_p + 1) * (Val / 2) + perm_idx][1] = { M1[0], M1[1] };
			src[0] = w0;
			src[1] = w1;
			// std::cout<<" base index: "<<(lvl_p + 1)*(Val/2)+perm_idx
			//  <<" m0 = "<<m0<<" "<<" m1 = "<<m1<<" w0 = "<<w0<<" "<<" w1 = "<<w1<<std::endl;
		}
		return;
	}

	if (values == 3)
	{
		m0 = src[0];
		m1 = src[1];
		temp_block = ot_output[lvl_p * (Val / 2) + perm_idx][0];
		memcpy(M0, temp_block.data(), sizeof(M0));
		w0 = M0[0] ^ m0;
		w1 = M0[1] ^ m1;
		temp_block = ot_output[lvl_p * (Val / 2) + perm_idx][1];
		memcpy(M1, temp_block.data(), sizeof(M1));
		corr_mesg[0] = M1[0] ^ m0 ^ w1;
		corr_mesg[1] = M1[1] ^ m1 ^ w0;
		correction_blocks[lvl_p * (Val / 2) + perm_idx] = { corr_mesg[0], corr_mesg[1] };
		M1[0] = m0 ^ w1;
		M1[1] = m1 ^ w0;
		ot_output[lvl_p * (Val / 2) + perm_idx][1] = { M1[0], M1[1] };
		src[0] = w0;
		src[1] = w1;

		m0 = src[1];
		m1 = src[2];
		temp_block = ot_output[(lvl_p + 1) * (Val / 2) + perm_idx][0];
		memcpy(M0, temp_block.data(), sizeof(M0));
		w0 = M0[0] ^ m0;
		w1 = M0[1] ^ m1;
		temp_block = ot_output[(lvl_p + 1) * (Val / 2) + perm_idx][1];
		memcpy(M1, temp_block.data(), sizeof(M1));
		corr_mesg[0] = M1[0] ^ m0 ^ w1;
		corr_mesg[1] = M1[1] ^ m1 ^ w0;
		correction_blocks[(lvl_p + 1) * (Val / 2) + perm_idx] = { corr_mesg[0], corr_mesg[1] };
		M1[0] = m0 ^ w1;
		M1[1] = m1 ^ w0;
		ot_output[(lvl_p + 1) * (Val / 2) + perm_idx][1] = { M1[0], M1[1] };
		src[1] = w0;
		src[2] = w1;

		m0 = src[0];
		m1 = src[1];
		temp_block = ot_output[(lvl_p + 2) * (Val / 2) + perm_idx][0];
		memcpy(M0, temp_block.data(), sizeof(M0));
		w0 = M0[0] ^ m0;
		w1 = M0[1] ^ m1;
		temp_block = ot_output[(lvl_p + 2) * (Val / 2) + perm_idx][1];
		memcpy(M1, temp_block.data(), sizeof(M1));
		corr_mesg[0] = M1[0] ^ m0 ^ w1;
		corr_mesg[1] = M1[1] ^ m1 ^ w0;
		correction_blocks[(lvl_p + 2) * (Val / 2) + perm_idx] = { corr_mesg[0], corr_mesg[1] };
		M1[0] = m0 ^ w1;
		M1[1] = m1 ^ w0;
		ot_output[(lvl_p + 2) * (Val / 2) + perm_idx][1] = { M1[0], M1[1] };
		src[0] = w0;
		src[1] = w1;

		return;
	}

	levels = 2 * n - 1;

	// partea superioara
	for (i = 0; i < values - 1; i += 2)
	{
		m0 = src[i];
		m1 = src[i ^ 1];
		temp_block = ot_output[(lvl_p) * (Val / 2) + perm_idx + i / 2][0];
		memcpy(M0, temp_block.data(), sizeof(M0));
		w0 = M0[0] ^ m0;
		w1 = M0[1] ^ m1;
		temp_block = ot_output[(lvl_p) * (Val / 2) + perm_idx + i / 2][1];
		memcpy(M1, temp_block.data(), sizeof(M1));
		corr_mesg[0] = M1[0] ^ m0 ^ w1;
		corr_mesg[1] = M1[1] ^ m1 ^ w0;
		correction_blocks[(lvl_p) * (Val / 2) + perm_idx + i / 2] = { corr_mesg[0], corr_mesg[1] };
		M1[0] = m0 ^ w1;
		M1[1] = m1 ^ w0;
		ot_output[(lvl_p) * (Val / 2) + perm_idx + i / 2][1] = { M1[0], M1[1] };
		src[i] = w0;
		src[i ^ 1] = w1;

		bottom1.push_back(src[i]);
		top1.push_back(src[i ^ 1]);
	}

	if (values % 2 == 1)
	{
		top1.push_back(src[values - 1]);
	}

	prepare_correction(n - 1, Val, lvl_p + 1, perm_idx, bottom1, ot_output, correction_blocks);
	prepare_correction(n - 1, Val, lvl_p + 1, perm_idx + values / 4, top1, ot_output,
		correction_blocks);

	// partea inferioara
	for (i = 0; i < values - 1; i += 2)
	{
		m1 = top1[i / 2];
		m0 = bottom1[i / 2];
		temp_block = ot_output[(lvl_p + levels - 1) * (Val / 2) + perm_idx + i / 2][0];
		memcpy(M0, temp_block.data(), sizeof(M0));
		w0 = M0[0] ^ m0;
		w1 = M0[1] ^ m1;
		temp_block = ot_output[(lvl_p + levels - 1) * (Val / 2) + perm_idx + i / 2][1];
		memcpy(M1, temp_block.data(), sizeof(M1));
		corr_mesg[0] = M1[0] ^ m0 ^ w1;
		corr_mesg[1] = M1[1] ^ m1 ^ w0;
		correction_blocks[(lvl_p + levels - 1) * (Val / 2) + perm_idx + i / 2] = { corr_mesg[0], corr_mesg[1] };
		M1[0] = m0 ^ w1;
		M1[1] = m1 ^ w0;
		ot_output[(lvl_p + levels - 1) * (Val / 2) + perm_idx + i / 2][1] = { M1[0], M1[1] };
		src[i] = w0;
		src[i ^ 1] = w1;
	}

	int idx = int(ceil(values * 0.5));
	if (values % 2 == 1)
	{
		src[values - 1] = top1[idx - 1];
	}
}
