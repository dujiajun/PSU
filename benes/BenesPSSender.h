#pragma once
#include "STSender.h"
#include "BenesNetwork.h"

namespace osuCrypto
{
	class BenesPSSender
	{
		block seed;
		Timer* timer;
		size_t set_size;
		size_t log2_permutation_size;
		std::vector<IknpOtExtReceiver> ot_receivers;
		std::vector<STSender> st_senders;
	public:
		BenesPSSender();
		void setParams(size_t set_size, size_t log2_permutation_size, block& seed);
		BenesPSSender(size_t log2_set_size, size_t log2_permutation_size, block& seed);
		std::vector<block> output(BenesNetwork& network, size_t chl_st, size_t chl_ab, size_t chl_mw, std::vector<Channel>& chls);
		void setTimer(Timer& timer);
	};

}