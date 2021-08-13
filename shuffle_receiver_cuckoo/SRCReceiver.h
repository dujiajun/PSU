#pragma once
#include "OSNReceiver.h"
#include "MPOPRFReceiver.h"
#include "CuckooIndex.h"
#include "cryptoTools/Common/Timer.h"
#include <PSUReceiver.h>

// Shuffle-Receiver Receiver with Cuckoo Hash
class SRCReceiver : public PSUReceiver
{
	oc::CuckooIndex<oc::ThreadSafe> cuckoo;
	std::vector<oc::block> after_cuckoo_set;

	OSNReceiver osn_receiver;
	oc::MPOPRFReceiver mp_oprf_receiver;
	oc::IknpOtExtReceiver ot_receiver;

	std::vector<oc::block> runPermuteShare(const std::vector<oc::block>& x_set, std::vector<oc::Channel>& chls);
	oc::u8* runMpOprf(std::vector<oc::Channel>& chls, const std::vector<oc::block>& recv_set, size_t width, size_t hash_length_in_bytes);
public:
	void setReceiverSet(const std::vector<oc::block>& receiver_set, size_t sender_size);
	std::vector<oc::block> output(std::vector<oc::Channel>& chls);
	void setTimer(oc::Timer& timer);
};
