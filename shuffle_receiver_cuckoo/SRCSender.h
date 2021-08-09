#pragma once
#include "OSNSender.h"
#include "MPOPRFSender.h"
#include "CuckooIndex.h"
#include "cryptoTools/Common/Timer.h"
#include <PSUSender.h>
// Shuffle-Receiver Sender with Cuckoo Hash
class SRCSender : public PSUSender
{
	oc::Timer* timer;
	size_t sender_set_size;
	size_t receiver_set_size;
	size_t cuckoo_hash_num;
	size_t shuffle_size;

	std::vector<oc::block> sender_set;

	oc::CuckooIndex<oc::ThreadSafe> cuckoo;
	OSNSender osn_sender;
	oc::MPOPRFSender mp_oprf_sender;
	oc::IknpOtExtSender ot_sender;
	std::vector<oc::block> runPermuteShare(std::vector<oc::Channel>& chls);
	void runMPOPRF(std::vector<oc::Channel>& chls,
		const oc::block& commonSeed,
		const oc::u64& set_size,
		const oc::u64& logHeight,
		const oc::u64& width,
		const oc::u64& hashLengthInBytes,
		const oc::u64& h1LengthInBytes,
		const oc::u64& bucket1,
		const oc::u64& bucket2);

 public:
	void setSenderSet(const std::vector<oc::block>& sender_set, size_t receiver_size);
	void output(std::vector<oc::Channel>& chls);
	void setTimer(oc::Timer& timer);
};


