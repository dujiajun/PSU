#pragma once

#include "OSNReceiver.h"
#include "MPOPRFReceiver.h"
#include "cryptoTools/Common/Timer.h"
#include <PSUSender.h>
// Shuffle-Sender Sender
class SSSender : public PSUSender
{
	OSNReceiver osn_receiver;
	oc::MPOPRFReceiver mp_oprf_receiver;

	std::vector<oc::block> runPermuteShare(const std::vector<oc::block>& x_set, std::vector<oc::Channel>& chls);
	oc::u8* runMpOprf(std::vector<oc::Channel>& chls, const std::vector<oc::block>& x_set, size_t width, size_t hash_length_in_bytes);

public:
	void setSenderSet(const std::vector<oc::block>& sender_set, size_t receiver_size);
	void output(std::vector<oc::Channel>& chls);
	void setTimer(oc::Timer& timer);
};
