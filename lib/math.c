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

void math_memory_exchange(const byte *mem, byte *res, size_t size)
{
	if (res && res != mem)
	{
		const byte *last;

		for (last = mem + size - 1; last >= mem; res++, last--)
		{
			*res = *last;
		}
	}
	else
	{
		byte tmp;
		byte *last;

		for (res = (byte *)mem, last = res + size - 1; last > res; res++, last--)
		{
			tmp = *last;
			*last = *res;
			*res = tmp;
		}
	}
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

char *math_text2memory(const char *text, byte *mem, size_t mem_size, int base)
{
	int value;
	bool negative;

	while (BYTE_IS_SPACE(*text))
	{
		text++;
	}

	if (*text == '-')
	{
		negative = true;
		text++;
	}
	else
	{
		negative = false;
	}

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

	if (negative)
	{
		math_memory_not(mem, NULL, mem_size);
		math_memory_add_single(mem, mem_size, 1, NULL, 0);
	}

	return (char *)text;
}

char *math_memory2text(const byte *mem, size_t mem_size, char *text, size_t text_size, int base, char fill, size_t size)
{
	char *text_bak, *text_end;
	byte value;
	byte buff[mem_size];
	bool negative;

	if (mem_size > 0 && mem[mem_size - 1] & (1 << 7))
	{
		math_memory_not(mem, buff, mem_size);
		math_memory_add_single(buff, mem_size, 1, NULL, 0);
		negative = true;
	}
	else
	{
		math_memory_copy(buff, mem_size, mem, mem_size);
		negative = false;
	}

	if (base < 2 || base > 24)
	{
		base = 10;
	}

	for (text_bak = text, text_end = text + text_size - 1; text < text_end && mem_size; text++)
	{
		value = math_memory_div_single(buff, mem_size, base, NULL, &mem_size);
		*text = value2char(value);
	}

	if (fill)
	{
		char *fill_end = text_bak + size;
		if (fill_end > text_end)
		{
			fill_end = text_end;
		}

		while (text < fill_end)
		{
			*text++ = fill;
		}
	}

	text = base2prefix_reverse(text, text_end - text, base);

	if (negative && text < text_end)
	{
		*text++ = '-';
	}

	text_reverse_simple(text_bak, text - 1);

	*text = 0;

	return text;
}

void math_memory_show(const char *prompt, const byte *mem, size_t mem_size, int base)
{
	char text[1024];

	math_memory2text(mem, mem_size, text, sizeof(text), base, 0, 0);

	if (prompt == NULL)
	{
		prompt = "Memory";
	}

	println("%s: %s", prompt, text);
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

void math_memory_and(const byte *left, const byte *right, byte *res, size_t size)
{
	const byte *right_end = right + size;

	if (res && res != left)
	{
		while (right < right_end)
		{
			*res = *left & *right;

			res++, left++, right++;
		}
	}
	else
	{
		for (res = (byte *)left; right < right_end; res++, right++)
		{
			*res &= *right;
		}
	}
}

void math_memory_or(const byte *left, const byte *right, byte *res, size_t size)
{
	const byte *right_end = right + size;

	if (res && res != left)
	{
		while (right < right_end)
		{
			*res = *left | *right;

			res++, left++, right++;
		}
	}
	else
	{
		for (res = (byte *)left; right < right_end; res++, right++)
		{
			*res |= *right;
		}
	}
}

void math_memory_not(const byte *mem, byte *res, size_t size)
{
	const byte *mem_end = mem + size;

	if (res && res != mem)
	{
		while (mem < mem_end)
		{
			*res++ = ~(*mem++);
		}
	}
	else
	{
		for (res = (byte *)mem; res < mem_end; res++)
		{
			*res = ~(*res);
		}
	}
}

void math_memory_xor(const byte *left, const byte *right, byte *res, size_t size)
{
	const byte *right_end = right + size;

	if (res && res != left)
	{
		while (right < right_end)
		{
			*res = *left ^ *right;

			res++, left++, right++;
		}
	}
	else
	{
		for (res = (byte *)left; right < right_end; res++, right++)
		{
			*res ^= *right;
		}
	}
}

// ================================================================================

byte math_memory_add_single(const byte *mem, size_t mem_size, byte value, byte *res, size_t res_size)
{
	u16 adder = value;
	byte *res_end;
	const byte *mem_last = math_memory_shrink(mem, mem_size);

	if (res && res != mem)
	{
		for (res_end = res + res_size; adder && mem <= mem_last && res < res_end; mem++, res++)
		{
			adder += *mem;
			*res = adder & 0xFF;
			adder >>= 8;
		}

		while (mem <= mem_last && res < res_end)
		{
			*res++ = *mem++;
		}
	}
	else
	{
		if (res_size == 0)
		{
			res_size = mem_size;
		}

		for (res = (byte *)mem, res_end = res + res_size; adder && res <= mem_last; res++)
		{
			adder += *res;
			*res = adder & 0xFF;
			adder >>= 8;
		}

		res = (byte *)mem_last + 1;
	}

	while (res < res_end && adder)
	{
		*res++ = adder;
		adder >>= 8;
	}

	while (res < res_end)
	{
		*res++ = 0;
	}

	return adder;
}

byte math_memory_add(const byte *left, size_t lsize, const byte *right, size_t rsize, byte *res, size_t res_size)
{
	u16 adder = 0;
	byte *res_end;
	const byte *left_last = math_memory_shrink(left, lsize);
	const byte *right_last = math_memory_shrink(right, rsize);

	if (res == NULL)
	{
		res = (byte *)left;
	}

	if (res_size == 0)
	{
		res_size = lsize;
	}

	for (res_end = res + res_size; left <= left_last && right <= right_last && res < res_end; left++, right++, res++)
	{
		adder += *left + *right;
		*res = adder & 0xFF;
		adder >>= 8;
	}

	if (right <= right_last)
	{
		return math_memory_add_single(right, right_last - right + 1, adder, res, res_end - res);
	}

	if (left <= left_last)
	{
		return math_memory_add_single(left, left_last - left + 1, adder, res, res_end - res);
	}

	if (res < res_end)
	{
		*res++ = adder;

		while (res < res_end)
		{
			*res++ = 0;
		}

		adder = 0;
	}

	return adder;
}

// ================================================================================

int math_memory_cmp_base(const byte *left_last, const byte *right, const byte *right_last)
{
	while (right_last >= right)
	{
		if (*left_last != *right_last)
		{
			if (*left_last > *right_last)
			{
				return 1;
			}
			else
			{
				return -1;
			}
		}

		left_last--;
		right_last--;
	}

	return 0;
}

int math_memory_cmp(const byte *left, size_t lsize, const byte *right, size_t rsize)
{
	const byte *left_last = math_memory_shrink(left, lsize);
	const byte *right_last = math_memory_shrink(right, rsize);

	lsize = left_last - left + 1;
	rsize = right_last - right + 1;

	if (lsize > rsize)
	{
		return 1;
	}
	else if (lsize < rsize)
	{
		return -1;
	}
	else
	{
		return math_memory_cmp_base(left_last, right, right_last);
	}
}

byte math_byte_sub_carry(byte left, byte right, byte carry, byte *res)
{
	if (left < carry)
	{
		left += 0xFF - carry + 1;
		carry = 1;
	}
	else
	{
		left -= carry;
		carry = 0;
	}

	if (left < right)
	{
		left += 0xFF - right + 1;
		carry++;
	}
	else
	{
		left -= right;
	}

	*res = left;

	return carry;
}

byte math_memory_sub_single(const byte *mem, size_t mem_size, byte value, byte carry, byte *res, size_t res_size)
{
	byte *res_end;
	const byte *mem_last = math_memory_shrink(mem, mem_size);

	if (res && res != mem)
	{
		for (res_end = res + res_size; mem <= mem_last && res < res_end && (value || carry); mem++, res++)
		{
			carry = math_byte_sub_carry(*mem, value, carry, res);
			value = 0;
		}

		while (mem <= mem_last && res < res_end)
		{
			*res++ = *mem++;
		}
	}
	else
	{
		if (res_size == 0)
		{
			res_size = mem_size;
		}

		for (res = (byte *)mem, res_end = res + res_size; res <= mem_last && (value || carry); res++)
		{
			carry = math_byte_sub_carry(*res, value, carry, res);
			value = 0;
		}

		res = (byte *)mem_last + 1;
	}

	if (carry || value)
	{
		while (res < res_end && (value || carry != 1))
		{
			carry = math_byte_sub_carry(0, value, carry, res++);
			value = 0;
		}

		while (res < res_end)
		{
			*res++ = 0xFF;
		}
	}
	else
	{
		while (res < res_end)
		{
			*res++ = 0;
		}
	}

	return carry;
}

byte math_memory_sub_single2(byte value, byte carry, const byte *mem, size_t mem_size, byte *res, size_t res_size)
{
	byte *res_end;
	const byte *mem_last = math_memory_shrink(mem, mem_size);

	if (res && res != mem)
	{
		for (res_end = res + res_size; mem <= mem_last && res < res_end && (value || carry != 1); mem++, res++)
		{
			carry = math_byte_sub_carry(value, *mem, carry, &value);
			*res = value;
			value = 0;
		}

		while (mem <= mem_last && res < res_end)
		{
			*res++ = 0xFF - *mem++;
		}
	}
	else
	{
		if (res_size == 0)
		{
			res_size = mem_size;
		}

		res = (byte *)mem;

		for (res_end = res + res_size; res <= mem_last && (value || carry != 1); res++)
		{
			carry = math_byte_sub_carry(value, *res, carry, &value);
			*res = value;
			value = 0;
		}

		while (res <= mem_last)
		{
			*res = 0xFF - *res;
			res++;
		}
	}

	if (value < carry)
	{
		while (res < res_end && (value || carry != 1))
		{
			carry = math_byte_sub_carry(value, 0, carry, &value);
			*res++ = value;
			value = 0;
		}

		while (res < res_end)
		{
			*res++ = 0xFF;
		}
	}
	else
	{
		if (res < res_end)
		{
			*res++ = value - carry;
		}

		while (res < res_end)
		{
			*res++ = 0;
		}

		carry = 0;
	}

	return carry;
}

byte math_memory_sub(const byte *left, size_t lsize, const byte *right, size_t rsize, byte *res, size_t res_size)
{
	byte carry = 0;
	byte *res_end;
	const byte *left_last = math_memory_shrink(left, lsize);
	const byte *right_last = math_memory_shrink(right, rsize);

	if (res && res != left)
	{
		for (res_end = res + res_size; left <= left_last && right <= right_last && res < res_end; left++, right++, res++)
		{
			carry = math_byte_sub_carry(*left, *right, carry, res);
		}

		if (left <= left_last)
		{
			return math_memory_sub_single(left, left_last - left + 1, 0, carry, res, res_end - res);
		}
	}
	else
	{
		if (res_size == 0)
		{
			res_size = lsize;
		}

		for (res = (byte *)left, res_end = res + res_size; right <= right_last && res <= left_last; right++, res++)
		{
			carry = math_byte_sub_carry(*res, *right, carry, res);
		}

		if (res <= left_last)
		{
			return math_memory_sub_single(res, left_last - res + 1, 0, carry, NULL, res_end - res);
		}
	}

	res_size = res_end - res;

	if (right <= right_last)
	{
		return math_memory_sub_single2(0, carry, right, right_last - right + 1, res, res_size);
	}

	return math_memory_sub_single(res, 0, carry, 0, NULL, res_size);
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
			adder += *mem * value;
			*res = adder & 0xFF;
			adder >>= 8;
		}
	}
	else
	{
		if (res_size == 0)
		{
			res_size = mem_size;
		}

		for (res = (byte *)mem, res_end = res + res_size; res <= mem_last; res++)
		{
			adder += *res * value;
			*res = adder & 0xFF;
			adder >>= 8;
		}
	}

	if (res < res_end)
	{
		*res++ = adder;
		adder = 0;
	}

	while (res < res_end)
	{
		*res++ = 0;
	}

	return adder;
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

// ================================================================================

byte math_memory_div_single(const byte *mem, size_t mem_size, byte value, byte *res, size_t *res_size)
{
	u16 divider;
	byte *res_end;
	const byte *mem_last = math_memory_shrink(mem, mem_size);

	if (mem_last >= mem && *mem_last < value)
	{
		divider = *mem_last--;
	}
	else
	{
		divider = 0;
	}

	if (res_size && *res_size)
	{
		mem_size = *res_size;
	}

	if (res && res != mem)
	{
		byte *res_bak = res;

		for (res_end = res + mem_size; mem_last >= mem && res < res_end; mem_last--, res++)
		{
			divider = divider << 8 | *mem_last;
			*res = divider / value;
			divider %= value;
		}

		if (res_size)
		{
			*res_size = res - res_bak;
		}

		math_memory_exchange(res_bak, NULL, res - res_bak);
	}
	else
	{
		res_end = (byte *)mem + mem_size;

		for (res = (byte *)mem_last; res >= mem; res--)
		{
			divider = divider << 8 | *res;
			*res = divider / value;
			divider %= value;
		}

		res = (byte *)mem_last + 1;

		if (res_size)
		{
			mem_last = math_memory_shrink(mem, mem_last - mem + 1);
			*res_size = mem_last - mem + 1;
		}
	}

	while (res < res_end)
	{
		*res++ = 0;
	}

	return divider;
}

byte math_memory_div_once(byte *left, size_t lsize, const byte *right, size_t rsize, byte *res, size_t res_size)
{
	int ret;
	u16 divider;
	u16 mh, ml;
	u16 rl;

	if (lsize < rsize)
	{
		return 0;
	}

	if (lsize > rsize)
	{
		divider = left[lsize - 1] << 8 | left[lsize - 2];
	}
	else
	{
		divider = left[lsize - 1];
	}

	rl = right[rsize - 1];
	mh = divider / rl;
	ml = divider / (rl + 1);

	if (mh > 0xFF)
	{
		pr_red_info("divider = 0x%04x, mh = 0x%04x, ml = 0x%04x", divider, mh, ml);
		mh = 0xFF;
	}

	while (1)
	{
		divider = (mh + ml + 1) / 2;

		math_memory_mul_single(right, rsize, divider, res, res_size);
		ret = math_memory_cmp(left, lsize, res, res_size);
		if (ret == 0)
		{
			break;
		}

		if (ret < 0)
		{
			mh = divider - 1;
		}
		else if (ml < divider)
		{
			ml = divider;
		}
		else
		{
			break;
		}
	}

	return divider;
}

size_t math_memory_div(byte *left, size_t lsize, const byte *right, size_t rsize, byte *res, size_t res_size)
{
	byte mult;
	byte *res_bak, *res_end;
	byte *left_pos, *left_last = math_memory_shrink(left, lsize);
	const byte *right_last = math_memory_shrink(right, rsize);

	rsize = right_last - right + 1;

	res_bak = res;
	left_pos = left_last - rsize + 1;

	for (res_end = res + res_size; left_pos >= left && res < res_end; res++, left_pos--)
	{
		byte buff[rsize + 1];

		lsize = left_last - left_pos + 1;

		mult = math_memory_div_once(left_pos, lsize, right, rsize, buff, sizeof(buff));
		if (mult)
		{
			math_memory_sub(left_pos, lsize, buff, sizeof(buff), NULL, 0);
		}

		*res = mult;

		while (left_last > left_pos && *left_last == 0)
		{
			left_last--;
		}
	}

	res_size = res - res_bak;
	math_memory_exchange(res_bak, NULL, res_size);

	while (res < res_end)
	{
		*res++ = 0;
	}

	return res_size;
}

size_t math_memory_div2(byte *left, size_t lsize, const byte *right, size_t rsize, byte *res, size_t res_size)
{
	byte *buff;
	const byte *left_last = math_memory_shrink(left, lsize);

	lsize = left_last - left + 1;

	buff = alloca(lsize);
	if (buff == NULL)
	{
		pr_error_info("buff == NULL");
		return 0xFF;
	}

	if (res && res != left)
	{
		math_memory_copy(buff, lsize, left, lsize);
		res_size = math_memory_div(buff, lsize, right, rsize, res, res_size);
	}
	else
	{
		res_size = math_memory_div((byte *)left, lsize, right, rsize, buff, lsize);
		math_memory_copy((byte *)left, lsize, buff, lsize);
	}

	return res_size;
}

// ================================================================================

int math_memory_calculator(const char *formula, byte *res, size_t res_size, int base, char fill, int size)
{
	byte left[res_size];
	byte right[res_size];
	char operator;
	char text_left[res_size << 3];
	char text_right[res_size << 3];
	char text_res[res_size << 3];
	const char *formula_end;

	if (res_size == 0)
	{
		pr_red_info("res_size == 0");
		return -EINVAL;
	}

	if (res == NULL)
	{
		res = alloca(res_size);
		if (res == NULL)
		{
			pr_error_info("res == NULL");
			return -ENOMEM;
		}
	}

	formula_end = formula + text_len(formula);

	formula = math_text2memory(formula, res, res_size, -1);

	while (formula < formula_end)
	{
		math_memory_copy(left, sizeof(left), res, res_size);
		operator = *formula;
		formula = math_text2memory(formula + 1, right, sizeof(right), -1);

		switch (operator)
		{
		case '+':
			math_memory_add(left, sizeof(left), right, sizeof(right), res, res_size);
			break;

		case '-':
			math_memory_sub(left, sizeof(left), right, sizeof(right), res, res_size);
			break;

		case '*':
			math_memory_mul(left, sizeof(left), right, sizeof(right), res, res_size);
			break;

		case '/':
			math_memory_div2(left, sizeof(left), right, sizeof(right), res, res_size);
			break;

		default:
			pr_red_info("invalid operator `%c'", operator);
			return -EINVAL;
		}

		math_memory2text(left, sizeof(left), text_left, sizeof(text_left), base, '0', size);
		math_memory2text(right, sizeof(right), text_right, sizeof(text_right), base, '0', size);
		math_memory2text(res, res_size, text_res, sizeof(text_res), base, '0', size);
		println("%s %c %s = %s", text_left, operator, text_right, text_res);
	}

	return 0;
}
