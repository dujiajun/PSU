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
	void runMPOPRF(std::vector<oc::Channel>& chls, size_t log2size, size_t width, size_t hash_length_in_bytes);
public:
	void setSenderSet(const std::vector<oc::block>& sender_set, size_t receiver_size);
	void output(std::vector<oc::Channel>& chls);
	void setTimer(oc::Timer& timer);
};


