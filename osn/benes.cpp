#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <vector>
#include <stack>
#include <cryptoTools/Common/Defines.h>
#include <cryptoTools/Common/BitVector.h>
#include "benes.h"

using namespace std;
using namespace oc;


void Benes::initialize(int values, int levels)
{
	path.resize(benes_size);
	perm.resize(values);
	inv_perm.resize(values);
	switched.resize(levels);
	for (int i = 0; i < levels; ++i)
		switched[i].resize(values / 2);
}

osuCrypto::BitVector Benes::return_switches(int N)
{
	int values = 1 << N;
	int levels = 2 * N - 1;
	osuCrypto::BitVector switches(values * levels / 2);
	for (int j = 0; j < levels; ++j)
		for (int i = 0; i < values / 2; ++i)
		{
			switches[(values * j) / 2 + i] = switched[j][i];
			// std::cout<<" . "<<int(switched[i][j])<<" -> "<<switches[values*j+i];
		}
	return switches;
}

void Benes::DFS(int idx, int route)
{
	stack<pair<int, int>> st;
	st.push({idx, route});
	pair<int, int> pr;
	while (!st.empty())
	{
		pr = st.top();
		st.pop();
		path[pr.first] = pr.second;
		if (path[pr.first ^ 1] < 0) // if the next item in the vertical array is unassigned
			st.push({pr.first ^ 1,
					 pr.second ^ 1}); /// the next item is always assigned the opposite of this item,
									  /// unless it was part of path/cycle of previous node

		idx = perm[inv_perm[pr.first] ^ 1];
		if (path[idx] < 0)
			st.push({idx, pr.second ^ 1});
	}
}

int shuffle(int i, int n) { return ((i & 1) << (n - 1)) | (i >> 1); }

void Benes::benes_route(int n, int lvl_p, int perm_idx, const vector<int> &src, const vector<int> &dest)
{
	int values, levels, i, j, x, s;
	vector<int> bottom1;
	vector<int> top1;

	/*std::cout << "in level p = " << lvl_p << std::endl;
	std::cout<< "permutation index = " << perm_idx << std::endl; */
	if (n == 1)
	{
		switched[lvl_p][perm_idx] = src[0] != dest[0];
		return;
	}

	values = 1 << n;
	// std::cout << "values is set to " << values << std::endl;
	levels = 2 * n - 1;
	// std::cout << "number of levels is set to " << levels << std::endl;

	vector<int> bottom2(values / 2);
	vector<int> top2(values / 2);

	/*std::cout << "in level p = " << lvl_p << std::endl;
	for (i = 0; i < values; ++i) {
	  std::cout << "src[i] = " << src[i] << endl;
	}

	for (i = 0; i < values; ++i) {
	  std::cout << "dest[i] = " << dest[i] << endl;
	}*/

	for (i = 0; i < values; ++i)
	{
		inv_perm[src[i]] = i;
		// std::cout << "inv_perm" << src[i] << inv_perm[src[i]] << std::endl;
	}

	for (i = 0; i < values; ++i)
	{
		perm[i] = inv_perm[dest[i]];
		// std::cout << "perm" << i << perm[i] << std::endl;
	}

	for (i = 0; i < values; ++i)
	{
		inv_perm[perm[i]] = i;
		// std::cout << "inv_perm" << perm[i] << i << std::endl;
	}
	fill(path.begin(), path.end(), -1);
	//memset(path, -1, sizeof(path));
	// std::cout << "size of the path " << sizeof(path) << std::endl;
	for (i = 0; i < values; ++i)
		if (path[i] < 0)
			DFS(i, 0);

	for (i = 0; i < values; i += 2)
	{
		switched[lvl_p][perm_idx + i / 2] = path[i];
		for (j = 0; j < 2; ++j)
		{
			x = shuffle((i | j) ^ path[i], n);
			if (x < values / 2)
				bottom1.push_back(src[i | j]);
			else
				top1.push_back(src[i | j]);
		}
	}

	for (i = 0; i < values; i += 2)
	{
		s = switched[lvl_p + levels - 1][perm_idx + i / 2] = path[perm[i]];
		for (j = 0; j < 2; ++j)
		{
			x = shuffle((i | j) ^ s, n);
			if (x < values / 2)
				bottom2[x] = src[perm[i | j]];
			else
				top2[x - values / 2] = src[perm[i | j]];
		}
	}

	benes_route(n - 1, lvl_p + 1, perm_idx, bottom1, bottom2);
	benes_route(n - 1, lvl_p + 1, perm_idx + values / 4, top1, top2);
	return;
}

