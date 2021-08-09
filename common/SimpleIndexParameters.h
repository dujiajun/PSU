#pragma once

using u64 = unsigned long long;

inline u64 getSimpleBinSize(u64 sender_size, u64 receiver_size)
{
	/*
	if (sender_size >= 1ull << 12)
	{
		if (receiver_size <= 1ull << 16)
		{
			return 98;
		}
		else if (receiver_size <= 1ull << 20)
		{
			return 816;
		}
		else if (receiver_size <= 1ull << 24)
		{
			return 10488;
		}
	}
	else if (sender_size >= 1ull << 8)
	{
		if (receiver_size <= 1ull << 16)
		{
			return 804;
		}
		else if (receiver_size <= 1ull << 20)
		{
			return 10426;
		}
		else if (receiver_size <= 1ull << 24)
		{
			return 167808;
		}
	}*/
	if (receiver_size <= (1ull << 8))
	{
		return 28;
	}
	else if (receiver_size <= (1ull << 10))
	{
		return 28;
	}
	else if (receiver_size <= (1ull << 12))
	{
		return 28;
	}
	else if (receiver_size <= (1ull << 14))
	{
		return 29;
	}
	else if (receiver_size <= (1ull << 16))
	{
		return 29;
	}
	else if (receiver_size <= (1ull << 18))
	{
		return 30;
	}
	else if (receiver_size <= (1ull << 20))
	{
		return 31;
	}
	else //if (receiver_size <= (1ull << 22))
	{
		return 31;
	}
}