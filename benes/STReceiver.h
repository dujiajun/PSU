#pragma once

#include "OPVReceiver.h"

namespace osuCrypto
{
	class STReceiver
	{
		Timer* timer;
		size_t set_size;
		OPVReceiver opv_receiver;
	public:
		STReceiver();
		void setParams(size_t set_size, block& seed);
		STReceiver(size_t set_size, block& seed);
		void output(std::vector<block> &a, std::vector<block> &b, Channel& chl, std::vector<std::array<block, 2>>& ot_msgs, size_t offset);
		void setTimer(Timer& timer);
	};
}


