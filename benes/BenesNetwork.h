#pragma once
#include <vector>
#include "PermutationLayer.h"

using namespace std;

namespace osuCrypto
{
	class BenesNetwork
	{
	public:
		size_t layer_counts;               //有多少层

		size_t item_counts;                //有多少个元素
		size_t permutaion_size;            //每个小置换包含的元素个数
		size_t middle_permutation_size;    //中间层每个小置换包含的元素个数

		size_t log2_item_counts;           //有多少个元素（对数）
		size_t log2_permutaion_size;       //每个小置换包含的元素个数（对数）
		size_t log2_middle_permutation_size;    //中间层每个小置换包含的元素个数(对数)

		vector<PermutationLayer> layers;

		BenesNetwork();
		void setParams(size_t item_counts, size_t log2_permutaion_size);
		BenesNetwork(size_t item_counts, size_t log2_permutaion_size);
		void randomShuffle();
		pair<size_t, size_t> numberPermutation(); //第一个元素是非中间层之外的置换个数，第二个元素是中间层的置换个数
		vector<size_t> output();
		void print();
	};
}
