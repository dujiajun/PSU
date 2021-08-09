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
	class MPOPRFReceiver
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
	public:
		MPOPRFReceiver();
		void setTimer(Timer& timer);
		void setParams(const block& commonSeed, const u64& set_size, const u64& logHeight, const u64& width, const u64& hashLengthInBytes, const u64& h1LengthInBytes, const u64& bucket1, const u64& bucket2);
		MPOPRFReceiver(const block& commonSeed, const u64& set_size, const u64& logHeight, const u64& width, const u64& hashLengthInBytes, const u64& h1LengthInBytes, const u64& bucket1, const u64& bucket2);
		u8* run(PRNG& prng, std::vector<Channel>& chls, const std::vector<block>& recverSet);
	};
}