// baseline: is in terms of the values, not switches
// size: is in terms of the wires/values, not switches

void Benes::f_propagate(int size, int baseline, int level, vector<uint64_t> &source,
				 vector<uint64_t> &dest)
{
	// std::cout << "in fwd propagate " << std::endl;
	int switch_count = size / 2;

	for (int j = 0; j < switch_count; j++)
	{
		if (switched[level][baseline / 2 + j] == 0)
		{
			dest[baseline + j] = source[baseline + 2 * j];
			dest[baseline + size / 2 + j] = source[baseline + 2 * j + 1];
		}
		else
		{
			dest[baseline + j] = source[baseline + 2 * j + 1];
			dest[baseline + size / 2 + j] = source[baseline + 2 * j];
		}
	}
}

void Benes::r_propagate(int size, int baseline, int level, vector<uint64_t> &source,
				 vector<uint64_t> &dest)
{
	// std::cout << "in rev propagate " << std::endl;
	int switch_count = size / 2;
	osuCrypto::block temp_block;
	uint64_t temp_int[2];
	for (int j = 0; j < switch_count; j++)
	{
		// temp_block = ot_msgs.at(0);
		// ot_msgs.erase(ot_msgs.begin());
		if (switched[level][baseline / 2 + j] == 0)
		{
			dest[baseline + 2 * j] = source[baseline + j];
			dest[baseline + 2 * j + 1] = source[baseline + size / 2 + j];
		}
		else
		{
			dest[baseline + 2 * j] = source[baseline + size / 2 + j];
			dest[baseline + 2 * j + 1] = source[baseline + j];
		}
	}
}

void Benes::fwd_propagate(int size, int baseline, int level, vector<uint64_t> &source,
				   vector<uint64_t> &dest, vector<osuCrypto::block> &ot_msgs)
{
	// std::cout << "in fwd propagate " << std::endl;
	int switch_count = size / 2;
	osuCrypto::block temp_block;
	uint64_t temp_int[2];
	for (int j = 0; j < switch_count; j++)
	{
		temp_block = ot_msgs.at(0);
		ot_msgs.erase(ot_msgs.begin());
		memcpy(temp_int, &temp_block, sizeof(temp_int));

		if (switched[level][baseline / 2 + j] == 0)
		{
			dest[baseline + j] = source[baseline + 2 * j] ^ temp_int[0];
			dest[baseline + size / 2 + j] = source[baseline + 2 * j + 1] ^ temp_int[1];
		}
		else
		{
			dest[baseline + j] = source[baseline + 2 * j + 1] ^ temp_int[1];
			dest[baseline + size / 2 + j] = source[baseline + 2 * j] ^ temp_int[0];
		}
	}
}

void Benes::rev_propagate(int size, int baseline, int level, vector<uint64_t> &source,
				   vector<uint64_t> &dest, vector<osuCrypto::block> &ot_msgs)
{
	// std::cout << "in rev propagate " << std::endl;
	int switch_count = size / 2;
	osuCrypto::block temp_block;
	uint64_t temp_int[2];
	for (int j = 0; j < switch_count; j++)
	{
		temp_block = ot_msgs.at(0);
		memcpy(temp_int, &temp_block, sizeof(temp_int));
		ot_msgs.erase(ot_msgs.begin());
		if (switched[level][baseline / 2 + j] == 0)
		{
			dest[baseline + 2 * j] = source[baseline + j] ^ temp_int[0];
			dest[baseline + 2 * j + 1] = source[baseline + size / 2 + j] ^ temp_int[1];
		}
		else
		{
			dest[baseline + 2 * j] = source[baseline + size / 2 + j] ^ temp_int[1];
			dest[baseline + 2 * j + 1] = source[baseline + j] ^ temp_int[0];
		}
	}
}

