#pragma once

#include <vector>
#include <string>
#include "cryptoTools/Common/Defines.h"
#include "cryptoTools/Common/BitVector.h"
#include "cryptoTools/Common/Timer.h"
#include "cryptoTools/Network/Channel.h"
#include "benes.h"
class OSNSender
{
	size_t size;
	int ot_type;
	oc::Timer* timer;

	Benes benes;
	void silent_ot_recv(osuCrypto::BitVector& choices,
		std::vector<osuCrypto::block>& recvMsg,
		std::vector<oc::Channel>& chls);
	void rand_ot_recv(osuCrypto::BitVector& choices,
		std::vector<osuCrypto::block>& recvMsg,
		std::vector<oc::Channel>& chls);
	std::vector<std::array<osuCrypto::block, 2>> gen_benes_server_osn(int values, std::vector<oc::Channel>& chls);
 public:
	std::vector<int> dest;
	OSNSender(size_t size = 0, int ot_type = 0);
	void init(size_t size, int ot_type = 0, const std::string& osn_cache = "");
	std::vector<oc::block> run_osn(std::vector<oc::Channel>& chls);
	void setTimer(oc::Timer& timer);
};

