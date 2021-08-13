#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>

// unbalanced
inline size_t get_mp_oprf_width(size_t small_size, size_t big_size)
{
	if (big_size >= 1ull << 12)
	{
		if (small_size <= 1ull << 16)
		{
			return 614;
		}
		else if (small_size <= 1ull << 20)
		{
			return 626;
		}
		else
		{
			return 638;
		}
	}
	else
	{
		if (small_size <= 1ull << 16)
		{
			return 615;
		}
		else if (small_size <= 1ull << 20)
		{
			return 627;
		}
		else
		{
			return 639;
		}
	}
}


inline size_t get_mp_oprf_hash_in_bytes(size_t small_size, size_t big_size)
{
	if (big_size >= 1ull << 12)
	{
		if (small_size <= 1ull << 16)
		{
			return 9;
		}
		else if (small_size <= 1ull << 20)
		{
			return 10;
		}
		else
		{
			return 10;
		}
	}
	else
	{
		if (small_size <= 1ull << 16)
		{
			return 9;
		}
		else if (small_size <= 1ull << 20)
		{
			return 9;
		}
		else
		{
			return 10;
		}
	}
}

// balanced
inline size_t get_mp_oprf_width(size_t size)
{
	if (size <= (1 << 8))
	{
		return 592;
	}
	else if (size <= (1 << 10))
	{
		return 597;
	}
	else if (size <= (1 << 12))
	{
		return 603;
	}
	else if (size <= (1 << 14))
	{
		return 609;
	}
	else if (size <= (1 << 16))
	{
		return 615;
	}
	else if (size <= (1 << 18))
	{
		return 621;
	}
	else if (size <= (1 << 20))
	{
		return 627;
	}
	else
	{
		return 633;
	}
}

inline size_t get_mp_oprf_hash_in_bytes(size_t size)
{
	if (size <= (1 << 8))
	{
		return 8;
	}
	else if (size <= (1 << 10))
	{
		return 8;
	}
	else if (size <= (1 << 12))
	{
		return 9;
	}
	else if (size <= (1 << 14))
	{
		return 9;
	}
	else if (size <= (1 << 16))
	{
		return 10;
	}
	else if (size <= (1 << 18))
	{
		return 10;
	}
	else if (size <= (1 << 20))
	{
		return 11;
	}
	else
	{
		return 11;
	}
}

inline std::pair<size_t, size_t> getMpOprfParams(size_t small_size, size_t big_size)
{
	size_t width, hashLengthInBytes;
	if (big_size == small_size)
	{
		width = get_mp_oprf_width(small_size);
		hashLengthInBytes = get_mp_oprf_hash_in_bytes(small_size);
	}
	else
	{
		width = get_mp_oprf_width(small_size, big_size);
		hashLengthInBytes = get_mp_oprf_hash_in_bytes(small_size, big_size);
	}
	return std::make_pair(width, hashLengthInBytes);
}