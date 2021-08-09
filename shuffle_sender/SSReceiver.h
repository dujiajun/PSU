#pragma once

#include "OSNSender.h"
#include "MPOPRFSender.h"
#include "cryptoTools/Common/Timer.h"
#include <PSUReceiver.h>
// Shuffle-Sender Receiver
class SSReceiver : public PSUReceiver
{
	oc::Timer* timer;
	size_t sender_set_size;
	size_t receiver_set_size;
	size_t shuffle_size;

	std::vector<oc::block> receiver_set;

	OSNSender osn_sender;
	oc::MPOPRFSender mp_oprf_sender;

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
	void setReceiverSet(const std::vector<oc::block>& receiver_set, size_t sender_size);
	std::vector<oc::block> output(std::vector<oc::Channel>& chls);
	void setTimer(oc::Timer& timer);
};

