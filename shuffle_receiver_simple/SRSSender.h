#pragma once
#include "OSNSender.h"
#include "MPOPRFSender.h"
#include "SimpleIndex.h"
#include "cryptoTools/Common/Timer.h"
#include <PSUSender.h>
// Shuffle-Receiver Sender with Simple Index
class SRSSender : public PSUSender
{
	oc::SimpleIndex simple;

	std::vector<OSNSender> osn_senders;
	oc::MPOPRFSender mp_oprf_sender;
	oc::IknpOtExtSender ot_sender;
	std::vector<oc::block> runPermuteShare(size_t tid, size_t shuffle_size, std::vector<oc::Channel>& chls);
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


