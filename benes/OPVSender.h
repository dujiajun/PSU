#pragma once

#include <iostream>

#include <libOTe/TwoChooseOne/IknpOtExtSender.h>
#include <libOTe/TwoChooseOne/IknpOtExtReceiver.h>

#include <cryptoTools/Common/Defines.h>
#include <cryptoTools/Network/Session.h>
#include <cryptoTools/Network/Channel.h>
#include <cryptoTools/Network/IOService.h>
#include <cryptoTools/Common/Timer.h>
#include <libOTe/Base/naor-pinkas.h>

namespace osuCrypto
{
	class OPVSender
	{
	private:
		AES aes;
		size_t set_size;
		//PRNG prng;
		Timer* timer;
		//IknpOtExtReceiver ot_receiver;
		std::vector<block> tree;
		size_t ot_cnt;
		//std::vector<block> ot_msgs;
		//BitVector choices;
		size_t level;
		std::vector<block> v;
		std::vector<size_t> indexes;

		block get(size_t level, size_t i);
		void build_tree(size_t index);
		block get_prf(const block& x, int which);
		void print_tree(size_t index);
		block calc_tree_node(size_t level, const block& msg, int which, size_t index);
	public:
		OPVSender();
		void setParams(size_t set_size, block& seed);
		OPVSender(size_t set_size, block& seed);
		void init(const std::vector<size_t>& indexes);
		std::vector<block> output(size_t which, Channel& chl, std::vector<block>& ot_msgs, BitVector& choices, size_t offset);
		void tranverse();
		void setTimer(Timer& timer);
		static void calc_choices(const std::vector<size_t>& indexes, BitVector& choices, size_t offset);
	};

}