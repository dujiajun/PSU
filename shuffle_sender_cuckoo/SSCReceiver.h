#pragma once

#include "OSNReceiver.h"
#include "OSNSender.h"
#include "MPOPRFSender.h"
#include "CuckooIndex.h"
#include "cryptoTools/Common/Timer.h"
#include <PSUReceiver.h>
class SSCReceiver : public PSUReceiver
{
	std::vector<oc::block> after_cuckoo_set;

	size_t cuckoo_max_bin_size;
	std::vector<std::vector<oc::block>> simple;

	oc::MPOPRFSender mp_oprf_sender;
	OSNSender osn_sender;

	OSNReceiver osn_receiver;

	std::vector<oc::block> runPermuteShareSender(std::vector<oc::Channel>& chls);
	std::vector<oc::block> runPermuteShareReceiver(const std::vector<oc::block>& x_set, std::vector<oc::Channel>& chls);
	void runMPOPRF(std::vector<oc::Channel>& chls, size_t width, size_t hash_length_in_bytes);

public:
	void setReceiverSet(const std::vector<oc::block>& receiver_set, size_t sender_size);
	std::vector<oc::block> output(std::vector<oc::Channel>& chls);
	void setTimer(oc::Timer& timer);
};
