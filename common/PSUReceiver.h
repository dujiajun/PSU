#pragma once
#include "utils.h"
#include "cryptoTools/Common/Defines.h"
#include "cryptoTools/Common/Timer.h"
class PSUReceiver
{
protected:
	Context context;
	oc::Timer* timer;
	size_t shuffle_size;
	std::vector<oc::block> receiver_set;
public:
	void setContext(const Context& context)
	{
		this->context = context;
	}
	virtual void setReceiverSet(const std::vector<oc::block>& receiver_set, size_t sender_size) = 0;
	virtual std::vector<oc::block> output(std::vector<oc::Channel>& chls) = 0;
	virtual void setTimer(oc::Timer& timer) = 0;
};