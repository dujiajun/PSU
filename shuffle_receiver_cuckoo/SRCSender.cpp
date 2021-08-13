#include "SRCSender.h"
#include "MP-OPRF-Parameters.h"
using namespace osuCrypto;
using namespace std;

void SRCSender::setSenderSet(const std::vector<block>& sender_set, size_t receiver_size)
{
	this->sender_set = sender_set;
	CuckooParam cuckoo_param = { 0, context.cuckoo_scaler, context.cuckoo_hash_num, context.receiver_size };
	cuckoo.init(cuckoo_param);
	shuffle_size = cuckoo.mBins.size();
	osn_sender.init(shuffle_size, context.osn_ot_type);
}

std::vector<block> SRCSender::runPermuteShare(std::vector<Channel>& chls)
{
	return osn_sender.run_osn(chls);
}

void SRCSender::runMPOPRF(std::vector<Channel>& chls, size_t width, size_t hash_length_in_bytes)
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
	auto params = getMpOprfParams(context.receiver_size, context.sender_size);
	size_t hashLengthInBytes = params.second;
	runMPOPRF(chls, params.first, params.second);
	timer->setTimePoint("after runMpOprf");

	vector<array<block, 2>> msgs(context.sender_size);

	vector<u8> oprfs(context.cuckoo_hash_num * context.sender_size * hashLengthInBytes);

	size_t& num_threads = context.num_threads;
	vector<thread> thrds(num_threads);
	auto routine = [&](size_t tid)
	{
		size_t start_idx = sender_set.size() * tid / num_threads;
		size_t end_idx = sender_set.size() * (tid + 1) / num_threads;
		end_idx = ((end_idx <= sender_set.size()) ? end_idx : sender_set.size());

		vector<block> tmp_thrd(context.cuckoo_hash_num * (end_idx - start_idx));
		for (size_t x = start_idx; x < end_idx; x++)
		{
			for (size_t i = 0; i < context.cuckoo_hash_num; i++)
			{
				size_t index = oc::CuckooIndex<oc::ThreadSafe>::getHash(sender_set[x], i, cuckoo.mBins.size());
				tmp_thrd[(x - start_idx) * context.cuckoo_hash_num + i] = _mm_xor_si128(sender_set[x], shares[pi_inv[index]]);
			}

		}
		vector<u8> oprfs_thrd = mp_oprf_sender.get_oprf(tmp_thrd);
		memcpy(oprfs.data() + start_idx * context.cuckoo_hash_num * hashLengthInBytes, oprfs_thrd.data(), oprfs_thrd.size());
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
