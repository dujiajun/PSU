#include "utils.h"
#include <iostream>
#include <cstring>

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