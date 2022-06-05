#include "BenesPSReceiver.h"

using namespace std;
using namespace osuCrypto;

osuCrypto::BenesPSReceiver::BenesPSReceiver()
{
}

void osuCrypto::BenesPSReceiver::setParams(size_t set_size, size_t log2_permutation_size, block& seed)
{
	this->seed = seed;
	this->set_size = set_size;
	this->log2_permutation_size = log2_permutation_size;
}

osuCrypto::BenesPSReceiver::BenesPSReceiver(size_t set_size, size_t log2_permutation_size, block& seed) : log2_set_size(log2_set_size), log2_permutation_size(log2_permutation_size)
{
	setParams(set_size, log2_permutation_size, seed);
}

std::vector<block> osuCrypto::BenesPSReceiver::output(BenesNetwork& network, const std::vector<block>& x_set, size_t chl_st, size_t chl_ab, size_t chl_mw, std::vector<Channel>& chls)
{
	/**************计算每个小 a b, 并组合成某一层的向量A B*****************/
	//cout << "receiver begin ST" << endl;
	vector<vector<block>> A(network.layer_counts, vector<block>(set_size)); //行数等于层数，每一行是每一层的a向量
	vector<vector<block>> B(network.layer_counts, vector<block>(set_size)); //行数等于层数，每一行是每一层的b向量

	size_t num_threads = chls.size();
	vector<thread> thrds(num_threads);
	st_receivers.resize(num_threads);
	ot_senders.resize(num_threads);
	auto routine = [&](size_t tid)
	{
		vector<array<block, 2>> ot_msgs;
		PRNG prng(seed);
		for (size_t l = 1; l <= network.layer_counts; l++)
		{//遍历每一层

			size_t offset = 0;

			for (size_t i = tid; i < network.layers[l - 1].permutation_counts; i += num_threads)
			{ //遍历一层中的每个置换
				size_t st_size = network.layers[l - 1].permutations[i].size;
				offset += st_size * log2ceil(st_size);
			}
			ot_msgs.resize(offset);
			ot_senders[tid].send(ot_msgs, prng, chls[tid]);
			offset = 0;
			for (size_t i = tid; i < network.layers[l - 1].permutation_counts; i += num_threads)
			{ //遍历一层中的每个置换
				//j = j + 1;
				size_t st_size = network.layers[l - 1].permutations[i].size;
				vector<block> a(permutation_size);
				vector<block> b(permutation_size);
				st_receivers[tid].setParams(st_size, seed);
				//timer->setTimePoint("receiver begin receive a,b");
				st_receivers[tid].output(a, b, chls[tid], ot_msgs, offset); //得到每个小置换的a,b向量，对应小置换相应的id

				for (size_t k = 0; k < a.size(); k++)
				{
					//cout <<"layer-"<<l<< "permuatation-" << i << "-receiver id: " << network.layers[l - 1].permutations[i].ids[k] << endl;
					A[l - 1][network.layers[l - 1].permutations[i].ids[k]] = a[k]; //把每个小置换的a填充到该layer对应的a，根据id来填充
					B[l - 1][network.layers[l - 1].permutations[i].ids[k]] = b[k]; //把每个小置换的b填充到该layer对应的b，根据id来填充
				}

				offset += st_size * log2ceil(st_size);
			}
		}
	};

	for (size_t t = 0; t < num_threads; t++)
		thrds[t] = std::thread(routine, t);
	for (size_t t = 0; t < num_threads; t++)
		thrds[t].join();

	timer->setTimePoint("receiver finish ST");
	//cout << "receiver finish ST" << endl;
	/***************获得d-1个a^new-b^old向量,并发送********************/
	//cout << "receiver begin computing sigma" << endl;
	vector<block> sigma((network.layer_counts - 1) * set_size); //行数等于层数-1，每一行是每个sigma向量

	for (size_t i = 0; i < chl_ab; i++)
	{
		for (size_t k = 0; k < set_size; k++)
		{
			sigma[i * set_size + k] = _mm_xor_si128(A[i + 1][k], B[i][k]);
		}
	}
	if (chl_ab > 0)
		chls[0].send(sigma);
	//timer->setTimePoint("receiver finish computing sigma");
	//cout << "receiver finish computing sigma" << endl;
	/***************计算m 和 w向量,并发送********************/
	vector<block> mw(2 * (set_size));

	PRNG prng(oc::toBlock(123));
	for (size_t i = 0; i < set_size; i++)
	{
		mw[i] = _mm_xor_si128(A[0][i], x_set[i]);
		mw[set_size + i] = prng.get<block>();
	}

	chls[0].send(mw);

	/******************计算secret share*********************/
	vector<block> secret_share(set_size);
	for (size_t i = 0; i < set_size; i++)
	{
		secret_share[i] = _mm_xor_si128(mw[set_size + i], B[network.layer_counts - 1][i]);
	}
	return secret_share;
}

void osuCrypto::BenesPSReceiver::setTimer(Timer& timer)
{
	this->timer = &timer;
	//st_receiver.setTimer(timer);
}