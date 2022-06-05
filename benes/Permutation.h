#pragma once

#include <vector>

namespace osuCrypto
{
	class Permutation
	{
	public:
		size_t size;                 //�û���С
		std::vector<size_t> ids;          //�ڵ���
		std::vector<size_t> permutations; //�û�����
		std::vector<size_t> permutation_map;  //�û���Ϊӳ�䱾��

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
