#pragma once
#include "STReceiver.h"
#include "BenesNetwork.h"

namespace osuCrypto
{
	class BenesPSReceiver
	{
		block seed;
		Timer* timer;
		size_t log2_set_size;
		size_t log2_permutation_size;
		size_t layerNum;		 //层数
		size_t set_size;		 //集合大小
		size_t permutation_size; //每个小置换包含的元素个数
		size_t log2_middle_permutation_size;
		size_t middle_permutation_size; //中间层小置换包含的元素个数
		std::vector<IknpOtExtSender> ot_senders;
		std::vector<STReceiver> st_receivers;
	public:
		BenesPSReceiver();
		void setParams(size_t set_size, size_t log2_permutation_size, block& seed);
		BenesPSReceiver(size_t set_size, size_t log2_permutation_size, block& seed);
		std::vector<block> output(BenesNetwork& network, const std::vector<block>& x_set, size_t chl_st, size_t chl_ab, size_t chl_mw, std::vector<Channel>& chls);
		void setTimer(Timer& timer);
	};

}
