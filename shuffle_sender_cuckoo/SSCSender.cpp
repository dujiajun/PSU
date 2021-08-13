#include "SSCSender.h"
#include "utils.h"
#include "SimpleIndexParameters.h"
#include <set>

using namespace oc;
using namespace std;

oc::u8* SSCSender::runMpOprf(std::vector<oc::Channel>& chls, const std::vector<oc::block>& x_set, size_t width, size_t hash_length_in_bytes)
{
	PRNG prng(oc::toBlock(123));
	mp_oprf_receiver.setParams(toBlock(123456),
		shuffle_size,
		log2ceil(shuffle_size),
		width,
		hash_length_in_bytes,
		32,
		1 << 8,
		1 << 8);
	return mp_oprf_receiver.run(prng, chls, x_set);
}
std::vector<oc::block> SSCSender::runPermuteShareSender(std::vector<oc::Channel>& chls)
{
	return osn_sender.run_osn(chls);
}
std::vector<oc::block> SSCSender::runPermuteShareReceiver(const std::vector<oc::block>& x_set,
	std::vector<oc::Channel>& chls)
{
	return osn_receiver.run_osn(x_set, chls);
}
void SSCSender::setSenderSet(const std::vector<oc::block>& sender_set, size_t receiver_size)
{
	this->sender_set = sender_set;

	CuckooParam param = { 0, context.cuckoo_scaler, context.cuckoo_hash_num, context.sender_size };
	cuckoo.init(param);

	vector<size_t> indexes(context.sender_size);
	for (size_t i = 0; i < context.sender_size; i++)indexes[i] = i;

	cuckoo.insert(indexes, this->sender_set);
	after_cuckoo_set.resize(cuckoo.mBins.size());
	for (size_t i = 0; i < cuckoo.mBins.size(); i++)
	{
		auto& bin = cuckoo.mBins[i];
		if (bin.isEmpty())
			after_cuckoo_set[i] = AllOneBlock;
		else
			after_cuckoo_set[i] = sender_set[bin.idx()];
	}
	shuffle_size = after_cuckoo_set.size();

	osn_sender.init(shuffle_size, context.osn_ot_type, context.osn_cache);
	osn_receiver.init(shuffle_size, context.osn_ot_type);
}
void SSCSender::output(std::vector<oc::Channel>& chls)
{
	PRNG prng(oc::toBlock(123));

	auto shares = runPermuteShareReceiver(after_cuckoo_set, chls);

	timer->setTimePoint("after runPermuteShareReceiver");

	auto params = getMpOprfParams(context.sender_size, context.receiver_size);
	size_t hashLengthInBytes = params.second;
	u8* oprfs = runMpOprf(chls, shares, params.first, params.second);
	timer->setTimePoint("after runMpOprf");

	vector<bool> b(shares.size());

	size_t& num_threads = context.num_threads;
	size_t max_bin_size = get_simple_bin_size(context.sender_size, context.receiver_size);
	auto routine = [&](size_t tid)
	{
		size_t start_idx = shares.size() * tid / num_threads;
		size_t end_idx = shares.size() * (tid + 1) / num_threads;
		end_idx = ((end_idx <= shares.size()) ? end_idx : shares.size());

		vector<u8> recv_oprfs(max_bin_size * hashLengthInBytes * (end_idx - start_idx));
		chls[tid].recv(recv_oprfs);

		for (size_t i = start_idx; i < end_idx; i++)
		{
			set<PRF> bf;
			for (size_t j = 0; j < max_bin_size; j++)
			{
				bf.insert(PRF(hashLengthInBytes,
					recv_oprfs.data() + ((i - start_idx) * max_bin_size + j) * hashLengthInBytes));
			}
			PRF si(hashLengthInBytes, oprfs + i * hashLengthInBytes);
			b[i] = bf.find(si) == bf.end();
		}
	};

	vector<thread> thrds(num_threads);
	for (size_t i = 0; i < num_threads; i++)
		thrds[i] = std::thread(routine, i);
	for (size_t i = 0; i < num_threads; i++)
		thrds[i].join();
	timer->setTimePoint("after recv oprf");
	auto a = runPermuteShareSender(chls);
	timer->setTimePoint("after runPermuteShareSender");
	vector<int> pi = osn_sender.dest;

	vector<block> c(shares.size());
	for (size_t i = 0; i < pi.size(); i++)
	{
		c[i] = b[pi[i]] ? (shares[pi[i]] ^ a[i]) : AllOneBlock;
	}
	chls[0].send(c);
	delete[]oprfs;

}
void SSCSender::setTimer(oc::Timer& timer)
{
	osn_sender.setTimer(timer);
	osn_receiver.setTimer(timer);
	mp_oprf_receiver.setTimer(timer);
	this->timer = &timer;
}
