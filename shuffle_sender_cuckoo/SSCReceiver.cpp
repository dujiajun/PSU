#include "SSCReceiver.h"
#include "utils.h"
#include "SimpleIndexParameters.h"

using namespace std;
using namespace oc;

void SSCReceiver::runMPOPRF(std::vector<oc::Channel>& chls, size_t width, size_t hash_length_in_bytes)
{
	PRNG prng(oc::toBlock(123));
	mp_oprf_sender.setParams(toBlock(123456),
		shuffle_size,
		log2ceil(shuffle_size),
		width,
		hash_length_in_bytes,
		32,
		1 << 8,
		1 << 8);
	mp_oprf_sender.run(prng, chls);
}
std::vector<oc::block> SSCReceiver::runPermuteShareReceiver(const std::vector<oc::block>& x_set,
	std::vector<oc::Channel>& chls)
{
	return osn_receiver.run_osn(x_set, chls);
}
void SSCReceiver::setReceiverSet(const std::vector<oc::block>& receiver_set, size_t sender_size)
{
	this->receiver_set = receiver_set;
	size_t cuckoo_bin_num = ceil(context.cuckoo_scaler * sender_size);
	simple.resize(cuckoo_bin_num);

	for (auto& y : receiver_set)
	{
		for (size_t i = 0; i < context.cuckoo_hash_num; i++)
		{
			size_t idx = CuckooIndex<ThreadSafe>::getHash(y, i, cuckoo_bin_num);
			simple[idx].emplace_back(y);
		}
	}

	cuckoo_max_bin_size = get_simple_bin_size(context.sender_size, context.receiver_size);

	shuffle_size = cuckoo_bin_num;

	osn_sender.init(shuffle_size, context.osn_ot_type, context.osn_cache);
	osn_receiver.init(shuffle_size, context.osn_ot_type);
}
std::vector<oc::block> SSCReceiver::output(std::vector<oc::Channel>& chls)
{
	auto shares = runPermuteShareSender(chls);
	timer->setTimePoint("after runPermuteShareSender");
	vector<int> pi_out = osn_sender.dest;

	auto params = getMpOprfParams(context.sender_size, context.receiver_size);
	size_t hashLengthInBytes = params.second;
	runMPOPRF(chls, params.first, params.second);
	timer->setTimePoint("after runMpOprf");
	size_t& num_threads = context.num_threads;
	auto routine = [&](size_t tid)
	{
		PRNG prng(oc::toBlock(123));

		size_t start_idx = shares.size() * tid / num_threads;
		size_t end_idx = shares.size() * (tid + 1) / num_threads;
		end_idx = ((end_idx <= shares.size()) ? end_idx : shares.size());

		vector<u8> send_buff(cuckoo_max_bin_size * hashLengthInBytes * (end_idx - start_idx));

		vector<block> tmp(cuckoo_max_bin_size * (end_idx - start_idx));
		size_t tmp_size = 0;
		for (size_t i = start_idx; i < end_idx; i++)
		{
			size_t bin_idx = pi_out[i];
			for (size_t j = 0; j < simple[bin_idx].size(); j++)
			{
				tmp[tmp_size] = shares[i] ^ simple[bin_idx][j];
				tmp_size++;
			}

		}
		tmp.resize(tmp_size);
		auto oprfs = mp_oprf_sender.get_oprf(tmp);
		size_t offset = 0;
		for (size_t i = start_idx; i < end_idx; i++)
		{
			size_t bin_idx = pi_out[i];
			size_t bin_oprf_size = simple[bin_idx].size() * hashLengthInBytes;
			size_t buff_start = (i - start_idx) * cuckoo_max_bin_size * hashLengthInBytes;
			memcpy(send_buff.data() + buff_start, oprfs.data() + offset * hashLengthInBytes, bin_oprf_size);
			prng.get<u8>(send_buff.data() + buff_start + bin_oprf_size,
				cuckoo_max_bin_size * hashLengthInBytes - bin_oprf_size);
			offset += simple[bin_idx].size();
		}
		chls[tid].asyncSend(std::move(send_buff));
	};
	vector<thread> thrds(num_threads);
	for (size_t i = 0; i < num_threads; i++)
		thrds[i] = std::thread(routine, i);
	for (size_t i = 0; i < num_threads; i++)
		thrds[i].join();
	timer->setTimePoint("after send oprf");
	auto a = runPermuteShareReceiver(shares, chls);

	timer->setTimePoint("after runPermuteShareReceiver");
	vector<block> c(shares.size());
	chls[0].recv(c);
	vector<block> union_result;
	for (size_t i = 0; i < c.size(); i++)
	{
		block tmp = c[i] ^ a[i];
		if (c[i] != AllOneBlock && tmp != AllOneBlock)
		{
			union_result.emplace_back(tmp);
		}
	}
	return union_result;
}
void SSCReceiver::setTimer(oc::Timer& timer)
{
	this->timer = &timer;
	osn_sender.setTimer(timer);
	osn_receiver.setTimer(timer);
	mp_oprf_sender.setTimer(timer);
}
std::vector<oc::block> SSCReceiver::runPermuteShareSender(vector<oc::Channel>& chls)
{
	return osn_sender.run_osn(chls);
}
