//
// Created by dujiajun on 2021/8/8.
//

#ifndef PSU_PSU_SSCRECEIVER_H_
#define PSU_PSU_SSCRECEIVER_H_
#include "OSNReceiver.h"
#include "OSNSender.h"
#include "MPOPRFSender.h"
//#include "cryptoTools/Common/CuckooIndex.h"
#include "CuckooIndex.h"
#include "cryptoTools/Common/Timer.h"
#include <PSUReceiver.h>
class SSCReceiver : public PSUReceiver
{
	oc::Timer* timer;
	size_t sender_set_size;
	size_t receiver_set_size;

	size_t shuffle_size;

	std::vector<oc::block> receiver_set;
	std::vector<oc::block> after_cuckoo_set;

	//SimpleIndex simple;
	size_t cuckoo_max_bin_size;
	std::vector<std::vector<oc::block>> simple;

	oc::MPOPRFSender mp_oprf_sender;
	OSNSender osn_sender;

	OSNReceiver osn_receiver;

	std::vector<oc::block> runPermuteShareSender(std::vector<oc::Channel>& chls);
	std::vector<oc::block> runPermuteShareReceiver(const std::vector<oc::block>& x_set, std::vector<oc::Channel>& chls);
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

#endif //PSU_PSU_SSCRECEIVER_H_
