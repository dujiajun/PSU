#pragma once

#include "cryptoTools/Common/Defines.h"
#include <cstring>
#include <string>

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

void PrintBuffer(void* pBuff, unsigned int nLen);

struct Context
{
	size_t role;
	size_t psu_type;

	std::string host;
	size_t port;

	size_t sender_size;
	size_t receiver_size;
	size_t num_threads;
	size_t osn_ot_type;
	std::string osn_cache;

	size_t cuckoo_hash_num;
	double cuckoo_scaler;
};