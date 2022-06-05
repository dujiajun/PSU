#pragma once

#include <vector>

namespace osuCrypto
{
	class Permutation
	{
	public:
		size_t size;                 //置换大小
		std::vector<size_t> ids;          //节点编号
		std::vector<size_t> permutations; //置换本身
		std::vector<size_t> permutation_map;  //置换改为映射本身

		Permutation();
		Permutation(size_t size);
		void setSize(size_t size);
		void setIds(const std::vector<size_t>& ids);
		void randomShuffle();
		std::vector<size_t> output();
		std::vector<size_t> tPi();
		void print();
	};

}