vector<uint64_t> Benes::masked_evaluate(int N, vector<uint64_t> &inputs,
								 vector<osuCrypto::block> ot_output)
{
	std::cout << "in masked eval " << std::endl;

	int values = 1 << N;
	int levels = 2 * N - 1;
	int size = values;
	int baseline_count = 1;
	vector<uint64_t> temp(inputs.size());
	vector<osuCrypto::block> ot_masks = ot_output;
	int toggle = 0;
	// forward
	for (int j = 0; j < levels / 2; j++)
	{
		baseline_count = pow(2, j);
		size = values / baseline_count; // you have the size and can figure the baselines

		if (toggle % 2 == 0)
		{
			for (int k = 0; k < baseline_count; k++)
			{
				fwd_propagate(size, k * size, j, inputs, temp, ot_masks);
			}
			toggle++;
		}
		else
		{
			for (int k = 0; k < baseline_count; k++)
			{
				fwd_propagate(size, k * size, j, temp, inputs, ot_masks);
			}
			toggle++;
		}
	}

	//-----------------------------------middle layer ----------------------------------------
	osuCrypto::block temp_block;
	uint64_t temp_int[2];
	if (toggle % 2 == 0)
	{
		for (int j = 0; j < values / 2; j++)
		{
			temp_block = ot_masks.at(0);
			ot_masks.erase(ot_masks.begin());
			memcpy(temp_int, &temp_block, sizeof(temp_int));
			if (switched[levels / 2][j] == 1)
			{
				temp[2 * j + 1] = inputs[2 * j] ^ temp_int[0];
				temp[2 * j] = inputs[2 * j + 1] ^ temp_int[1];
				/*std::cout << "flip" << std::endl;
				std::cout << "m0 xor w1 " << temp_int[0] << std::endl;
				std::cout << "m1 xor w0 " << temp_int[1] << std::endl;
				std::cout << "in benes: output wire 0 " << inputs[2 * j] << std::endl;
				std::cout << "in benes: output wire 1 " << inputs[2 * j + 1] << std::endl;*/
			}
			else
			{
				temp[2 * j + 1] = inputs[2 * j + 1] ^ temp_int[1];
				temp[2 * j] = inputs[2 * j] ^ temp_int[0];
				/*std::cout << "m0 xor w0 " << temp_int[0] << std::endl;
				std::cout << "m1 xor w1 " << temp_int[1] << std::endl;
				std::cout << "in benes: output wire 0 " << inputs[2 * j] << std::endl;
				std::cout << "in benes: output wire 1 " << inputs[2 * j + 1] << std::endl;*/
			}
		}

		toggle++;
	}
	else
	{
		for (int j = 0; j < values / 2; j++)
		{
			temp_block = ot_masks.at(0);
			ot_masks.erase(ot_masks.begin());
			memcpy(temp_int, &temp_block, sizeof(temp_int));
			if (switched[levels / 2][j] == 1)
			{
				inputs[2 * j + 1] = temp[2 * j] ^ temp_int[0];
				inputs[2 * j] = temp[2 * j + 1] ^ temp_int[1];
				/*std::cout << "flip" << std::endl;
				std::cout << "m0 xor w1 " << temp_int[0] << std::endl;
				std::cout << "m1 xor w0 " << temp_int[1] << std::endl;
				std::cout << "in benes: output wire 0 " << inputs[2 * j] << std::endl;
				std::cout << "in benes: output wire 1 " << inputs[2 * j + 1] << std::endl;*/
			}
			else
			{
				inputs[2 * j + 1] = temp[2 * j + 1] ^ temp_int[1];
				inputs[2 * j] = temp[2 * j] ^ temp_int[0];
				/*std::cout << "m0 xor w0 " << temp_int[0] << std::endl;
				std::cout << "m1 xor w1 " << temp_int[1] << std::endl;
				std::cout << "in benes: output wire 0 " << inputs[2 * j] << std::endl;
				std::cout << "in benes: output wire 1 " << inputs[2 * j + 1] << std::endl;*/
			}
		}

		toggle++;
	}
	//--------------------------------------------end middle layer-------------------------------

	for (int j = levels / 2 - 1; j >= 0; j--)
	{
		baseline_count = pow(2, j);
		size = values / baseline_count; // you have the size and can figure the baselines

		if (toggle % 2 == 0)
		{
			for (int k = 0; k < baseline_count; k++)
			{
				rev_propagate(size, k * size, levels - (j + 1), inputs, temp, ot_masks);
			}

			toggle++;
		}
		else
		{
			for (int k = 0; k < baseline_count; k++)
			{
				rev_propagate(size, k * size, levels - (j + 1), temp, inputs, ot_masks);
			}
			toggle++;
		}
	}
	// std::cout << "in benes/evaluate()" << std::endl;
	if (toggle % 2 == 0)
	{
		// for (int i = 0; i < values; i ++)
		// std::cout << inputs[i] << std::endl;
		return inputs;
	}
	else
	{
		// for (int i = 0; i < values; i ++)
		// std::cout << temp[i] << std::endl;
		return temp;
	}
}

