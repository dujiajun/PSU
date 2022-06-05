#include "OPVReceiver.h"

using namespace osuCrypto;
using namespace std;

void OPVReceiver::build_tree(size_t index)
{
	if (index >= tree.size())return;
	if (index * 2 < tree.size())
	{
		tree[2 * index] = get_prf(tree[index], 0);
		build_tree(2 * index);
	}
	if (index * 2 + 1 < tree.size())
	{
		tree[2 * index + 1] = get_prf(tree[index], 1);
		build_tree(2 * index + 1);
	}
}

block OPVReceiver::get_prf(const block& x, int which)
{
	block result;
	block tmp = _mm_xor_si128(x, which == 1 ? OneBlock : ZeroBlock);
	aes.ecbEncBlock(tmp, result);
	return _mm_xor_si128(result, tmp);
}

void OPVReceiver::print_tree(size_t index)
{
	if (index >= tree.size())
		return;
	cout << index << " " << tree[index] << endl;
	print_tree(index * 2);
	print_tree(index * 2 + 1);
}

void OPVReceiver::prepare_ot_base(size_t ot_cnt, vector<array<block, 2>>& ot_base)
{
	for (size_t i = 0; i < ot_cnt; i++)
	{
		auto l = i + 1;
		auto pos = i;
		ot_base[pos][0] = get(l, 0);
		ot_base[pos][1] = get(l, 1);
		for (size_t j = 2; j < (size_t(1) << l); j += 2)
		{
			ot_base[pos][0] = _mm_xor_si128(ot_base[pos][0], get(l, j));
			ot_base[pos][1] = _mm_xor_si128(ot_base[pos][1], get(l, j + 1));
		}
	}
}

osuCrypto::OPVReceiver::OPVReceiver()
{
}

void osuCrypto::OPVReceiver::setParams(size_t set_size, block& seed)
{
	this->set_size = set_size;
	prng.SetSeed(seed);
	aes.setKey(seed);
}

OPVReceiver::OPVReceiver(size_t set_size, block& seed) :set_size(set_size), prng(seed), aes(seed)
{

}

void OPVReceiver::init()
{
	if (set_size < 1)return;
	level = log2ceil(set_size) + 1;
	ot_cnt = level - 1;

	tree.resize(1ull << level);
	tree[1] = prng.get<block>();
	build_tree(1);

	v.resize(set_size);

	//ot_base.resize(ot_cnt * set_size);

	//ot_sender.send(ot_base, prng, chl);
}

std::vector<block> osuCrypto::OPVReceiver::output(size_t which, Channel& chl, std::vector<std::array<block, 2>>& ot_msgs, size_t offset)
{
	if (set_size == 1)
	{
		chl.asyncSend(ZeroBlock);
		v[0] = tree[1];
		return v;
	}
	std::vector<std::array<block, 2>> ot_base_use(ot_cnt);
	prepare_ot_base(ot_cnt, ot_base_use);

	std::vector<std::array<block, 2>> temp(ot_base_use.size());

	for (u64 i = 0; i < static_cast<u64>(ot_base_use.size()); ++i)
	{
		temp[i][0] = ot_msgs[offset + i + which * ot_cnt][0] ^ ot_base_use[i][0];
		temp[i][1] = ot_msgs[offset + i + which * ot_cnt][1] ^ ot_base_use[i][1];
	}

	chl.asyncSend(std::move(temp));

	size_t pos = (size_t)1 << (level - 1);

	for (auto i = 0; i < set_size; i++)
	{
		v[i] = tree[pos + i];
	}

	return v;
}


void osuCrypto::OPVReceiver::tranverse()
{
	//cout << IoStream::lock;
	print_tree(1);
	//cout << IoStream::unlock;
}

void osuCrypto::OPVReceiver::setTimer(Timer& timer)
{
	//ot_sender.setTimer(timer);
}

block OPVReceiver::get(size_t level, size_t i)
{
	return tree[((size_t)1 << level) + i];
}
