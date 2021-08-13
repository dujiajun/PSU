#include "MP-OPRF-Parameters.h"
#include "SSReceiver.h"
using namespace std;
using namespace oc;

std::vector<oc::block> SSReceiver::runPermuteShare(std::vector<oc::Channel>& chls)
{
	return osn_sender.run_osn(chls);
}
void SSReceiver::runMPOPRF(std::vector<oc::Channel>& chls, size_t width, size_t hash_length_in_bytes)
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
void SSReceiver::setReceiverSet(const std::vector<oc::block>& receiver_set, size_t sender_size)
{
	this->receiver_set = receiver_set;
	shuffle_size = sender_size;
	osn_sender.init(shuffle_size, context.osn_ot_type);
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

	auto params = getMpOprfParams(context.sender_size, context.receiver_size);
	size_t hashLengthInBytes = params.second;
	runMPOPRF(chls, params.first, params.second);

	timer->setTimePoint("after runMpOprf");

	size_t& num_threads = context.num_threads;
	auto routine = [&](size_t tid)
	{
		for (size_t i = tid; i < shares.size(); i += num_threads)
		{
			vector<block> tmp(context.receiver_size);
			for (size_t j = 0; j < context.receiver_size; j++)
				tmp[j] = _mm_xor_si128(shares[i], receiver_set[j]);
			auto oprfs = mp_oprf_sender.get_oprf(tmp);
			chls[tid].asyncSend(std::move(oprfs));
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