vector<uint64_t> Benes::evaluate(int N, vector<uint64_t> &inputs)
{
	int values = 1 << N;	// get some value n
	int levels = 2 * N - 1; // can compute levels
	int size = values;
	int baseline_count = 1;
	vector<uint64_t> temp(inputs.size());
	int toggle = 0;
	// forward
	for (int j = 0; j < levels / 2; j++)
	{
		baseline_count = pow(2, j);
		size = values / baseline_count; // you have the size and can figure the baselines

		if (toggle % 2 == 0)
		{
			for (int k = 0; k < baseline_count; k++)
			{
				f_propagate(size, k * size, j, inputs, temp);
			}
			toggle++;
		}
		else
		{
			for (int k = 0; k < baseline_count; k++)
			{
				f_propagate(size, k * size, j, temp, inputs);
			}
			toggle++;
		}
	}

	if (toggle % 2 == 0)
	{
		for (int j = 0; j < values / 2; j++)
		{
			if (switched[levels / 2][j] == 1)
			{
				temp[2 * j + 1] = inputs[2 * j];
				temp[2 * j] = inputs[2 * j + 1];
			}
			else
			{
				temp[2 * j + 1] = inputs[2 * j + 1];
				temp[2 * j] = inputs[2 * j];
			}
		}

		toggle++;
	}
	else
	{
		for (int j = 0; j < values / 2; j++)
		{
			if (switched[levels / 2][j] == 1)
			{
				inputs[2 * j + 1] = temp[2 * j];
				inputs[2 * j] = temp[2 * j + 1];
			}
			else
			{
				inputs[2 * j + 1] = temp[2 * j + 1];
				inputs[2 * j] = temp[2 * j];
			}
		}

		toggle++;
	}

	for (int j = levels / 2 - 1; j >= 0; j--)
	{
		baseline_count = pow(2, j);
		size = values / baseline_count; // you have the size and can figure the baselines

		if (toggle % 2 == 0)
		{
			for (int k = 0; k < baseline_count; k++)
			{
				r_propagate(size, k * size, levels - (j + 1), inputs, temp);
			}

			toggle++;
		}
		else
		{
			for (int k = 0; k < baseline_count; k++)
			{
				r_propagate(size, k * size, levels - (j + 1), temp, inputs);
			}
			toggle++;
		}
	}
	// std::cout << "in benes/evaluate()" << std::endl;
	if (toggle % 2 == 0)
	{
		// for (int i = 0; i < values; i ++)
		// std::cout << inputs[i] << std::endl;
		return inputs;
	}
	else
	{
		// for (int i = 0; i < values; i ++)
		// std::cout << temp[i] << std::endl;
		return temp;
	}
}

