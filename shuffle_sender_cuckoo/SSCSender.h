#pragma once

#include "OSNReceiver.h"
#include "OSNSender.h"
#include "MPOPRFReceiver.h"
#include "CuckooIndex.h"
#include "cryptoTools/Common/Timer.h"
#include <PSUSender.h>
class SSCSender : public PSUSender
{
	std::vector<oc::block> after_cuckoo_set;

	oc::CuckooIndex<oc::ThreadSafe> cuckoo;

	OSNReceiver osn_receiver;
	OSNSender osn_sender;
	oc::MPOPRFReceiver mp_oprf_receiver;

	std::vector<oc::block> runPermuteShareSender(std::vector<oc::Channel>& chls);
	std::vector<oc::block> runPermuteShareReceiver(const std::vector<oc::block>& x_set, std::vector<oc::Channel>& chls);
	oc::u8* runMpOprf(std::vector<oc::Channel>& chls, const std::vector<oc::block>& x_set, size_t width, size_t hash_length_in_bytes);

public:
	void setSenderSet(const std::vector<oc::block>& sender_set, size_t receiver_size);
	void output(std::vector<oc::Channel>& chls);
	void setTimer(oc::Timer& timer);
};
