//
// Created by dujiajun on 2021/8/8.
//

#include "SRReceiver.h"
#include <utils.h>
#include <set>

using namespace std;
using namespace oc;

std::vector<oc::block> SRReceiver::runPermuteShare(const std::vector<oc::block>& x_set, std::vector<oc::Channel>& chls)
{
	return osn_receiver.run_osn(x_set, chls);
}

oc::u8* SRReceiver::runMpOprf(std::vector<oc::Channel>& chls, const std::vector<oc::block>& recv_set, const oc::block& commonSeed, const oc::u64& set_size, const oc::u64& logHeight, const oc::u64& width, const oc::u64& hashLengthInBytes, const oc::u64& h1LengthInBytes, const oc::u64& bucket1, const oc::u64& bucket2)
{
	PRNG prng(oc::toBlock(123));
	mp_oprf_receiver.setParams(commonSeed, set_size, logHeight, width, hashLengthInBytes, h1LengthInBytes, bucket1, bucket2);
	return mp_oprf_receiver.run(prng, chls, recv_set);
}

void SRReceiver::setReceiverSet(const std::vector<block>& receiver_set, size_t sender_size)
{
	this->receiver_set_size = receiver_set.size();
	this->receiver_set = receiver_set;

	shuffle_size = receiver_set_size;
	sender_set_size = sender_size;

	osn_receiver.init(shuffle_size, 1);
}

std::vector<block> SRReceiver::output(std::vector<Channel>& chls)
{
	PRNG prng(oc::toBlock(123));

	auto share = runPermuteShare(receiver_set, chls);
	vector<block> union_result;

	timer->setTimePoint("after runPermuteShare");

	auto hashLengthInBytes = get_mp_oprf_hash_in_bytes(shuffle_size);
	auto oprfs = runMpOprf(chls, share, toBlock(123456), shuffle_size, log2ceil(shuffle_size), get_mp_oprf_width(shuffle_size), hashLengthInBytes, 32, 1 << 8, 1 << 8);
	timer->setTimePoint("after runMpOprf");

	BitVector choices(sender_set_size);

	size_t num_threads = chls.size();
	auto routine = [&](size_t tid)
	{
		for (size_t i = tid; i < sender_set_size; i += num_threads)
		{
			bool flag = false;

			set<PRF> recved_oprfs_set;
			vector<u8> recv_oprfs(share.size() * hashLengthInBytes);
			chls[tid].recv(recv_oprfs);

			for (size_t j = 0; j < share.size(); j++)
			{
				recved_oprfs_set.insert(PRF(hashLengthInBytes, recv_oprfs.data() + j * hashLengthInBytes));
			}
			for (size_t j = 0; j < receiver_set_size; j++)
			{
				if (recved_oprfs_set.find(PRF(hashLengthInBytes, oprfs + j * hashLengthInBytes)) != recved_oprfs_set.end())
				{
					choices[i] = 1;
					flag = true;
				}
			}
			if (flag == false)
			{
				choices[i] = 0;
			}

		}
	};
	vector<thread> thrds(num_threads);
	for (size_t i = 0; i < num_threads; i++)
		thrds[i] = std::thread(routine, i);
	for (size_t i = 0; i < num_threads; i++)
		thrds[i].join();
	delete[] oprfs;
	timer->setTimePoint("after compare oprf");
	vector<block> msgs(sender_set_size);
	ot_receiver.receiveChosen(choices, msgs, prng, chls[0]);

	for (auto& msg : msgs)
	{
		if (msg != ZeroBlock)
		{
			union_result.push_back(msg);
		}
	}
	return union_result;
}

void SRReceiver::setTimer(Timer& timer)
{
	this->timer = &timer;
	osn_receiver.setTimer(timer);
	mp_oprf_receiver.setTimer(timer);
}