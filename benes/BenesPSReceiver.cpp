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
	/**************����ÿ��С a b, ����ϳ�ĳһ�������A B*****************/
	//cout << "receiver begin ST" << endl;
	vector<vector<block>> A(network.layer_counts, vector<block>(set_size)); //�������ڲ�����ÿһ����ÿһ���a����
	vector<vector<block>> B(network.layer_counts, vector<block>(set_size)); //�������ڲ�����ÿһ����ÿһ���b����

	size_t num_threads = chls.size();
	vector<thread> thrds(num_threads);
	st_receivers.resize(num_threads);
	ot_senders.resize(num_threads);
	auto routine = [&](size_t tid)
	{
		vector<array<block, 2>> ot_msgs;
		PRNG prng(seed);
		for (size_t l = 1; l <= network.layer_counts; l++)
		{//����ÿһ��

			size_t offset = 0;

			for (size_t i = tid; i < network.layers[l - 1].permutation_counts; i += num_threads)
			{ //����һ���е�ÿ���û�
				size_t st_size = network.layers[l - 1].permutations[i].size;
				offset += st_size * log2ceil(st_size);
			}
			ot_msgs.resize(offset);
			ot_senders[tid].send(ot_msgs, prng, chls[tid]);
			offset = 0;
			for (size_t i = tid; i < network.layers[l - 1].permutation_counts; i += num_threads)
			{ //����һ���е�ÿ���û�
				//j = j + 1;
				size_t st_size = network.layers[l - 1].permutations[i].size;
				vector<block> a(permutation_size);
				vector<block> b(permutation_size);
				st_receivers[tid].setParams(st_size, seed);
				//timer->setTimePoint("receiver begin receive a,b");
				st_receivers[tid].output(a, b, chls[tid], ot_msgs, offset); //�õ�ÿ��С�û���a,b��������ӦС�û���Ӧ��id

				for (size_t k = 0; k < a.size(); k++)
				{
					//cout <<"layer-"<<l<< "permuatation-" << i << "-receiver id: " << network.layers[l - 1].permutations[i].ids[k] << endl;
					A[l - 1][network.layers[l - 1].permutations[i].ids[k]] = a[k]; //��ÿ��С�û���a��䵽��layer��Ӧ��a������id�����
					B[l - 1][network.layers[l - 1].permutations[i].ids[k]] = b[k]; //��ÿ��С�û���b��䵽��layer��Ӧ��b������id�����
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
	/***************���d-1��a^new-b^old����,������********************/
	//cout << "receiver begin computing sigma" << endl;
	vector<block> sigma((network.layer_counts - 1) * set_size); //�������ڲ���-1��ÿһ����ÿ��sigma����

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
	/***************����m �� w����,������********************/
	vector<block> mw(2 * (set_size));

	PRNG prng(oc::toBlock(123));
	for (size_t i = 0; i < set_size; i++)
	{
		mw[i] = _mm_xor_si128(A[0][i], x_set[i]);
		mw[set_size + i] = prng.get<block>();
	}

	chls[0].send(mw);

	/******************����secret share*********************/
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