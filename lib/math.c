/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Mon Nov 26 16:08:05 CST 2012
 */

#include <cavan.h>
#include <cavan/math.h>

u8 *math_memory_shift_left(const u8 *src, u8 *result, size_t size, int shift)
{
	u8 *p, *p_end;
	const u8 *src_end;

	src_end = src + size;
	p_end = result + size;

	for (src += shift, p = result; src < src_end; src++)
	{
		*p++ = *src++;
	}

	while (p < p_end)
	{
		*p++ = 0;
	}

	return result;
}

u8 *math_memory_shift_right(const u8 *src, u8 *result, size_t size, int shift)
{
	u8 *p_result;
	const u8 *p_src;

	for (p_src = src + size - 1 - shift, p_result = result + size - 1; p_src >= src; p_src--, p_result--)
	{
		*p_result = *p_src;
	}

	while (p_result >= result)
	{
		*p_result-- = 0;
	}

	return result;
}

u8 *math_memory_add_single(const u8 *left, const u8 *left_last, u8 right, u8 *result, u8 *result_last)
{
	u16 tmp;
	u8 *result_bak = result;

	while (left <= left_last && right && result <= result_last)
	{
		tmp = left[0] + right;
		result[0] = tmp & 0xFF;

		right = tmp >> 8;
		left++;
		result++;
	}

	while (left <= left_last && result <= result_last)
	{
		*result++ = *left++;
	}

	while (result <= result_last)
	{
		*result++ = 0;
	}

	return result_bak;
}

u8 *math_memory_add(const u8 *left, const u8 *right, u8 *result, size_t size, size_t result_size)
{
	u16 tmp;
	u8 *result_bak = result;
	const u8 *right_last;
	u8 *result_last;

	for (right_last = right + size - 1; right_last >= right && right_last[0] == 0; right_last--);

	result_last = result + result_size - 1;

	for (tmp = 0; right <= right_last && result <= result_last; left++, right++, result++)
	{
		tmp = (tmp >> 8) + left[0] + right[0];
		*result = tmp & 0xFF;
	}

	math_memory_add_single(result, result_last, tmp >> 8, result, result_last);

	return result_bak;
}

u8 *math_memory_mul_single(const u8 *left, const u8 *left_last, u8 right, u8 *result, u8 *result_last)
{
	u16 tmp;
	u8 *result_bak = result;

	while (left_last >= left && left_last[0] == 0)
	{
		left_last++;
	}

	for (tmp = 0; left <= left_last && result <= result_last; left++, result++)
	{
		tmp = (tmp >> 8) + left[0] * right;
		result[0] = tmp & 0xFF;
	}

	if (result <= result_last)
	{
		*result++ = tmp >> 8;
	}

	while (result <= result_last)
	{
		*result++ = 0;
	}

	return result_bak;
}

u8 *math_text2memory(const char *text, u8 *mem, size_t mem_size, int base)
{
	int value;
	u8 *mem_last;

	if (base < 2)
	{
		base = prefix2base(text, &text);
		if (base < 0)
		{
			base = 10;
		}
	}

	mem_last = mem + mem_size - 1;
	mem_set(mem, 0, mem_size);

	while (1)
	{
		value = char2value(*text);
		if (value < 0 || value >= base)
		{
			break;
		}

		math_memory_mul_single(mem, mem_last, base, mem, mem_last);
		math_memory_add_single(mem, mem_last, value, mem, mem_last);

		text++;
	}

	return mem;
}

u8 *math_memory_mul(const u8 *left, const u8 *right, u8 *result, size_t size, size_t result_size)
{
	const u8 *left_last, *right_last;
	u8 buff[result_size], *buff_last;

	for (left_last = left + size - 1; left_last >= left && left_last[0] == 0; left_last--);

	for (right_last = right + size - 1; right_last >= right && right_last[0] == 0; right_last--);

	mem_set(result, 0, result_size);
	buff_last = buff + sizeof(buff) - 1;

	while (right_last >= right)
	{
		math_memory_mul_single(left, left_last, *right_last, buff, buff_last);
		math_memory_shift_right(result, result, result_size, 1);
		math_memory_add(result, buff, result, result_size, result_size);

		right_last--;
	}

	return result;
}
