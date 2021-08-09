#pragma once

#include <vector>

#include <cryptoTools/Common/BitVector.h>

class Benes
{
	std::vector<int> perm;
	std::vector<int> inv_perm;
	std::vector<std::vector<int>> switched;

	static const int benes_size = 1.27 * (1 << 20);
	std::vector<char> path;

	void DFS(int idx, int route);
	void f_propagate(int size, int baseline, int level, std::vector<uint64_t>& source,
		std::vector<uint64_t>& dest);
	void r_propagate(int size, int baseline, int level, std::vector<uint64_t>& source,
		std::vector<uint64_t>& dest);
	void fwd_propagate(int size, int baseline, int level, std::vector<uint64_t>& source,
		std::vector<uint64_t>& dest, std::vector<osuCrypto::block>& ot_msgs);
	void rev_propagate(int size, int baseline, int level, std::vector<uint64_t>& source,
		std::vector<uint64_t>& dest, std::vector<osuCrypto::block>& ot_msgs);
	void gen_benes_eval(int n, int lvl_p, int perm_idx, std::vector<uint64_t>& src);
public:

	void initialize(int values, int levels);

	std::vector<uint64_t> evaluate(int N, std::vector<uint64_t>& inputs);

	std::vector<uint64_t> masked_evaluate(int N, std::vector<uint64_t>& inputs,
		std::vector<osuCrypto::block> ot_output);
	void benes_route(int n, int lvl_p, int perm_idx, const std::vector<int>& src,
		const std::vector<int>& dest);

	osuCrypto::BitVector return_switches(int N);

	void gen_benes_route(int n, int lvl_p, int perm_idx, const std::vector<int>& src,
		const std::vector<int>& dest);

	void gen_benes_eval(int n, int lvl_p, int perm_idx, std::vector<oc::block>& src);

	void gen_benes_masked_evaluate(int n, int lvl_p, int perm_idx, std::vector<oc::block>& src,
		std::vector<std::vector<std::array<osuCrypto::block, 2>>>& ot_output);

	osuCrypto::BitVector return_gen_benes_switches(int values);
};