#include "SRCReceiver.h"
#include "MP-OPRF-Parameters.h"
#include "utils.h"
#include <set>
using namespace osuCrypto;
using namespace std;


void SRCReceiver::setReceiverSet(const std::vector<block>& receiver_set, size_t sender_size)
{
	this->receiver_set = receiver_set;
	this->receiver_set_size = receiver_set.size();
	this->sender_set_size = sender_size;

	cuckoo_hash_num = 4;
	CuckooParam cuckoo_param = { 0, 1.09, cuckoo_hash_num, receiver_set_size };
	cuckoo.init(cuckoo_param);
	vector<size_t> indexes(receiver_set_size);
	for (size_t i = 0; i < receiver_set_size; i++)indexes[i] = i;
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
	osn_receiver.init(shuffle_size, 1);
}

std::vector<block> SRCReceiver::runPermuteShare(const vector<block>& x_set, std::vector<Channel>& chls)
{
	return osn_receiver.run_osn(x_set, chls);
}

u8* SRCReceiver::runMpOprf(std::vector<Channel>& chls,
	const vector<block>& recv_set,
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
	mp_oprf_receiver
		.setParams(commonSeed, set_size, logHeight, width, hashLengthInBytes, h1LengthInBytes, bucket1, bucket2);
	return mp_oprf_receiver.run(prng, chls, recv_set);
}

std::vector<block> SRCReceiver::output(vector<Channel>& chls)
{
	PRNG prng(oc::toBlock(123));
	vector<block> union_result;

	auto shares = runPermuteShare(after_cuckoo_set, chls);

	timer->setTimePoint("after runPermuteShare");

	size_t hashLengthInBytes = get_mp_oprf_hash_in_bytes(receiver_set_size, sender_set_size);

	u8* oprfs = runMpOprf(chls,
		shares,
		toBlock(123456),
		shuffle_size,
		log2ceil(shuffle_size),
		get_mp_oprf_width(receiver_set_size, sender_set_size),
		hashLengthInBytes,
		32,
		1 << 8,
		1 << 8);
	timer->setTimePoint("after runMpOprf");

	set<PRF> oprfs_set;
	for (size_t i = 0; i < shuffle_size; i++)
	{
		PRF oprf(hashLengthInBytes, oprfs + i * hashLengthInBytes);
		oprfs_set.insert(oprf);
	}

	BitVector choices(sender_set_size);

	vector<u8> recv_oprfs(cuckoo_hash_num * sender_set_size * hashLengthInBytes);

	chls[0].recv(recv_oprfs);
	timer->setTimePoint("after recv oprf");

	for (size_t x = 0; x < sender_set_size; x++)
	{
		bool flag = false;
		for (size_t j = 0; j < cuckoo_hash_num; j++)
		{
			auto oprf = PRF(hashLengthInBytes, recv_oprfs.data() + (x * cuckoo_hash_num + j) * hashLengthInBytes);//
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
	vector<block> msgs(sender_set_size);
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

