#pragma once

#include <cstddef>
#include <cstdint>

// used in shuffle receiver using simple index
// used in shuffle sender using cuckoo index when balanced set
inline size_t get_simple_bin_size(size_t size)
{
	if (size <= (1ull << 8))
	{
		return 28;
	}
	else if (size <= (1ull << 10))
	{
		return 28;
	}
	else if (size <= (1ull << 12))
	{
		return 28;
	}
	else if (size <= (1ull << 14))
	{
		return 29;
	}
	else if (size <= (1ull << 16))
	{
		return 29;
	}
	else if (size <= (1ull << 18))
	{
		return 30;
	}
	else if (size <= (1ull << 20))
	{
		return 31;
	}
	else //if (receiver_size <= (1ull << 22))
	{
		return 31;
	}
}

// used in shuffle sender using cuckoo index when unbalanced set
// receiver will put elements in simple hash table using their cuckoo hashes
inline size_t get_simple_bin_size(size_t sender_size, size_t receiver_size)
{
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
		else //if (receiver_size <= 1ull << 24)
		{
			return 10488;
		}
	}
	else //if (sender_size >= 1ull << 8)
	{
		if (receiver_size <= 1ull << 16)
		{
			return 804;
		}
		else if (receiver_size <= 1ull << 20)
		{
			return 10426;
		}
		else //if (receiver_size <= 1ull << 24)
		{
			return 167808;
		}
	}
}
