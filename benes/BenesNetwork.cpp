#include "BenesNetwork.h"
#include <cryptoTools/Common/Defines.h>
#include <iostream>
using namespace std;
using namespace osuCrypto;

osuCrypto::BenesNetwork::BenesNetwork()
{
}

void osuCrypto::BenesNetwork::setParams(size_t item_counts, size_t log2_permutaion_size)
{
	this->item_counts = item_counts;
	permutaion_size = 1ull << log2_permutaion_size;

	size_t log2_item_counts = log2ceil(item_counts);
	layer_counts = 2 * ((log2_item_counts + log2_permutaion_size - 1) / log2_permutaion_size) - 1;
	layers.resize(layer_counts);
	int layer_m = (layer_counts + 1) / 2; //中间layer的id
	for (size_t i = 1; i <= layer_counts; i++)
	{
		if (i != layer_m)
			layers[i - 1].setArbitraryParams(this->item_counts, log2_permutaion_size, (i < (layer_counts + 1) / 2) ? i : layer_counts - i + 1, 0); //最后一个参数用于标识是否是中间层
		else
			layers[i - 1].setArbitraryParams(this->item_counts, log2_permutaion_size, i, 1);
	}
}

osuCrypto::BenesNetwork::BenesNetwork(size_t item_counts, size_t log2_permutaion_size)
{
	setParams(item_counts, log2_permutaion_size);
}

void osuCrypto::BenesNetwork::randomShuffle()
{
	for (auto& layer : layers)
		layer.randomShuffle();
}
std::pair<size_t, size_t> osuCrypto::BenesNetwork::numberPermutation()
{
	//除中间层之外的置换个数
	//中间层的置换个数
	return std::make_pair((layer_counts - 1) * item_counts / permutaion_size, item_counts / middle_permutation_size);
}
std::vector<size_t> osuCrypto::BenesNetwork::output()
{
	vector<size_t> before(item_counts), after(item_counts);
	for (size_t i = 0; i < item_counts; i++)
	{
		before[i] = i;
		after[i] = i;
	}
	for (auto& layer : layers)
	{
		vector<size_t> permutation = layer.output();
		for (size_t i = 0; i < permutation.size(); i++)
		{
			after[permutation[i]] = before[i];
		}
		for (size_t i = 0; i < item_counts; i++)
		{
			before[i] = after[i];
		}
	}
	return after;
}

void osuCrypto::BenesNetwork::print()
{
	for (auto& layer : layers)
		layer.print();
}