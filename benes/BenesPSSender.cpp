#include "BenesPSSender.h"
using namespace std;
using namespace osuCrypto;

osuCrypto::BenesPSSender::BenesPSSender()
{
}

void osuCrypto::BenesPSSender::setParams(size_t set_size, size_t log2_permutation_size, block& seed)
{
	this->seed = seed;
	this->set_size = set_size;
	this->log2_permutation_size = log2_permutation_size;
	size_t log2_set_size = log2ceil(set_size);   //当前set size二进制表示的位数
}

BenesPSSender::BenesPSSender(size_t set_size, size_t log2_permutation_size, block& seed) : set_size(set_size), log2_permutation_size(log2_permutation_size)
{
	setParams(set_size, log2_permutation_size, seed);
}

vector<block> BenesPSSender::output(BenesNetwork& network, size_t chl_st, size_t chl_ab, size_t chl_mw, std::vector<Channel>& chls)
{
	/*************** 获得每个小delta，组成每个layer的Delta ****************/

	vector<vector<block>> delta(network.layer_counts, vector<block>(set_size)); //行数等于层数，每一行是每一层的delta
	size_t num_threads = chls.size();
	vector<thread> thrds(num_threads);
	st_senders.resize(num_threads);
	ot_receivers.resize(num_threads);
	auto routine = [&](size_t tid)
	{
		vector<block> ot_msgs;
		BitVector choices;
		PRNG prng(seed);
		for (size_t l = 1; l <= network.layers.size(); l++)
		{ //遍历每一层
			size_t offset = 0;

			for (size_t i = tid; i < network.layers[l - 1].permutation_counts; i += num_threads)
			{ //遍历一层中的每个置换
				size_t st_size = network.layers[l - 1].permutations[i].size;
				offset += st_size * log2ceil(st_size);
			}
			ot_msgs.resize(offset);
			choices.resize(offset);
			offset = 0;
			for (size_t i = tid; i < network.layers[l - 1].permutation_counts; i += num_threads)
			{ //遍历一层中的每个置换
				auto& after = network.layers[l - 1].permutations[i].permutation_map;
				OPVSender::calc_choices(after, choices, offset);

				size_t st_size = network.layers[l - 1].permutations[i].size;
				offset += st_size * log2ceil(st_size);
			}

			ot_receivers[tid].receive(choices, ot_msgs, prng, chls[tid]);

			offset = 0;
			for (size_t i = tid; i < network.layers[l - 1].permutation_counts; i += num_threads)
			{ //遍历一层中的每个置换
				size_t st_size = network.layers[l - 1].permutations[i].size;
				auto& after = network.layers[l - 1].permutations[i].permutation_map;//用于把permutation转为映射，permutation中存的是位置的移动，如(2，3，1，0)意思是(0->2,1->3,2->1,3->0),所以(0,1,2,3)置换为(3,2,0,1)
				st_senders[tid].setParams(st_size, seed);
				vector<block> d = st_senders[tid].output(after, chls[tid], ot_msgs, choices, offset); //得到每个小置换的delta，对应小置换相应的id
				for (size_t k = 0; k < d.size(); k++)
				{
					delta[l - 1][network.layers[l - 1].permutations[i].ids[k]] = d[k]; //把每个小置换的delta填充到该layer对应的delta，根据id来填充
				}
				offset += st_size * log2ceil(st_size);
			}
		}
	};

	//size_t j = 0; //用于标记当前到哪个permutation

	for (size_t t = 0; t < num_threads; t++)
		thrds[t] = std::thread(routine, t);
	for (size_t t = 0; t < num_threads; t++)
		thrds[t].join();

	timer->setTimePoint("sender finish ST");
	//cout << "sender finish ST" << endl;
	/***************获得d-1个a^new-b^old向量********************/
	//cout << "sender begin to wait for sigma" << endl;
	vector<block> sigma((network.layer_counts - 1) * set_size); //行数等于层数-1，每一行是每个sigma向量
	if (chl_ab > 0)
		chls[0].recv(sigma);
	//timer->setTimePoint("after receiving sigam");
	//cout << "sender finish receiving sigma" << endl;
	/*************** m 和 w 向量********************/
	//cout << "sender begin to wait for m w" << endl;
	vector<block> mw(2 * (set_size)); //行数等于2，第0行是m向量，第1行是w向量

	chls[0].recv(mw);

	//timer->setTimePoint("after receiving m w");
	//cout << "sender finish receiving m w" << endl;
	/******************计算最终的Delta向量*********************/
	//cout << "sender begin computing final Delta" << endl;
	vector<block> final_Delta(set_size);
	vector<block> sigma_delta(set_size, ZeroBlock);
	vector<block> tmp(set_size);
	vector<size_t> pi_i(set_size);
	for (size_t k = 1; k < network.layer_counts; k++)
	{
		//timer->setTimePoint("layer begin");
		pi_i = network.layers[k].layer_perm;  //当前layer所对应的置换，pi_i=\pi[i]
		for (size_t i = 0; i < set_size; i++)
		{
			//size_t pi = network.layers[k].pi()[i];
			tmp[i] = _mm_xor_si128(_mm_xor_si128(sigma[(k - 1) * set_size + pi_i[i]], delta[k - 1][pi_i[i]]), sigma_delta[pi_i[i]]);
		}
		for (size_t i = 0; i < set_size; i++)
		{
			sigma_delta[i] = tmp[i];
		}
		//timer->setTimePoint("layer finish");
	}
	for (size_t i = 0; i < set_size; i++)
	{
		final_Delta[i] = _mm_xor_si128(delta[network.layer_counts - 1][i], sigma_delta[i]);
	}
	//timer->setTimePoint("after receiving Delta");
	//cout << "sender finish computing final Delta" << endl;
	/******************计算secret share*********************/
	//cout << "sender begin computing Secret share" << endl;
	vector<block> secret_share(set_size);
	auto res = network.output();
	for (size_t i = 0; i < set_size; i++)
	{
		secret_share[i] = _mm_xor_si128(mw[res[i]], _mm_xor_si128(final_Delta[i], mw[set_size + i]));
	}
	//cout << "sender get Secret share" << endl;
	return secret_share;
}

void BenesPSSender::setTimer(Timer& timer)
{
	this->timer = &timer;
	//st_sender.setTimer(timer);
}
