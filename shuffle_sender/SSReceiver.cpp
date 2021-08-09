//
// Created by dujiajun on 2021/8/8.
//

#include "MP-OPRF-Parameters.h"
#include "SSReceiver.h"
using namespace std;
using namespace oc;

std::vector<oc::block> SSReceiver::runPermuteShare(std::vector<oc::Channel>& chls)
{
	return osn_sender.run_osn(chls);
}
void SSReceiver::runMPOPRF(std::vector<oc::Channel>& chls,
	const oc::block& commonSeed,
	const osuCrypto::u64& set_size,
	const osuCrypto::u64& logHeight,
	const osuCrypto::u64& width,
	const osuCrypto::u64& hashLengthInBytes,
	const osuCrypto::u64& h1LengthInBytes,
	const osuCrypto::u64& bucket1,
	const osuCrypto::u64& bucket2)
{
	PRNG prng(oc::toBlock(123));
	mp_oprf_sender
		.setParams(commonSeed, set_size, logHeight, width, hashLengthInBytes, h1LengthInBytes, bucket1, bucket2);
	mp_oprf_sender.run(prng, chls);
}
void SSReceiver::setReceiverSet(const std::vector<oc::block>& receiver_set, size_t sender_size)
{
	this->receiver_set_size = receiver_set.size();
	this->receiver_set = receiver_set;
	sender_set_size = sender_size;
	shuffle_size = sender_size;

	osn_sender.init(shuffle_size, 1);
}
std::vector<oc::block> SSReceiver::output(std::vector<oc::Channel>& chls)
{
	PRNG prng(oc::toBlock(123));
	vector<block> union_result;
	auto shares = runPermuteShare(chls);

	timer->setTimePoint("after runPermuteShare");
	
	vector<int> pi_out = osn_sender.dest;
	vector<size_t> pi_inv(pi_out.size());
	for (size_t i = 0; i < pi_inv.size(); i++)
	{
		pi_inv[pi_out[i]] = i;
	}

	size_t hashLengthInBytes = get_mp_oprf_hash_in_bytes(shuffle_size);
	runMPOPRF(chls,
		toBlock(123456),
		shuffle_size,
		log2ceil(shuffle_size),
		get_mp_oprf_width(shuffle_size),
		hashLengthInBytes,
		32,
		1 << 8,
		1 << 8);

	auto sender_size = receiver_set_size;
	timer->setTimePoint("after runMpOprf");
	
	size_t num_threads = chls.size();
	auto routine = [&](size_t tid)
	{
	  for (size_t i = tid; i < shares.size(); i += num_threads)
	  {
		  vector<block> tmp(receiver_set_size);
		  for (size_t j = 0; j < receiver_set_size; j++)
			  tmp[j] = _mm_xor_si128(shares[i], receiver_set[j]);
		  auto oprfs = mp_oprf_sender.get_oprf(tmp);
		  chls[tid].send(oprfs);
	  }
	};
	vector<thread> thrds(num_threads);
	for (size_t i = 0; i < num_threads; i++)
		thrds[i] = std::thread(routine, i);
	for (size_t i = 0; i < num_threads; i++)
		thrds[i].join();

	vector<block> msgs(shares.size());
	chls[0].recv(msgs);
	for (size_t i = 0; i < msgs.size(); i++)
	{
		if (msgs[i] != AllOneBlock)
		{
			union_result.emplace_back(_mm_xor_si128(msgs[i], shares[i]));
		}
	}
	return union_result;
}
void SSReceiver::setTimer(oc::Timer& timer)
{
	this->timer = &timer;
	osn_sender.setTimer(timer);
	mp_oprf_sender.setTimer(timer);
}
