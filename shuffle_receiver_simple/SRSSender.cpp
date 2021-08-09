#include "SRSSender.h"

using namespace std;
using namespace oc;

std::vector<oc::block> SRSSender::runPermuteShare(size_t tid, size_t shuffle_size, std::vector<oc::Channel>& chls)
{
	osn_senders[tid].init(shuffle_size, 1);
	vector<Channel> tmpchls(chls.begin() + tid, chls.begin() + tid + 1);
	return osn_senders[tid].run_osn(tmpchls);
}

void SRSSender::runMPOPRF(std::vector<oc::Channel>& chls, const oc::block& commonSeed, const oc::u64& set_size, const oc::u64& logHeight, const oc::u64& width, const oc::u64& hashLengthInBytes, const oc::u64& h1LengthInBytes, const oc::u64& bucket1, const oc::u64& bucket2)
{
	PRNG prng(oc::toBlock(123));
	mp_oprf_sender.setParams(commonSeed, set_size, logHeight, width, hashLengthInBytes, h1LengthInBytes, bucket1, bucket2);
	mp_oprf_sender.run(prng, chls);
}

void SRSSender::setThreads(size_t num_threads)
{
	osn_senders.resize(num_threads);
}

void SRSSender::setSenderSet(const std::vector<oc::block>& sender_set, size_t receiver_size)
{
	this->sender_set_size = sender_set.size();
	this->sender_set = sender_set;
	receiver_set_size = receiver_size;

	simple.init(sender_set.size());
	simple.insertItems(sender_set, 1);
	for (auto& bin : simple.mBins)
	{
		for (size_t j = bin.mBinRealSizes; j < simple.mMaxBinSize; j++)
			bin.items.push_back(AllOneBlock);
	}

}

void SRSSender::output(std::vector<oc::Channel>& chls)
{
	PRNG prng(oc::toBlock(123));
	vector<vector<block>> shares(simple.mNumBins);

	auto num_threads = chls.size();
	std::vector<std::thread> thrds(num_threads);

	auto ps_thread = [&](size_t tid)
	{
		for (size_t j = tid; j < simple.mNumBins; j += num_threads)
		{
			shares[j] = runPermuteShare(tid, simple.mBins[j].items.size(), chls);
		}

	};

	for (u64 i = 0; i < thrds.size(); ++i)
	{
		thrds[i] = std::thread(ps_thread, i);
	}
	for (auto& thrd : thrds)
		thrd.join();

	auto total_count = simple.mNumBins * simple.mMaxBinSize;
	auto tmp_count = log2ceil(total_count);
	cout << "Sender: " << total_count << " " << tmp_count << endl;
	size_t hashLengthInBytes = get_mp_oprf_hash_in_bytes(1ull << tmp_count);
	runMPOPRF(chls, toBlock(123456), 1ull << tmp_count, tmp_count, get_mp_oprf_width(1ull << tmp_count), hashLengthInBytes, 32, 1 << 8, 1 << 8);
	vector<array<block, 2>> msgs(simple.mMaxBinSize * simple.mNumBins);

	auto cmp_thread = [&](size_t tid)
	{
		for (size_t j = tid; j < simple.mNumBins; j += num_threads)
		{
			auto& bin = simple.mBins[j];
			auto& share = shares[j];

			vector<block> tmp(share.size());
			vector<u8> oprfs(simple.mMaxBinSize * share.size() * hashLengthInBytes);
			for (size_t x = 0; x < simple.mMaxBinSize; x++)
			{
				for (size_t i = 0; i < share.size(); i++)
					tmp[i] = _mm_xor_si128(bin.items[x], share[i]);
				vector<u8> oprfs_tmp = mp_oprf_sender.get_oprf(tmp);
				memcpy(oprfs.data() + x * share.size() * hashLengthInBytes, oprfs_tmp.data(), hashLengthInBytes * tmp.size());
			}
			chls[tid].asyncSend(std::move(oprfs));

			for (size_t i = 0; i < simple.mMaxBinSize; i++)
			{
				msgs[j * simple.mMaxBinSize + i][0] = bin.items[i];
				msgs[j * simple.mMaxBinSize + i][1] = ZeroBlock;
			}
		}
	};

	for (size_t i = 0; i < num_threads; i++)
	{
		thrds[i] = std::thread(cmp_thread, i);
	}
	for (auto& thrd : thrds)
		thrd.join();

	ot_sender.sendChosen(msgs, prng, chls[0]);
}

void SRSSender::setTimer(oc::Timer& timer)
{
	this->timer = &timer;
	mp_oprf_sender.setTimer(timer);
	for (auto& sender : osn_senders)
		sender.setTimer(timer);
}
