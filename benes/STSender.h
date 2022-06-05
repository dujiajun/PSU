#pragma once

#include <vector>
#include "OPVSender.h"

namespace osuCrypto
{
	class STSender
	{
		Timer* timer;
		size_t set_size;
		OPVSender opv_sender;
	public:
		STSender();
		void setParams(size_t set_size, block& seed);
		STSender(size_t set_size, block& seed);
		std::vector<block> output(const std::vector<size_t>& permutation, Channel& chl, std::vector<block>& ot_msgs, BitVector& choices, size_t offset);
		void setTimer(Timer& timer);
	};

}