// n <- number of layers in the network (initialize as int(ceil(log2(src.size()))) )
void Benes::gen_benes_route(int n, int lvl_p, int perm_idx, const vector<int> &src,
					 const vector<int> &dest)
{
	int levels, i, j, x, s;
	vector<int> bottom1;
	vector<int> top1;
	int values = src.size();
	// printf("values: %d, location: %d %d \n", values,lvl_p,perm_idx);

	/*
	 * daca avem doar un nivel in retea
	 */
	if (values == 2)
	{
		if (n == 1)
			switched[lvl_p][perm_idx] = src[0] != dest[0];
		else
			switched[lvl_p + 1][perm_idx] = src[0] != dest[0];
		return;
	}

	if (values == 3)
	{
		if (src[0] == dest[0])
		{
			switched[lvl_p][perm_idx] = 0;
			switched[lvl_p + 2][perm_idx] = 0;
			if (src[1] == dest[1])
				switched[lvl_p + 1][perm_idx] = 0;
			else
				switched[lvl_p + 1][perm_idx] = 1;
		}

		if (src[0] == dest[1])
		{
			switched[lvl_p][perm_idx] = 0;
			switched[lvl_p + 2][perm_idx] = 1;
			if (src[1] == dest[0])
				switched[lvl_p + 1][perm_idx] = 0;
			else
				switched[lvl_p + 1][perm_idx] = 1;
		}

		if (src[0] == dest[2])
		{
			switched[lvl_p][perm_idx] = 1;
			switched[lvl_p + 1][perm_idx] = 1;
			if (src[1] == dest[0])
				switched[lvl_p + 2][perm_idx] = 0;
			else
				switched[lvl_p + 2][perm_idx] = 1;
		}
		// std::cout<<"based case 3:"<<switched[lvl_p][perm_idx]<<" "<<switched[lvl_p+1][perm_idx]<<"
		// "<<switched[lvl_p+2][perm_idx];
		return;
	}

	/*
	 * aflam dimensiunea retelei benes
	 */
	levels = 2 * n - 1;

	vector<int> bottom2(values / 2);
	vector<int> top2(int(ceil(values * 0.5)));

	/*
	 * destinatia este o permutare a intrari
	 */

	for (i = 0; i < values; ++i)
		inv_perm[src[i]] = i;

	for (i = 0; i < values; ++i)
		perm[i] = inv_perm[dest[i]];

	for (i = 0; i < values; ++i)
		inv_perm[perm[i]] = i;

	/*
	 * cautam sa vedem ce switch-uri vor fi activate in partea
	 * inferioara a retelei
	 */
	fill(path.begin(), path.end(), -1);
	//memset(path, -1, sizeof(path));
	if (values % 2 == 1)
	{
		path[values - 1] = 1;
		path[perm[values - 1]] = 1;
		if (perm[values - 1] != values - 1)
		{
			int idx = perm[inv_perm[values - 1] ^ 1];
			DFS(idx, 0);
		}
	}

	for (i = 0; i < values; ++i)
		if (path[i] < 0)
		{
			DFS(i, 0);
		}

	/*
	 * calculam noile perechi sursa-destinatie
	 * 1 pentru partea superioara
	 * 2 pentru partea inferioara
	 */
	// partea superioara
	for (i = 0; i < values - 1; i += 2)
	{
		switched[lvl_p][perm_idx + i / 2] = path[i];
		for (j = 0; j < 2; ++j)
		{
			x = shuffle((i | j) ^ path[i], n);
			if (x < values / 2)
				bottom1.push_back(src[i | j]);
			else
				top1.push_back(src[i | j]);
			// input_wires[lvl_p][perm_idx + i / 2][j] = src[i | j];
		}
	}
	if (values % 2 == 1)
	{
		top1.push_back(src[values - 1]);
		// cout<<"pushing source: "<<src[values]<<"\n";
	}

	// partea inferioara
	for (i = 0; i < values - 1; i += 2)
	{
		s = switched[lvl_p + levels - 1][perm_idx + i / 2] = path[perm[i]];
		for (j = 0; j < 2; ++j)
		{
			x = shuffle((i | j) ^ s, n);
			if (x < values / 2)
				bottom2[x] = src[perm[i | j]];
			else
			{
				top2[i / 2] = src[perm[i | j]];
				// std:cout<<"top2 index "<<x - values / 2<<" : "<<src[perm[i | j]]<<std::endl;
				// top2[x - values / 2]
			}
			// input_wires[lvl_p + levels - 1][perm_idx + i / 2][j] = src[perm[i | j]];
		}
	}

	int idx = int(ceil(values * 0.5));
	if (values % 2 == 1)
	{
		// for (int i=0; i <= idx-2; ++i)
		//  top2[i] = top2[i+1];
		// printf("top2 added %d at location %d \n",dest[values-1], idx);
		top2[idx - 1] = dest[values - 1];
		// cout<<"pushing destination: "<<dest[values];
	}

	/*
	std::cout<<"sizes of vectors - bottom 1: "<<bottom1.size()
	<<" bottom 2: "<<bottom2.size()<<" top 1: "<<top1.size()
	<<"top 2: "<<top2.size();
	*/

	/*
	 * recursivitate prin partea superioara si inferioara
	 */
	gen_benes_route(n - 1, lvl_p + 1, perm_idx, bottom1, bottom2);
	gen_benes_route(n - 1, lvl_p + 1, perm_idx + values / 4, top1, top2);
}

