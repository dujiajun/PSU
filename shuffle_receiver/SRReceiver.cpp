#include "SRReceiver.h"
#include <utils.h>
#include <set>

using namespace std;
using namespace oc;

std::vector<oc::block> SRReceiver::runPermuteShare(const std::vector<oc::block>& x_set, std::vector<oc::Channel>& chls)
{
	return osn_receiver.run_osn(x_set, chls);
}

oc::u8* SRReceiver::runMpOprf(std::vector<oc::Channel>& chls, const std::vector<oc::block>& recv_set, size_t width, size_t hash_length_in_bytes)
{
	PRNG prng(oc::toBlock(123));
	mp_oprf_receiver.setParams(toBlock(123456), shuffle_size, log2ceil(shuffle_size), width, hash_length_in_bytes, 32, 1 << 8, 1 << 8);
	return mp_oprf_receiver.run(prng, chls, recv_set);
}

void SRReceiver::setReceiverSet(const std::vector<block>& receiver_set, size_t sender_size)
{
	this->receiver_set = receiver_set;
	shuffle_size = context.receiver_size;
	osn_receiver.init(shuffle_size, context.osn_ot_type);
}

std::vector<block> SRReceiver::output(std::vector<Channel>& chls)
{
	PRNG prng(oc::toBlock(123));

	auto share = runPermuteShare(receiver_set, chls);
	vector<block> union_result;

	timer->setTimePoint("after runPermuteShare");

	auto params = getMpOprfParams(0, context.sender_size, context.receiver_size);
	auto hashLengthInBytes = params.second;
	auto oprfs = runMpOprf(chls, share, params.first, params.second);
	timer->setTimePoint("after runMpOprf");

	BitVector choices(context.sender_size);

	size_t& num_threads = context.num_threads;
	auto routine = [&](size_t tid)
	{
		for (size_t i = tid; i < context.sender_size; i += num_threads)
		{
			bool flag = false;

			set<PRF> recved_oprfs_set;
			vector<u8> recv_oprfs(share.size() * hashLengthInBytes);
			chls[tid].recv(recv_oprfs);

			for (size_t j = 0; j < share.size(); j++)
			{
				recved_oprfs_set.insert(PRF(hashLengthInBytes, recv_oprfs.data() + j * hashLengthInBytes));
			}
			for (size_t j = 0; j < context.receiver_size; j++)
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
	vector<block> msgs(context.sender_size);
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