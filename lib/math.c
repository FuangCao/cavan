/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Mon Nov 26 16:08:05 CST 2012
 */

#include <cavan.h>
#include <cavan/math.h>

byte *math_memory_shrink(const byte *mem, size_t size)
{
	const byte *mem_last;

	for (mem_last = mem + size - 1; mem_last >= mem && *mem_last == 0; mem_last--);

	return (byte *)mem_last;
}

void math_memory_copy(byte *dest, size_t dest_size, const byte *src, size_t src_size)
{
	if (dest < src)
	{
		byte *dest_end = dest + dest_size;
		const byte *src_end = src + src_size;

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
		byte *pdest = dest + dest_size - 1;
		const byte *psrc = src + src_size - 1;

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

byte *math_text2memory(const char *text, byte *mem, size_t mem_size, int base)
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

		math_memory_mul_single(mem, mem_size, base, NULL, 0);
		math_memory_add_single(mem, mem_size, value, NULL, 0);

		text++;
	}

	return mem;
}

// ================================================================================

void math_memory_shift_left_byte(const byte *mem, size_t mem_size, size_t shift, byte *res, size_t res_size)
{
	if (res == NULL)
	{
		res = (byte *)mem;
		res_size = mem_size;
	}

	if (shift < mem_size)
	{
		math_memory_copy(res, res_size, mem + shift, mem_size - shift);
	}
	else
	{
		mem_set(res, 0, res_size);
	}
}

void math_memory_shift_right_byte(const byte *mem, size_t mem_size, size_t shift, byte *res, size_t res_size)
{
	if (res == NULL)
	{
		res = (byte *)mem;
		res_size = mem_size;
	}

	if (shift < mem_size && shift < res_size)
	{
		math_memory_copy(res + shift, res_size - shift, mem, mem_size - shift);
		mem_set(res, 0, shift);
	}
	else
	{
		mem_set(res, 0, res_size);
	}
}

// ================================================================================

byte math_memory_add_single(const byte *mem, size_t mem_size, byte value, byte *res, size_t res_size)
{
	u16 adder;
	byte *res_end;

	if (res && res != mem)
	{
		const byte *mem_end = mem + mem_size;

		for (res_end = res + res_size; value && mem < mem_end && res < res_end; mem++, res++)
		{
			adder = *mem + value;
			*res = adder & 0xFF;
			value = adder >> 8;
		}

		while (mem < mem_end && res < res_end)
		{
			*res++ = *mem++;
		}

		while (res < res_end)
		{
			*res++ = 0;
		}
	}
	else
	{
		if (res_size == 0)
		{
			res_size = mem_size;
		}

		for (res = (byte *)mem, res_end = res + res_size; value && res < res_end; res++)
		{
			adder = *res + value;
			*res = adder & 0xFF;
			value = adder >> 8;
		}
	}

	return value;
}

byte math_memory_add(const byte *left, size_t lsize, const byte *right, size_t rsize, byte *res, size_t res_size)
{
	u16 adder = 0;
	byte *res_end;
	const byte *right_last = math_memory_shrink(right, rsize);

	if (res && res != left)
	{
		const byte *left_last = math_memory_shrink(left, lsize);

		for (res_end = res + res_size; left <= left_last && right <= right_last && res < res_end; left++, right++, res++)
		{
			adder = *left + *right + (adder >> 8);
			*res = adder & 0xFF;
		}

		adder >>= 8;
		res_size = res_end - res;

		if (left <= left_last)
		{
			adder = math_memory_add_single(left, left_last - left + 1, adder, res, res_size);
		}
		else if (right <= right_last)
		{
			adder = math_memory_add_single(right, right_last - right + 1, adder, res, res_size);
		}
		else
		{
			mem_set(res, 0, res_size);
		}
	}
	else
	{
		if (res_size == 0)
		{
			res_size = lsize;
		}

		for (res = (byte *)left, res_end = res + res_size; right <= right_last && res < res_end; right++, res++)
		{
			adder = *res + *right + (adder >> 8);
			*res = adder & 0xFF;
		}

		adder >>= 8;
	}

	return math_memory_add_single(res, res_end - res, adder, NULL, 0);
}

// ================================================================================

byte math_memory_mul_single(const byte *mem, size_t mem_size, byte value, byte *res, size_t res_size)
{
	u16 adder = 0;
	byte *res_end;
	const byte *mem_last = math_memory_shrink(mem, mem_size);

	if (res && res != mem)
	{
		for (res_end = res + res_size; mem <= mem_last && res < res_end; mem++, res++)
		{
			adder = *mem * value + (adder >> 8);
			*res = adder & 0xFF;
		}

		mem_set(res, 0, res_end - res);
	}
	else
	{
		byte *res_last;

		if (res_size == 0)
		{
			res_size = mem_size;
		}

		res = (byte *)mem;
		res_last = (byte *)mem_last;

		for (res_end = res + res_size; res <= res_last; res++)
		{
			adder = *res * value + (adder >> 8);
			*res = adder & 0xFF;
		}
	}

	return math_memory_add_single(res, res_end - res, adder >> 8, NULL, 0);
}

byte math_memory_mul(const byte *left, size_t lsize, const byte *right, size_t rsize, byte *res, size_t res_size)
{
	byte carry;
	byte *buff;
	size_t buff_size;
	const byte *right_last = math_memory_shrink(right, rsize);
	const byte *left_last = math_memory_shrink(left, lsize);

	lsize = left_last - left + 1;
	rsize = right_last - right + 1;

	buff_size = lsize + 1;
	buff = alloca(buff_size);
	if (buff == NULL)
	{
		return 0xFF;
	}

	if (res == NULL || res == left)
	{
		if ((right >= left && right < left + lsize) || (left >= right && left < right + rsize))
		{
			const byte *right_bak = right;

			right = alloca(rsize);
			if (right == NULL)
			{
				return 0xFF;
			}

			math_memory_copy((byte *)right, rsize, right_bak, rsize);
		}

		math_memory_copy(buff, lsize, left, lsize);

		return math_memory_mul(buff, lsize, right, rsize, (byte *)left, res_size > 0 ? res_size : lsize);
	}

	mem_set(res, 0, res_size);

	for (carry = 0; right_last >= right; right_last--)
	{
		math_memory_shift_right_byte(res, res_size, 1, NULL, 0);

		math_memory_mul_single(left, lsize, *right_last, buff, buff_size);

		carry = math_memory_add(res, res_size, buff, buff_size, NULL, 0);
		if (carry)
		{
			break;
		}
	}

	return carry;
}
