#pragma once
#include "utils.h"
#include "cryptoTools/Common/Defines.h"
#include "cryptoTools/Common/Timer.h"

class PSUSender
{
protected:
	Context context;
	oc::Timer* timer;
	size_t shuffle_size;
	std::vector<oc::block> sender_set;
public:
	void setContext(const Context& context)
	{
		this->context = context;
	}
	virtual void setSenderSet(const std::vector<oc::block>& sender_set, size_t receiver_size) = 0;
	virtual void output(std::vector<oc::Channel>& chls) = 0;
	virtual void setTimer(oc::Timer& timer) = 0;
};