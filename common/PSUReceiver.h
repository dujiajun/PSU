#pragma once
class PSUReceiver
{
public:
	virtual void setReceiverSet(const std::vector<oc::block>& receiver_set, size_t sender_size) = 0;
	virtual std::vector<oc::block> output(std::vector<oc::Channel>& chls) = 0;
	virtual void setTimer(oc::Timer& timer) = 0;
};