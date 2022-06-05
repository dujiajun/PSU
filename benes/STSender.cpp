#include "STSender.h"

using namespace osuCrypto;
using namespace std;
osuCrypto::STSender::STSender()
{
}
void osuCrypto::STSender::setParams(size_t set_size, block& seed)
{
	this->set_size = set_size;
	opv_sender.setParams(set_size, seed);
}
osuCrypto::STSender::STSender(size_t set_size, block& seed)
	:set_size(set_size), opv_sender(set_size, seed)
{
}

std::vector<block> osuCrypto::STSender::output(const std::vector<size_t>& permutation, Channel& chl, std::vector<block>& ot_msgs, BitVector& choices, size_t offset)
{
	//timer->setTimePoint("OPV init");
	opv_sender.init(permutation);
	//timer->setTimePoint("OPV finish init");
	vector<vector<block> >  matrix(set_size);
	for (size_t i = 0; i < set_size; i++)
	{
		matrix[i] = opv_sender.output(i, chl, ot_msgs, choices, offset);
	}
	//timer->setTimePoint("has received matrix");
	vector<block> a(set_size);
	vector<block> b(set_size);
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

	vector<block> delta(set_size);
	for (size_t i = 0; i < set_size; i++)
	{
		delta[i] = _mm_xor_si128(a[permutation[i]], b[i]);
	}
	//timer->setTimePoint("finish compute delta");
	return delta;
}

void osuCrypto::STSender::setTimer(Timer& timer)
{
	this->timer = &timer;
	opv_sender.setTimer(timer);
}
