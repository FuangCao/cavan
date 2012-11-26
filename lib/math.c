/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Mon Nov 26 16:08:05 CST 2012
 */

#include <cavan.h>
#include <cavan/math.h>

void math_memory_copy(u8 *dest, const u8 *src, size_t dest_size, size_t src_size)
{
	if (dest < src)
	{
		u8 *dest_end = dest + dest_size;
		const u8 *src_end = src + src_size;

		while (src < src_end && dest < dest_end)
		{
			*dest++ = *src++;
		}

		while (dest <= dest_end)
		{
			*dest++ = 0;
		}
	}
	else if (dest > src)
	{
		u8 *pdest = dest + dest_size - 1;
		const u8 *psrc = src + src_size - 1;

		while (psrc >= src && pdest >= dest)
		{
			*pdest-- = *psrc--;
		}

		while (pdest >= dest)
		{
			*pdest-- = 0;
		}
	}
}

u8 *math_memory_shift_left(u8 *mem, size_t size, size_t shift)
{
	if (shift > size)
	{
		shift = size;
	}

	math_memory_copy(mem, mem + shift, size, size - shift);

	return mem;
}

u8 *math_memory_shift_left2(const u8 *mem, u8 *result, size_t size, size_t shift)
{
	if (shift > size)
	{
		shift = size;
	}

	math_memory_copy(result, mem + shift, size, size - shift);

	return result;
}

u8 *math_memory_shift_right(u8 *mem, size_t size, size_t shift)
{
	if (shift > size)
	{
		shift = size;
	}

	math_memory_copy(mem + shift, mem, size - shift, size - shift);
	mem_set(mem, 0, shift);

	return mem;
}

u8 *math_memory_shift_right2(const u8 *mem, u8 *result, size_t size, size_t shift)
{
	if (shift > size)
	{
		shift = size;
	}

	math_memory_copy(result + shift, mem, size - shift, size - shift);
	mem_set(result, 0, shift);

	return result;
}

// ================================================================================

u8 *math_memory_add_single(u8 *mem, u8 *mem_end, u8 value)
{
	u16 tmp;
	u8 *mem_bak = mem;

	while (mem < mem_end && value)
	{
		tmp = value + *mem;
		*mem = tmp & 0xFF;

		value = tmp >> 8;
		mem++;
	}

	return mem_bak;
}

u8 *math_memory_add_single2(u8 *mem, size_t size, u8 value)
{
	math_memory_add_single(mem, mem + size, value);

	return mem;
}

u8 *math_memory_add_single3(const u8 *mem, u8 *result, size_t mem_size, size_t result_size, u8 value)
{
	math_memory_copy(result, mem, result_size, mem_size);
	math_memory_add_single2(result, result_size, value);

	return result;
}

u8 *math_memory_add(u8 *left, u8 *left_end, const u8 *right, const u8 *right_last)
{
	u16 tmp;
	u8 *left_bak = left;

	for (tmp = 0; left < left_end && right <= right_last; left++, right++)
	{
		tmp = *left + *right + (tmp >> 8);
		*left = tmp & 0xFF;
	}

	math_memory_add_single(left, left_end, tmp >> 8);

	return left_bak;
}

u8 *math_memory_add2(u8 *left, const u8 *right, size_t lsize, size_t rsize)
{
	const u8 *right_last;

	for (right_last = right + rsize - 1; right_last >= right && *right_last == 0; right_last--);

	math_memory_add(left, left + lsize, right, right_last);

	return left;
}

u8 *math_memory_add3(const u8 *left, const u8 *right, u8 *result, size_t size, size_t result_size)
{
	math_memory_copy(result, left, result_size, size);
	math_memory_add2(result, right, result_size, size);

	return result;
}

// ================================================================================

u8 *math_memory_mul_single(u8 *mem, u8 *mem_last, u8 *mem_end, u8 value)
{
	u16 tmp;
	u8 *mem_bak = mem;

	for (tmp = 0; mem <= mem_last; mem++)
	{
		tmp = *mem * value + (tmp >> 8);
		*mem = tmp & 0xFF;
	}

	if (mem < mem_end)
	{
		*mem = tmp >> 8;
	}

	return mem_bak;
}

u8 *math_memory_mul_single2(u8 *mem, size_t size, u8 value)
{
	u8 *mem_last;

	for (mem_last = mem + size - 1; mem_last >= mem && *mem_last == 0; mem_last--);

	math_memory_mul_single(mem, mem_last, mem + size, value);

	return mem;
}

u8 *math_memory_mul_single3(const u8 *mem, u8 *result, size_t mem_size, size_t result_size, u8 value)
{
	math_memory_copy(result, mem, result_size, mem_size);
	math_memory_mul_single2(result, result_size, value);

	return result;
}

u8 *math_text2memory(const char *text, u8 *mem, size_t mem_size, int base)
{
	int value;

	value = prefix2base(text, &text);
	if (value > 1)
	{
		base = value;
	}
	else if (base < 2)
	{
		base = 10;
	}

	mem_set(mem, 0, mem_size);

	while (1)
	{
		value = char2value(*text);
		if (value < 0 || value >= base)
		{
			break;
		}

		math_memory_mul_single2(mem, mem_size, base);
		math_memory_add_single2(mem, mem_size, value);

		text++;
	}

	return mem;
}

u8 *math_memory_mul(const u8 *left, const u8 *right, u8 *result, size_t size, size_t result_size)
{
	const u8 *right_last;
	u8 buff[result_size];

	for (right_last = right + size - 1; right_last >= right && *right_last == 0; right_last--);

	mem_set(result, 0, result_size);

	while (right_last >= right)
	{
		math_memory_mul_single3(left, buff, size, sizeof(buff), *right_last);
		math_memory_shift_right(result, result_size, 1);
		math_memory_add2(result, buff, result_size, result_size);

		right_last--;
	}

	return result;
}

u8 *math_memory_mul2(u8 *left, const u8 *right, size_t lsize, size_t rsize)
{
	u8 result[lsize];

	math_memory_mul(left, right, result, lsize, rsize);
	math_memory_copy(left, result, lsize, lsize);

	return left;
}
