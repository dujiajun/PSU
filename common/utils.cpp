//
// Created by dujiajun on 2021/8/8.
//

#include "utils.h"
#include <iostream>
#include <cstring>
// True if a < b, for unsigned 128 bit integers
/*inline bool operator<(__m128i a, __m128i b)
{
	// Flip the sign bits in both arguments.
	// Transforms 0 into -128 = minimum for signed bytes,
	// 0xFF into +127 = maximum for signed bytes
	const __m128i signBits = _mm_set1_epi8((char)0x80);
	a = _mm_xor_si128(a, signBits);
	b = _mm_xor_si128(b, signBits);

	// Now the signed byte comparisons will give the correct order
	const int less = _mm_movemask_epi8(_mm_cmplt_epi8(a, b));
	const int greater = _mm_movemask_epi8(_mm_cmpgt_epi8(a, b));
	return less > greater;
}*/

void PrintBuffer(void* pBuff, unsigned int nLen)
{
	using namespace std;

	if (NULL == pBuff || 0 == nLen)
	{
		return;
	}

	const int nBytePerLine = 16;
	unsigned char* p = (unsigned char*)pBuff;
	char szHex[3 * nBytePerLine + 1] = { 0 };

	for (unsigned int i = 0; i < nLen; ++i)
	{
		int idx = 3 * (i % nBytePerLine);
		if (0 == idx)
		{
			memset(szHex, 0, sizeof(szHex));
		}
#ifdef WIN32
		sprintf_s(&szHex[idx], 4, "%02x ", p[i]);// buff长度要多传入1个字节
#else
		snprintf(&szHex[idx], 4, "%02x ", p[i]); // buff长度要多传入1个字节
#endif

// 以16个字节为一行，进行打印
		if (0 == ((i + 1) % nBytePerLine))
		{
			cout << szHex << endl;
		}
	}

	// 打印最后一行未满16个字节的内容
	if (0 != (nLen % nBytePerLine))
	{
		cout << szHex << endl;
	}
}