void Benes::gen_benes_eval(int n, int lvl_p, int perm_idx, vector<uint64_t> &src)
{
	int levels, i, j, x, s;
	vector<uint64_t> bottom1;
	vector<uint64_t> top1;
	int values = src.size();
	uint64_t temp;

	if (values == 2)
	{
		if (n == 1)
		{
			if (switched[lvl_p][perm_idx] == 1)
			{
				temp = src[0];
				src[0] = src[1];
				src[1] = temp;
			}
		}
		else if (switched[lvl_p + 1][perm_idx] == 1)
		{
			temp = src[0];
			src[0] = src[1];
			src[1] = temp;
		}
		return;
	}

	if (values == 3)
	{
		if (switched[lvl_p][perm_idx] == 1)
		{
			temp = src[0];
			src[0] = src[1];
			src[1] = temp;
		}
		if (switched[lvl_p + 1][perm_idx] == 1)
		{
			temp = src[1];
			src[1] = src[2];
			src[2] = temp;
		}
		if (switched[lvl_p + 2][perm_idx] == 1)
		{
			temp = src[0];
			src[0] = src[1];
			src[1] = temp;
		}
		return;
	}

	levels = 2 * n - 1;

	// partea superioara
	for (i = 0; i < values - 1; i += 2)
	{
		int s = switched[lvl_p][perm_idx + i / 2];
		for (j = 0; j < 2; ++j)
		{
			x = shuffle((i | j) ^ s, n);
			if (x < values / 2)
				bottom1.push_back(src[i | j]);
			else
				top1.push_back(src[i | j]);
		}
	}
	if (values % 2 == 1)
	{
		top1.push_back(src[values - 1]);
		// cout<<"pushing source: "<<src[values]<<"\n";
	}

	gen_benes_eval(n - 1, lvl_p + 1, perm_idx, bottom1);
	gen_benes_eval(n - 1, lvl_p + 1, perm_idx + values / 4, top1);

	// partea inferioara
	for (i = 0; i < values - 1; i += 2)
	{
		s = switched[lvl_p + levels - 1][perm_idx + i / 2];
		for (j = 0; j < 2; ++j)
		{
			x = shuffle((i | j) ^ s, n);
			if (x < values / 2)
				src[i | j] = bottom1[x];
			else
			{
				src[i | j] = top1[i / 2];
			}
		}
	}

	int idx = int(ceil(values * 0.5));
	if (values % 2 == 1)
	{
		src[values - 1] = top1[idx - 1];
	}
}

