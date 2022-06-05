#include "Permutation.h"

#include <iostream>
#include <cassert>
#include <algorithm>
using namespace std;
using namespace osuCrypto;

Permutation::Permutation()
{
	size = 0;
}

Permutation::Permutation(size_t size)
{
	setSize(size);
}

void Permutation::setSize(size_t size)
{
	this->size = size;
	ids.resize(size);
	permutations.resize(size);
	for (size_t i = 0; i < size; i++)
	{
		permutations[i] = i;
		ids[i] = i;
	}
}

void Permutation::setIds(const vector<size_t>& ids)
{
	assert(size == ids.size());
	this->ids = ids;
}

void Permutation::randomShuffle()
{
	random_shuffle(permutations.begin(), permutations.end());
	permutation_map = tPi();
}

std::vector<size_t> Permutation::output()
{
	vector<size_t> results(size);
	for (size_t i = 0; i < size; i++)
		results[i] = ids[permutations[i]];
	return results;
}

std::vector<size_t> Permutation::tPi()
{
	vector<size_t> after(size);
	
	for (size_t i = 0; i < size; i++)
		after[permutations[i]]=i;
	return after;
}

void Permutation::print()
{
	auto res = output();
	for (size_t i = 0; i < size; i++)
		cout << "Location: " << ids[i] << " -> " << "Location: " << ids[permutations[i]] << endl;
	cout << endl;
}