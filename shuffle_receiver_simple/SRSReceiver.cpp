#include "SRSReceiver.h"
#include <set>
#include "utils.h"
using namespace std;
using namespace oc;

std::vector<oc::block> SRSReceiver::runPermuteShare(size_t tid, const std::vector<oc::block>& x_set, std::vector<oc::Channel>& chls)
{
	osn_receivers[tid].init(x_set.size(), context.osn_ot_type);
	vector<Channel> tmpchls(chls.begin() + tid, chls.begin() + tid + 1);
	return osn_receivers[tid].run_osn(x_set, tmpchls);
}

oc::u8* SRSReceiver::runMpOprf(std::vector<oc::Channel>& chls, const std::vector<oc::block>& recv_set, size_t log2size, size_t width, size_t hash_length_in_bytes)
{
	PRNG prng(oc::toBlock(123));
	mp_oprf_receiver.setParams(toBlock(123456), 1ull << log2size, log2size, width, hash_length_in_bytes, 32, 1 << 8, 1 << 8);
	return mp_oprf_receiver.run(prng, chls, recv_set);
}

void SRSReceiver::setReceiverSet(const std::vector<oc::block>& receiver_set, size_t sender_size)
{
	this->receiver_set = receiver_set;
	simple.init(context.receiver_size);
	simple.insertItems(receiver_set, context.num_threads);
	for (auto& bin : simple.mBins)
	{
		for (size_t j = bin.mBinRealSizes; j < simple.mMaxBinSize; j++)
			bin.items.push_back(AllOneBlock);
	}
	osn_receivers.resize(context.num_threads);
}

std::vector<oc::block> SRSReceiver::output(std::vector<oc::Channel>& chls)
{
	PRNG prng(oc::toBlock(123));
	vector<block> union_result;
	vector<vector<block>> shares(simple.mBins.size());

	size_t& num_threads = context.num_threads;
	std::vector<std::thread> thrds(num_threads);

	auto ps_thread = [&](size_t tid)
	{
		for (size_t i = tid; i < simple.mNumBins; i += num_threads)
		{
			shares[i] = runPermuteShare(tid, simple.mBins[i].items, chls);
		}
	};

	for (u64 i = 0; i < thrds.size(); ++i)
	{
		thrds[i] = std::thread(ps_thread, i);
	}
	for (auto& thrd : thrds)
		thrd.join();


	timer->setTimePoint("after runPermuteShare");
	auto total_count = simple.mNumBins * simple.mMaxBinSize;
	auto tmp_count = log2ceil(total_count);
	vector<block> tmp_share(1ull << tmp_count);
	for (size_t i = 0; i < total_count; i++)
	{
		tmp_share[i] = shares[i / simple.mMaxBinSize][i % simple.mMaxBinSize];
	}

	auto params = getMpOprfParams(0, 1ull << tmp_count, 1ull << tmp_count);
	size_t hashLengthInBytes = params.second;
	u8* oprfs = runMpOprf(chls, tmp_share, tmp_count, params.first, params.second);
	timer->setTimePoint("after runMpOprf");
	BitVector choices(simple.mNumBins * simple.mMaxBinSize);

	auto cmp_thread = [&](size_t tid)
	{
		set<PRF> oprfs_set;
		vector<u8> recv_oprfs(simple.mMaxBinSize * shares[0].size() * hashLengthInBytes);

		for (size_t k = tid; k < simple.mNumBins; k += num_threads)
		{
			auto& bin = simple.mBins[k];
			auto& share = shares[k];
			chls[tid].recv(recv_oprfs);
			vector<PRF> recv_oprfs_v(simple.mMaxBinSize * share.size());
			for (size_t i = 0; i < recv_oprfs_v.size(); i++)
			{
				recv_oprfs_v[i].set(hashLengthInBytes, recv_oprfs.data() + i * hashLengthInBytes);
			}

			for (size_t x = 0; x < simple.mMaxBinSize; x++)
			{
				bool flag = false;
				oprfs_set.clear();
				for (size_t i = 0; i < share.size(); i++)
				{
					oprfs_set.insert(recv_oprfs_v[x * simple.mMaxBinSize + i]);
				}
				for (size_t j = 0; j < share.size(); j++)
				{
					auto oprf = PRF(hashLengthInBytes, oprfs + (k * simple.mMaxBinSize + j) * hashLengthInBytes);//
					if (oprfs_set.find(oprf) != oprfs_set.end())
					{
						choices[k * simple.mMaxBinSize + x] = 1;
						flag = true;
					}
				}
				if (flag == false)
				{
					choices[k * simple.mMaxBinSize + x] = 0;
				}
			}
		}
	};
	for (size_t i = 0; i < num_threads; i++)
	{
		thrds[i] = std::thread(cmp_thread, i);
	}
	for (auto& thrd : thrds)
		thrd.join();

	delete[]oprfs;
	vector<block> msgs(simple.mMaxBinSize * simple.mNumBins);
	ot_receiver.receiveChosen(choices, msgs, prng, chls[0]);

	for (auto& msg : msgs)
	{
		if (msg != ZeroBlock && msg != AllOneBlock)
		{
			union_result.push_back(msg);
		}
	}
	return union_result;
}

void SRSReceiver::setTimer(oc::Timer& timer)
{
	this->timer = &timer;
	mp_oprf_receiver.setTimer(timer);
	for (auto& receiver : osn_receivers)
		receiver.setTimer(timer);
}
