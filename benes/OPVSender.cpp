#include "OPVSender.h"

using namespace osuCrypto;
using namespace std;

void OPVSender::build_tree(size_t index)
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

block OPVSender::get_prf(const block& x, int which)
{
	block result;
	block tmp = _mm_xor_si128(x, which == 1 ? OneBlock : ZeroBlock);
	aes.ecbEncBlock(tmp, result);
	return _mm_xor_si128(result, tmp);
}

void osuCrypto::OPVSender::print_tree(size_t index)
{
	if (index >= tree.size())
		return;
	cout << index << " " << tree[index] << endl;
	print_tree(index * 2);
	print_tree(index * 2 + 1);
}

block OPVSender::calc_tree_node(size_t level, const block& msgs, int which, size_t index)
{
	block blk = msgs;
	size_t start = size_t(1) << level;
	for (size_t i = which; i < start; i += 2)
	{
		if (start + i != index)
		{
			blk = _mm_xor_si128(blk, get(level, i));
		}
	}
	return blk;
}

osuCrypto::OPVSender::OPVSender()
{
}

void osuCrypto::OPVSender::setParams(size_t set_size, block& seed)
{
	this->set_size = set_size;
	aes.setKey(seed);
}

OPVSender::OPVSender(size_t set_size, block& seed) :set_size(set_size), aes(seed)
{

}

void osuCrypto::OPVSender::calc_choices(const std::vector<size_t>& indexes, BitVector& choices, size_t offset)
{
	size_t ot_cnt = log2ceil(indexes.size());
	for (size_t j = 0; j < indexes.size(); j++)
	{
		size_t tmp = indexes[j];
		for (auto i = 0; i < ot_cnt; i++)
		{
			choices[offset + ot_cnt * j + ot_cnt - i - 1] = (tmp & 1) ? 0 : 1;
			tmp = tmp / 2;
		}
	}
}

void OPVSender::init(const std::vector<size_t>& indexes)
{
	ot_cnt = log2ceil(set_size);
	//ot_msgs.resize(ot_cnt * set_size);
	//choices.resize(ot_cnt * set_size);
	level = ot_cnt + 1;

	tree.resize(size_t(1) << level);

	v.resize(set_size);

	this->indexes = indexes;
	//ot_receiver.receive(choices, ot_msgs, prng, chl);
}

std::vector<block> osuCrypto::OPVSender::output(size_t which, Channel& chl, std::vector<block>& ot_msgs, BitVector& choices, size_t offset)
{
	if (set_size == 1)
	{
		chl.recv(v[0]);
		return v;
	}
	tree.clear();
	tree.resize(size_t(1) << level);

	std::vector<block> msgs(ot_msgs.begin() + offset + which * ot_cnt, ot_msgs.begin() + offset + (which + 1) * ot_cnt);
	std::vector<std::array<block, 2>> temp(ot_cnt);
	chl.recv(temp);

	auto iter = offset + which * ot_cnt;
	for (size_t i = 0; i < ot_cnt; i++)
	{
		msgs[i] = msgs[i] ^ temp[i][choices[iter]];
		++iter;
	}

	size_t idx = 1;
	for (size_t i = 0; i < ot_cnt; i++)
	{
		auto l = i + 1;
		auto pos = i;
		idx = idx * 2 + choices[offset + pos + which * ot_cnt];
		tree[idx] = calc_tree_node(l, msgs[pos], choices[offset + pos + which * ot_cnt], idx);
		build_tree(idx);
		idx = (idx / 2) * 2 + (1 - choices[offset + pos + which * ot_cnt]);
	}

	for (size_t i = 0; i < set_size; i++)
	{
		if (i != indexes[which])
			v[i] = get(ot_cnt, i);
		else
			v[i] = ZeroBlock;
	}


	return v;
}

void osuCrypto::OPVSender::tranverse()
{
	//cout << IoStream::lock;
	print_tree(1);
	//cout << IoStream::unlock;
}

void osuCrypto::OPVSender::setTimer(Timer& timer)
{
	//ot_receiver.setTimer(timer);
}

block OPVSender::get(size_t level, size_t i)
{
	return tree[((size_t)1 << level) + i];
}