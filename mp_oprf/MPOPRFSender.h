#pragma once

#include <iostream>

#include <libOTe/TwoChooseOne/IknpOtExtSender.h>
#include <libOTe/TwoChooseOne/IknpOtExtReceiver.h>
#include <cryptoTools/Crypto/RandomOracle.h>
#include <cryptoTools/Common/Defines.h>
#include <cryptoTools/Common/Timer.h>
#include <libOTe/Base/naor-pinkas.h>
#include "MP-OPRF-Parameters.h"

namespace osuCrypto
{
	class MPOPRFSender
	{
		block commonSeed;
		u64 set_size;
		u64 height;
		u64 logHeight;
		u64 width;
		u64 hashLengthInBytes;
		u64 h1LengthInBytes;
		u64 bucket1;
		u64 bucket2;
		Timer* timer;
		std::vector<std::vector<u8>> matrixC;
	public:
		MPOPRFSender();
		void setTimer(Timer& timer);
		void setParams(const block& commonSeed, const u64& set_size, const u64& logHeight, const u64& width, const u64& hashLengthInBytes, const u64& h1LengthInBytes, const u64& bucket1, const u64& bucket2);
		MPOPRFSender(const block& commonSeed, const u64& set_size, const u64& logHeight, const u64& width, const u64& hashLengthInBytes, const u64& h1LengthInBytes, const u64& bucket1, const u64& bucket2);
		void run(PRNG& prng, std::vector<Channel>& chls);
		std::vector<u8> get_oprf(std::vector<block>& senderSet);
		~MPOPRFSender();
	};
}

