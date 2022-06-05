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
		size_t layerNum;		 //����
		size_t set_size;		 //���ϴ�С
		size_t permutation_size; //ÿ��С�û�������Ԫ�ظ���
		size_t log2_middle_permutation_size;
		size_t middle_permutation_size; //�м��С�û�������Ԫ�ظ���
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
