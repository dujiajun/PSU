#pragma once

#include "cryptoTools/Common/Defines.h"
#include "cryptoTools/Common/Timer.h"
#include <OSNReceiver.h>
#include <MPOPRFReceiver.h>
#include <PSUReceiver.h>


class SRReceiver : public PSUReceiver
{
	oc::Timer* timer;
	size_t sender_set_size;
	size_t receiver_set_size;
	size_t shuffle_size;

	std::vector<oc::block> receiver_set;

	OSNReceiver osn_receiver;
	oc::MPOPRFReceiver mp_oprf_receiver;
	oc::IknpOtExtReceiver ot_receiver;
	std::vector<oc::block> runPermuteShare(const std::vector<oc::block>& x_set, std::vector<oc::Channel>& chls);
	oc::u8* runMpOprf(std::vector<oc::Channel>& chls, const std::vector<oc::block>& recv_set, const oc::block& commonSeed, const oc::u64& set_size, const oc::u64& logHeight, const oc::u64& width, const oc::u64& hashLengthInBytes, const oc::u64& h1LengthInBytes, const oc::u64& bucket1, const oc::u64& bucket2);

public:
	void setReceiverSet(const std::vector<oc::block>& receiver_set, size_t sender_size);
	std::vector<oc::block> output(std::vector<oc::Channel>& chls);
	void setTimer(oc::Timer& timer);
};
