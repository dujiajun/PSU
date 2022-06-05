#pragma once
#include <vector>
#include "Permutation.h"
using namespace std;

namespace osuCrypto
{
	class PermutationLayer
	{
	public:
		size_t layer_id;                   //当前是第几层（只有前一半不同，后一半使用相同的id）
		bool middle;                       //当前层是否是中间层，若是middle=1，否则middle=0
		size_t item_counts;                //有多少个元素
		size_t permutation_size;            //每个小置换包含的元素个数
		size_t permutation_counts;          //小置换个数，等于item_counts/permutation_size

		size_t log2_item_counts;           //有多少个元素（对数）
		size_t log2_permutation_size;       //每个小置换包含的元素个数（对数）

		vector<Permutation> permutations;   //小置换
		vector<size_t> layer_perm;          //当前层的映射

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
