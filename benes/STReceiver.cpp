#include "STReceiver.h"

using namespace osuCrypto;
using namespace std;
osuCrypto::STReceiver::STReceiver()
{
}
void osuCrypto::STReceiver::setParams(size_t set_size, block& seed)
{
	this->set_size = set_size;
	opv_receiver.setParams(set_size, seed);
}
osuCrypto::STReceiver::STReceiver(size_t set_size, block& seed)
	:set_size(set_size), opv_receiver(set_size, seed)
{
}

void osuCrypto::STReceiver::output(std::vector<block>& a, std::vector<block>& b, Channel& chl,std::vector<std::array<block, 2>>& ot_msgs, size_t offset)
{
	opv_receiver.init();
	vector<vector<block> >  matrix(set_size);
	for (size_t i = 0; i < set_size; i++)
	{
		matrix[i] = opv_receiver.output(i, chl, ot_msgs, offset);
	}

	a.resize(set_size);
	b.resize(set_size);
	for (size_t i = 0; i < set_size; i++)
	{
		a[i] = matrix[0][i];
		b[i] = matrix[i][0];
		for (size_t j = 1; j < set_size; j++)
		{
			a[i] = _mm_xor_si128(matrix[j][i], a[i]);
			b[i] = _mm_xor_si128(matrix[i][j], b[i]);
		}
	}
}

void osuCrypto::STReceiver::setTimer(Timer& timer)
{
	this->timer = &timer;
	opv_receiver.setTimer(timer);
}
