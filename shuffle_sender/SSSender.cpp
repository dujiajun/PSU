//
// Created by dujiajun on 2021/8/8.
//

#include "SSSender.h"
#include "utils.h"
#include <set>

using namespace std;
using namespace oc;
std::vector<oc::block> SSSender::runPermuteShare(const vector<oc::block>& x_set, vector<oc::Channel>& chls)
{
	return osn_receiver.run_osn(x_set, chls);
}
oc::u8* SSSender::runMpOprf(vector<oc::Channel>& chls,
	const vector<oc::block>& x_set,
	const block& commonSeed,
	const u64& set_size,
	const u64& logHeight,
	const u64& width,
	const u64& hashLengthInBytes,
	const u64& h1LengthInBytes,
	const u64& bucket1,
	const u64& bucket2)
{
	PRNG prng(oc::toBlock(123));
	mp_oprf_receiver
		.setParams(commonSeed, set_size, logHeight, width, hashLengthInBytes, h1LengthInBytes, bucket1, bucket2);
	return mp_oprf_receiver.run(prng, chls, x_set);
}
void SSSender::setSenderSet(const vector<oc::block>& sender_set, size_t receiver_size)
{
	this->sender_set = sender_set;
	shuffle_size = context.sender_size;
	osn_receiver.init(shuffle_size, context.osn_ot_type);
}
void SSSender::output(vector<oc::Channel>& chls)
{
	PRNG prng(oc::toBlock(123));

	auto shares = runPermuteShare(sender_set, chls);

	timer->setTimePoint("after runPermuteShare");

	size_t hashLengthInBytes = get_mp_oprf_hash_in_bytes(shuffle_size);
	u8* oprfs = runMpOprf(chls,
		shares,
		toBlock(123456),
		shuffle_size,
		log2ceil(shuffle_size),
		get_mp_oprf_width(shuffle_size),
		hashLengthInBytes,
		32,
		1 << 8,
		1 << 8);
	timer->setTimePoint("after runMpOprf");

	vector<block> msgs(shares.size());

	size_t& num_threads = context.num_threads;
	auto routine = [&](size_t tid)
	{
	  for (size_t i = tid; i < shares.size(); i += num_threads)
	  {
		  vector<u8> recv_oprfs(context.receiver_size * hashLengthInBytes);
		  chls[tid].recv(recv_oprfs);
		  set<PRF> bf;
		  for (size_t j = 0; j < context.receiver_size; j++)
		  {
			  bf.insert(PRF(hashLengthInBytes, recv_oprfs.data() + j * hashLengthInBytes));
		  }
		  PRF si(hashLengthInBytes, oprfs + i * hashLengthInBytes);
		  msgs[i] = (bf.find(si) == bf.end()) ? shares[i] : AllOneBlock;
	  }
	};

	vector<thread> thrds(num_threads);
	for (size_t i = 0; i < num_threads; i++)
		thrds[i] = std::thread(routine, i);
	for (size_t i = 0; i < num_threads; i++)
		thrds[i].join();

	chls[0].asyncSend(std::move(msgs));

	delete[]oprfs;
}
void SSSender::setTimer(Timer& timer)
{
	this->timer = &timer;
	osn_receiver.setTimer(timer);
	mp_oprf_receiver.setTimer(timer);
}
