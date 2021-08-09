#include "SRCSender.h"
#include "MP-OPRF-Parameters.h"
using namespace osuCrypto;
using namespace std;

void SRCSender::setSenderSet(const std::vector<block>& sender_set, size_t receiver_size)
{
	this->sender_set_size = sender_set.size();
	this->sender_set = sender_set;
	receiver_set_size = receiver_size;
	cuckoo_hash_num = 3;
	CuckooParam cuckoo_param = { 0, 1.27, cuckoo_hash_num, receiver_set_size };
	cuckoo.init(cuckoo_param);
	shuffle_size = cuckoo.mBins.size();
	osn_sender.init(shuffle_size, 1);
}

std::vector<block> SRCSender::runPermuteShare(std::vector<Channel>& chls)
{
	return osn_sender.run_osn(chls);
}

void SRCSender::runMPOPRF(std::vector<Channel>& chls,
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
	mp_oprf_sender
		.setParams(commonSeed, set_size, logHeight, width, hashLengthInBytes, h1LengthInBytes, bucket1, bucket2);
	mp_oprf_sender.run(prng, chls);
}

void SRCSender::output(vector<Channel>& chls)
{
	PRNG prng(oc::toBlock(123));

	auto shares = runPermuteShare(chls);

	timer->setTimePoint("after runPermuteShare");
	vector<int> pi_out = osn_sender.dest;
	vector<size_t> pi_inv(pi_out.size());
	for (size_t i = 0; i < pi_inv.size(); i++)
	{
		pi_inv[pi_out[i]] = i;
	}

	size_t hashLengthInBytes = get_mp_oprf_hash_in_bytes(receiver_set_size, sender_set_size);
	runMPOPRF(chls,
		toBlock(123456),
		shuffle_size,
		log2ceil(shuffle_size),
		get_mp_oprf_width(receiver_set_size, sender_set_size),
		hashLengthInBytes,
		32,
		1 << 8,
		1 << 8);
	timer->setTimePoint("after runMpOprf");

	vector<array<block, 2>> msgs(sender_set_size);

	vector<u8> oprfs(cuckoo_hash_num * sender_set_size * hashLengthInBytes);

	size_t num_threads = chls.size();
	vector<thread> thrds(num_threads);
	auto routine = [&](size_t tid)
	{
	  size_t start_idx = sender_set.size() * tid / num_threads;
	  size_t end_idx = sender_set.size() * (tid + 1) / num_threads;
	  end_idx = ((end_idx <= sender_set.size()) ? end_idx : sender_set.size());

	  vector<block> tmp_thrd(cuckoo_hash_num * (end_idx - start_idx));
	  for (size_t x = start_idx; x < end_idx; x++)
	  {
		  for (size_t i = 0; i < cuckoo_hash_num; i++)
		  {
			  size_t index = oc::CuckooIndex<oc::ThreadSafe>::getHash(sender_set[x], i, cuckoo.mBins.size());
			  tmp_thrd[(x - start_idx) * cuckoo_hash_num + i] = _mm_xor_si128(sender_set[x], shares[pi_inv[index]]);
		  }

	  }
	  vector<u8> oprfs_thrd = mp_oprf_sender.get_oprf(tmp_thrd);
	  memcpy(oprfs.data() + start_idx * cuckoo_hash_num * hashLengthInBytes, oprfs_thrd.data(), oprfs_thrd.size());
	};

	for (size_t t = 0; t < num_threads; t++)
		thrds[t] = std::thread(routine, t);
	for (size_t t = 0; t < num_threads; t++)
		thrds[t].join();

	timer->setTimePoint("after compute oprf");
	chls[0].asyncSend(std::move(oprfs));
	timer->setTimePoint("after send oprf");
	for (size_t x = 0; x < sender_set.size(); x++)
	{
		msgs[x][0] = sender_set[x];
		msgs[x][1] = AllOneBlock;
	}
	ot_sender.sendChosen(msgs, prng, chls[0]);
}

void SRCSender::setTimer(Timer& timer)
{
	ot_sender.setTimer(timer);
	osn_sender.setTimer(timer);
	mp_oprf_sender.setTimer(timer);
	this->timer = &timer;
}
