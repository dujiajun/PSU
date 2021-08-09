#pragma once

class PSUSender
{
public:
	virtual void setSenderSet(const std::vector<oc::block>& sender_set, size_t receiver_size) = 0;
	virtual void output(std::vector<oc::Channel>& chls) = 0;
	virtual void setTimer(oc::Timer& timer) = 0;
};