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
	class OPVReceiver
	{
	private:
		size_t set_size;
		PRNG prng;
		AES aes;
		//IknpOtExtSender ot_sender;
		std::vector<block> tree;
		size_t ot_cnt;
		//std::vector<block> msgs;
		//BitVector choices;
		size_t level;
		std::vector<block> v;
		//std::vector<std::array<block, 2>> ot_base;

		block get(size_t level, size_t i);
		void build_tree(size_t index);
		block get_prf(const block& x, int which);
		void print_tree(size_t index);
		void prepare_ot_base(size_t ot_cnt, std::vector<std::array<block, 2>>& ot_base);
	public:
		OPVReceiver();
		void setParams(size_t set_size, block& seed);
		OPVReceiver(size_t set_size, block& seed);
		void init();
		std::vector<block> output(size_t which, Channel& chl, std::vector<std::array<block, 2>>& ot_msgs, size_t offset);
		void tranverse();
		void setTimer(Timer& timer);
	};
}


