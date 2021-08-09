#pragma once
inline size_t get_mp_oprf_width(size_t receiver_size, size_t sender_size)
{
	if (sender_size >= 1ull << 12)
	{
		if (receiver_size <= 1ull << 16)
		{
			return 614;
		}
		else if (receiver_size <= 1ull << 20)
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
		if (receiver_size <= 1ull << 16)
		{
			return 615;
		}
		else if (receiver_size <= 1ull << 20)
		{
			return 627;
		}
		else
		{
			return 639;
		}
	}
}


inline size_t get_mp_oprf_hash_in_bytes(size_t receiver_size, size_t sender_size)
{
	if (sender_size >= 1ull << 12)
	{
		if (receiver_size <= 1ull << 16)
		{
			return 9;
		}
		else if (receiver_size <= 1ull << 20)
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
		if (receiver_size <= 1ull << 16)
		{
			return 9;
		}
		else if (receiver_size <= 1ull << 20)
		{
			return 9;
		}
		else
		{
			return 10;
		}
	}
}

inline size_t get_mp_oprf_width(size_t size)
{
	if (size <= (1 << 8))
	{
		return 610;
	}
	else if (size <= (1 << 10))
	{
		return 622;
	}
	else if (size <= (1 << 12))
	{
		return 633;
	}
	else if (size <= (1 << 14))
	{
		return 645;
	}
	else if (size <= (1 << 16))
	{
		return 656;
	}
	else if (size <= (1 << 18))
	{
		return 667;
	}
	else
	{
		return 678;
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
		return 9;
	}
	else if (size <= (1 << 12))
	{
		return 10;
	}
	else if (size <= (1 << 14))
	{
		return 11;
	}
	else if (size <= (1 << 16))
	{
		return 11;
	}
	else if (size <= (1 << 18))
	{
		return 12;
	}
	else
	{
		return 13;
	}
}