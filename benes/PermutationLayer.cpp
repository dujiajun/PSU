#include "PermutationLayer.h"

#include <iostream>
using namespace std;
using namespace osuCrypto;

PermutationLayer::PermutationLayer()
{
}

PermutationLayer::PermutationLayer(size_t log2_item_counts, size_t log2_permutation_size, size_t log2_middle_permutation_size, size_t layer_id, bool middle)
{
	setParams(log2_item_counts, log2_permutation_size, layer_id, middle);
}

void PermutationLayer::setParams(size_t log2_item_counts, size_t log2_permutation_size, size_t layer_id, bool middle)
{
	this->log2_item_counts = log2_item_counts;
	this->log2_permutation_size = log2_permutation_size;
	this->layer_id = layer_id;
	this->middle = middle;

	item_counts = 1ull << log2_item_counts;
	if (middle == 0)
	{ //���м��
		permutation_size = 1ull << log2_permutation_size;
		permutation_counts = item_counts / permutation_size;
		permutations.resize(permutation_counts);
		for (size_t i = 0; i < permutation_counts; i++)
		{
			vector<size_t> ids(permutation_size);
			for (size_t x = 0; x < permutation_size; x++) // ������Щ�ڵ�һ�𹹳�С�û���
			{
				size_t tmp1 = i >> (log2_item_counts - layer_id * log2_permutation_size);		   //xǰ��Ĳ���
				size_t tmp2 = i % (1ull << (log2_item_counts - layer_id * log2_permutation_size)); //x����Ĳ���
				ids[x] = (tmp1 << (log2_item_counts - layer_id * log2_permutation_size + log2_permutation_size)) + (x << (log2_item_counts - layer_id * log2_permutation_size)) + tmp2;
			}
			permutations[i].setSize(permutation_size);
			permutations[i].setIds(ids);
		}
	}
	else
	{ //�м��
		size_t log2_middle_permutation_size = log2_item_counts % log2_permutation_size == 0 ? log2_permutation_size : log2_item_counts % log2_permutation_size;
		permutation_size = 1ull << log2_middle_permutation_size;
		permutation_counts = item_counts / permutation_size;
		permutations.resize(permutation_counts);
		for (size_t i = 0; i < permutation_counts; i++)
		{
			vector<size_t> ids(permutation_size);
			for (size_t x = 0; x < permutation_size; x++) // ������Щ�ڵ�һ�𹹳�С�û���
			{
				size_t tmp1 = i << log2_middle_permutation_size; //xǰ��Ĳ��֣���ʱx����û������
				ids[x] = tmp1 + x;
			}
			permutations[i].setSize(permutation_size);
			permutations[i].setIds(ids);
		}
	}
}

void PermutationLayer::setArbitraryParams(size_t item_counts, size_t log2_permutation_size, size_t layer_id, bool middle)
{
	this->item_counts = item_counts;
	this->log2_permutation_size = log2_permutation_size;
	this->layer_id = layer_id;
	this->middle = middle;
	this->log2_item_counts = ceil(log2(item_counts));																  //��ǰset size�����Ʊ�ʾ��λ��
	size_t item_counts_high = (this->item_counts - 1) >> (log2_item_counts - (layer_id - 1) * log2_permutation_size); //item_counts��λ�Ĳ���,x��ߵĲ���
	//size_t item_counts_low = this->item_counts % (1ull << (log2_item_counts - layer_id * log2_permutation_size)); //item_counts�ĵ�λ����,x�ұߵĲ���
	size_t item_counts_xlow = (this->item_counts - 1) % (1ull << (log2_item_counts - (layer_id - 1) * log2_permutation_size)); //item_counts x���Ұ벿��
	if (middle == 0)
	{ //���м��
		//������һ�����м����û�
		permutation_counts = (item_counts_high << (log2_item_counts - layer_id * log2_permutation_size)) + min(item_counts_xlow, ((1ull << (log2_item_counts - layer_id * log2_permutation_size)) - 1)) + 1;
		permutations.resize(permutation_counts);

		for (size_t i = 0; i < permutation_counts; i++)
		{

			size_t permutation_size_max = 1ull << log2_permutation_size;

			vector<size_t> real_ids;
			for (size_t x = 0; x < permutation_size_max; x++) // ������Щ�ڵ�һ�𹹳�С�û���
			{
				size_t tmp1 = i >> (log2_item_counts - layer_id * log2_permutation_size);		   //xǰ��Ĳ���
				size_t tmp2 = i % (1ull << (log2_item_counts - layer_id * log2_permutation_size)); //x����Ĳ���
				size_t id = (tmp1 << (log2_item_counts - layer_id * log2_permutation_size + log2_permutation_size)) + (x << (log2_item_counts - layer_id * log2_permutation_size)) + tmp2;
				if (id < item_counts)
				{ //��������item_counts�Ľڵ㣬��ɽ���Ԫ�ص��û�
					real_ids.push_back(id);
				}
			}
			permutation_size = real_ids.size();
			permutations[i].setSize(permutation_size);
			permutations[i].setIds(real_ids);
		}
	}
	else
	{ //�м��
		size_t log2_middle_permutation_size = log2_item_counts % log2_permutation_size == 0 ? log2_permutation_size : log2_item_counts % log2_permutation_size;
		size_t permutation_size_max = 1ull << log2_middle_permutation_size;
		permutation_counts = item_counts_high + 1;
		permutations.resize(permutation_counts);
		for (size_t i = 0; i < permutation_counts; i++)
		{
			vector<size_t> real_ids;
			for (size_t x = 0; x < permutation_size_max; x++) // ������Щ�ڵ�һ�𹹳�С�û���
			{
				size_t tmp1 = i << log2_middle_permutation_size; //xǰ��Ĳ��֣���ʱx����û������
				size_t id = tmp1 + x;
				if (id < item_counts)
				{ //��������item_counts�Ľڵ㣬��ɽ���Ԫ�ص��û�
					real_ids.push_back(id);
				}
			}
			permutation_size = real_ids.size();
			permutations[i].setSize(permutation_size);
			permutations[i].setIds(real_ids);
		}
	}

}

void PermutationLayer::randomShuffle()
{
	for (auto& permutaion : permutations)
		permutaion.randomShuffle();
	this->layer_perm = pi();
}

std::vector<size_t> PermutationLayer::output()
{
	vector<size_t> results(item_counts);
	for (auto& permutation : permutations)
	{
		vector<size_t> sub_permutation = permutation.output();
		for (size_t i = 0; i < permutation.size; i++)
		{
			results[permutation.ids[i]] = sub_permutation[i];
		}
	}
	return results;
}

std::vector<size_t> PermutationLayer::pi()
{
	vector<size_t> after(item_counts);

	for (size_t i = 0; i < after.size(); i++)
	{
		after[i] = i;
	}
	auto results = output();
	for (size_t i = 0; i < results.size(); i++)
	{
		after[results[i]] = i;
	}
	return after;
}

void PermutationLayer::print()
{
	std::cout << "layer id: " << layer_id << endl;
	for (auto& permutation : permutations)
		permutation.print();
	std::cout << "layer output: ";
	auto results = output();
	for (auto& i : results)
		std::cout << i << " ";
	std::cout << endl
		<< endl;
}