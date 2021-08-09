//
// Created by dujiajun on 2021/8/8.
//

#ifndef PSU_PSU_UTILS_H_
#define PSU_PSU_UTILS_H_


#include "cryptoTools/Common/Defines.h"

struct PRF
	{
	oc::u8* data;
	size_t len;
	PRF() : data(nullptr), len(0)
	{
	}
	~PRF()
	{
		delete[]data;
	}
	PRF(size_t len, oc::u8* data)
	{
		this->len = len;
		this->data = new oc::u8[len];
		memcpy(this->data, data, len);
	}
	PRF(const PRF& x) : len(x.len)
	{
		this->data = new oc::u8[x.len];
		memcpy(this->data, x.data, x.len);
	}
	void set(size_t len, oc::u8* data = nullptr)
	{
		this->len = len;
		if (this->data != nullptr)
		{
			delete[] this->data;
			this->data = nullptr;
		}

		if (data != nullptr)
		{
			this->data = new oc::u8[len];
			memcpy(this->data, data, len);
		}

	}
	bool operator<(const PRF& x) const
	{
		return memcmp(data, x.data, len) < 0;
	}
	PRF& operator=(const PRF& x)
		{
		if (this == &x)return *this;
		set(x.len, x.data);
		return *this;
		}
	};

inline bool operator<(__m128i a, __m128i b);

void PrintBuffer(void* pBuff, unsigned int nLen);
#endif //PSU_PSU_UTILS_H_