void Benes::gen_benes_masked_evaluate(int n, int lvl_p, int perm_idx, vector<oc::block> &src,
							   vector<vector<array<osuCrypto::block, 2>>> &ot_output)
{
	int levels, i, j, x, s;
	vector<oc::block> bottom1;
	vector<oc::block> top1;
	int values = src.size();
	oc::block temp, temp_int[2];
	std::array<oc::block, 2> temp_block;

	uint64_t Val = ot_output[0].size(); //   number of inputs in the entire benes network

	if (values == 2)
	{
		if (n == 1)
		{
			temp_block = ot_output[lvl_p][perm_idx];
			memcpy(temp_int, &temp_block, sizeof(temp_int));
			src[0] = src[0] ^ temp_int[0];
			src[1] = src[1] ^ temp_int[1];
			if (switched[lvl_p][perm_idx] == 1)
			{
				temp = src[0];
				src[0] = src[1];
				src[1] = temp;
			}
			// std::cout<<"base index: "<<" "<<(lvl_p)*(Val/2)+perm_idx<<" switch = "<<s<<" upper =
			// "<<src[0]<<" "<<" lower = "<<src[1]<<" "<<temp_int[1]<<std::endl;
		}
		else
		{
			temp_block = ot_output[lvl_p + 1][perm_idx];
			memcpy(temp_int, temp_block.data(), sizeof(temp_int));
			src[0] = src[0] ^ temp_int[0];
			src[1] = src[1] ^ temp_int[1];
			if (switched[lvl_p + 1][perm_idx] == 1)
			{
				temp = src[0];
				src[0] = src[1];
				src[1] = temp;
			}
		}
		return;
	}

	if (values == 3)
	{
		temp_block = ot_output[lvl_p][perm_idx];
		memcpy(temp_int, temp_block.data(), sizeof(temp_int));
		src[0] = src[0] ^ temp_int[0];
		src[1] = src[1] ^ temp_int[1];
		if (switched[lvl_p][perm_idx] == 1)
		{
			temp = src[0];
			src[0] = src[1];
			src[1] = temp;
		}

		temp_block = ot_output[lvl_p + 1][perm_idx];
		memcpy(temp_int, temp_block.data(), sizeof(temp_int));
		src[1] = src[1] ^ temp_int[0];
		src[2] = src[2] ^ temp_int[1];
		if (switched[lvl_p + 1][perm_idx] == 1)
		{
			temp = src[1];
			src[1] = src[2];
			src[2] = temp;
		}

		temp_block = ot_output[lvl_p + 2][perm_idx];
		memcpy(temp_int, temp_block.data(), sizeof(temp_int));
		src[0] = src[0] ^ temp_int[0];
		src[1] = src[1] ^ temp_int[1];
		if (switched[lvl_p + 2][perm_idx] == 1)
		{
			temp = src[0];
			src[0] = src[1];
			src[1] = temp;
		}

		return;
	}

	levels = 2 * n - 1;

	// partea superioara
	for (i = 0; i < values - 1; i += 2)
	{
		int s = switched[lvl_p][perm_idx + i / 2];

		temp_block = ot_output[lvl_p][perm_idx + i / 2];
		memcpy(temp_int, temp_block.data(), sizeof(temp_int));

		// std::cout<<"testing index : "<<(lvl_p)*(Val/2)+perm_idx+i/2<<std::endl;
		// std::cout<<" input 1 : "<<src[i]<< " input 2 : "<<src[i^1]<<"ot message :"<<temp_int[0]
		//<<" "<<temp_int[1]<<std::endl;

		src[i] = src[i] ^ temp_int[0];
		src[i ^ 1] = src[i ^ 1] ^ temp_int[1]; //  ^ or | ??

		for (j = 0; j < 2; ++j)
		{
			x = shuffle((i | j) ^ s, n);
			if (x < values / 2)
			{
				bottom1.push_back(src[i | j]);
			}
			else
			{
				top1.push_back(src[i | j]);
			}
		}
	}
	if (values % 2 == 1)
	{
		top1.push_back(src[values - 1]);
		// cout<<"pushing source: "<<src[values]<<"\n";
	}

	gen_benes_masked_evaluate(n - 1, lvl_p + 1, perm_idx, bottom1, ot_output);
	gen_benes_masked_evaluate(n - 1, lvl_p + 1, perm_idx + values / 4, top1, ot_output);

	// std::vector<uint64_t> test_vec(src.size());
	for (i = 0; i < values - 1; i += 2)
	{
		s = switched[lvl_p + levels - 1][perm_idx + i / 2];

		for (j = 0; j < 2; ++j)
		{
			x = shuffle((i | j) ^ s, n);
			if (x < values / 2)
				src[i | j] = bottom1[x];
			else
			{
				src[i | j] = top1[i / 2];
			}
		}

		temp_block = ot_output[lvl_p + levels - 1][perm_idx + i / 2];
		memcpy(temp_int, temp_block.data(), sizeof(temp_int));
		src[i] = src[i] ^ temp_int[s];
		src[i ^ 1] = src[i ^ 1] ^ temp_int[1 - s];
	}

	int idx = int(ceil(values * 0.5));
	if (values % 2 == 1)
	{
		src[values - 1] = top1[idx - 1];
	}
}

osuCrypto::BitVector Benes::return_gen_benes_switches(int values)
{
	int N = int(ceil(log2(values)));
	int levels = 2 * N - 1;
	osuCrypto::BitVector switches(levels * (values / 2));
	for (int j = 0; j < levels; ++j)
		for (int i = 0; i < values / 2; ++i)
		{
			switches[j * (values / 2) + i] = (switched[j][i]);
			// std::cout<<j*(values/2) +i<<" "<<int(switched[j][i])<<" "<<switches[j*(values/2)
			// +i]<<std::endl;
		}
	return switches;
}


