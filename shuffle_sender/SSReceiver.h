#pragma once

#include "OSNSender.h"
#include "MPOPRFSender.h"
#include "cryptoTools/Common/Timer.h"
#include <PSUReceiver.h>
// Shuffle-Sender Receiver
class SSReceiver : public PSUReceiver
{
	OSNSender osn_sender;
	oc::MPOPRFSender mp_oprf_sender;

	std::vector<oc::block> runPermuteShare(std::vector<oc::Channel>& chls);
	void runMPOPRF(std::vector<oc::Channel>& chls, size_t width, size_t hash_length_in_bytes);

public:
	void setReceiverSet(const std::vector<oc::block>& receiver_set, size_t sender_size);
	std::vector<oc::block> output(std::vector<oc::Channel>& chls);
	void setTimer(oc::Timer& timer);
};

