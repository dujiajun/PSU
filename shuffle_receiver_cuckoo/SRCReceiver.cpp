#include "SRCReceiver.h"
#include "MP-OPRF-Parameters.h"
#include "utils.h"
#include <set>
using namespace osuCrypto;
using namespace std;


void SRCReceiver::setReceiverSet(const std::vector<block>& receiver_set, size_t sender_size)
{
	this->receiver_set = receiver_set;

	CuckooParam cuckoo_param = { 0, context.cuckoo_scaler, context.cuckoo_hash_num, context.receiver_size };
	cuckoo.init(cuckoo_param);
	vector<size_t> indexes(context.receiver_size);
	for (size_t i = 0; i < context.receiver_size; i++)indexes[i] = i;
	//cuckoo.insert(receiver_set);
	cuckoo.insert(indexes, this->receiver_set);
	after_cuckoo_set.resize(cuckoo.mBins.size());
	for (size_t i = 0; i < cuckoo.mBins.size(); i++)
	{
		auto& bin = cuckoo.mBins[i];
		if (bin.isEmpty())
			after_cuckoo_set[i] = AllOneBlock;
		else
			after_cuckoo_set[i] = receiver_set[bin.idx()];
	}
	shuffle_size = after_cuckoo_set.size();
	osn_receiver.init(shuffle_size, context.osn_ot_type);
}

std::vector<block> SRCReceiver::runPermuteShare(const vector<block>& x_set, std::vector<Channel>& chls)
{
	return osn_receiver.run_osn(x_set, chls);
}

u8* SRCReceiver::runMpOprf(std::vector<Channel>& chls,
	const vector<block>& recv_set, size_t width, size_t hash_length_in_bytes)
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
	return mp_oprf_receiver.run(prng, chls, recv_set);
}

std::vector<block> SRCReceiver::output(vector<Channel>& chls)
{
	PRNG prng(oc::toBlock(123));
	vector<block> union_result;

	auto shares = runPermuteShare(after_cuckoo_set, chls);
	timer->setTimePoint("after runPermuteShare");

	auto params = getMpOprfParams(context.cuckoo_hash_num, context.receiver_size, context.sender_size);
	size_t hashLengthInBytes = params.second;
	u8* oprfs = runMpOprf(chls, shares, params.first, params.second);
	timer->setTimePoint("after runMpOprf");

	set<PRF> oprfs_set;
	for (size_t i = 0; i < shuffle_size; i++)
	{
		PRF oprf(hashLengthInBytes, oprfs + i * hashLengthInBytes);
		oprfs_set.insert(oprf);
	}

	BitVector choices(context.sender_size);
	vector<u8> recv_oprfs(context.cuckoo_hash_num * context.sender_size * hashLengthInBytes);

	chls[0].recv(recv_oprfs);
	timer->setTimePoint("after recv oprf");

	for (size_t x = 0; x < context.sender_size; x++)
	{
		bool flag = false;
		for (size_t j = 0; j < context.cuckoo_hash_num; j++)
		{
			auto oprf = PRF(hashLengthInBytes, recv_oprfs.data() + (x * context.cuckoo_hash_num + j) * hashLengthInBytes);//
			if (oprfs_set.find(oprf) != oprfs_set.end())
			{
				choices[x] = 1;
				flag = true;
				break;
			}
		}
		if (!flag)
		{
			choices[x] = 0;
		}
	}
	timer->setTimePoint("after compare oprf");

	delete[]oprfs;
	vector<block> msgs(context.sender_size);
	ot_receiver.receiveChosen(choices, msgs, prng, chls[0]);

	for (auto& msg : msgs)
	{
		if (msg != AllOneBlock)
		{
			union_result.emplace_back(msg);
		}
	}
	return union_result;
}

void SRCReceiver::setTimer(Timer& timer)
{
	this->timer = &timer;
	ot_receiver.setTimer(timer);
	osn_receiver.setTimer(timer);
	mp_oprf_receiver.setTimer(timer);
}

