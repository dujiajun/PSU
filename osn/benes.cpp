#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <vector>
#include <stack>
#include <fstream>
#include <cryptoTools/Common/Defines.h>
#include <cryptoTools/Common/BitVector.h>
#include "benes.h"

using namespace std;
using namespace oc;

bool Benes::dump(const std::string& filename)
{
	ofstream out(filename, ios::out | ios::binary);
	if (!out.is_open())
		return false;
	size_t benes_size = path.size();
	size_t values = perm.size();
	size_t levels = switched.size();
	out.write((char*)&benes_size, sizeof(size_t));
	out.write((char*)&values, sizeof(size_t));
	out.write((char*)&levels, sizeof(size_t));
	//out << benes_size << values << levels;
	out.write(path.data(), benes_size * sizeof(path[0]));
	out.write((char*)perm.data(), values * sizeof(perm[0]));
	out.write((char*)inv_perm.data(), values * sizeof(inv_perm[0]));
	
	for (size_t i = 0; i < levels; i++)
	{
		out.write((char*)switched[i].data(), (values / 2) * sizeof(switched[i][0]));
	}
	out.close();
	return true;
}

bool Benes::load(const std::string& filename)
{
	ifstream in(filename, ios::in | ios::binary);
	if (!in.is_open())
		return false;
	size_t benes_size, values, levels;
	in.read((char*)&benes_size, sizeof(size_t));
	in.read((char*)&values, sizeof(size_t));
	in.read((char*)&levels, sizeof(size_t));
	path.resize(benes_size);
	perm.resize(values);
	inv_perm.resize(values);
	switched.resize(levels);
	in.read(path.data(), benes_size * sizeof(path[0]));
	in.read((char*)perm.data(), values * sizeof(perm[0]));
	in.read((char*)inv_perm.data(), values * sizeof(inv_perm[0]));
	for (size_t i = 0; i < levels; i++)
	{
		in.read((char*)switched[i].data(), (values / 2) * sizeof(switched[i][0]));
	}
	in.close();
	return true;
}

void Benes::initialize(int values, int levels)
{
	int benes_size = 1.27 * values;
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


