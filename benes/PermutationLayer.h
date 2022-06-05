#pragma once
#include <vector>
#include "Permutation.h"
using namespace std;

namespace osuCrypto
{
	class PermutationLayer
	{
	public:
		size_t layer_id;                   //��ǰ�ǵڼ��㣨ֻ��ǰһ�벻ͬ����һ��ʹ����ͬ��id��
		bool middle;                       //��ǰ���Ƿ����м�㣬����middle=1������middle=0
		size_t item_counts;                //�ж��ٸ�Ԫ��
		size_t permutation_size;            //ÿ��С�û�������Ԫ�ظ���
		size_t permutation_counts;          //С�û�����������item_counts/permutation_size

		size_t log2_item_counts;           //�ж��ٸ�Ԫ�أ�������
		size_t log2_permutation_size;       //ÿ��С�û�������Ԫ�ظ�����������

		vector<Permutation> permutations;   //С�û�
		vector<size_t> layer_perm;          //��ǰ���ӳ��

		PermutationLayer();
		PermutationLayer(size_t log2_item_counts, size_t log2_permutation_size, size_t log2_middle_permutation_size, size_t layer_id, bool middle);
		void setParams(size_t log2_item_counts, size_t log2_permutation_size, size_t layer_id, bool middle);
		void setArbitraryParams(size_t item_counts, size_t log2_permutation_size, size_t layer_id, bool middle);
		void randomShuffle();
		vector<size_t> output();
		vector<size_t> pi();
		void print();
	};
}
