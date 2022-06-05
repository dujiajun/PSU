#pragma once
#include <vector>
#include "PermutationLayer.h"

using namespace std;

namespace osuCrypto
{
	class BenesNetwork
	{
	public:
		size_t layer_counts;               //�ж��ٲ�

		size_t item_counts;                //�ж��ٸ�Ԫ��
		size_t permutaion_size;            //ÿ��С�û�������Ԫ�ظ���
		size_t middle_permutation_size;    //�м��ÿ��С�û�������Ԫ�ظ���

		size_t log2_item_counts;           //�ж��ٸ�Ԫ�أ�������
		size_t log2_permutaion_size;       //ÿ��С�û�������Ԫ�ظ�����������
		size_t log2_middle_permutation_size;    //�м��ÿ��С�û�������Ԫ�ظ���(����)

		vector<PermutationLayer> layers;

		BenesNetwork();
		void setParams(size_t item_counts, size_t log2_permutaion_size);
		BenesNetwork(size_t item_counts, size_t log2_permutaion_size);
		void randomShuffle();
		pair<size_t, size_t> numberPermutation(); //��һ��Ԫ���Ƿ��м��֮����û��������ڶ���Ԫ�����м����û�����
		vector<size_t> output();
		void print();
	};
}
