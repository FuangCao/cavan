// Fuang.Cao <cavan.cfa@gmail.com> Wed Apr 13 09:53:53 CST 2011

#include <cavan.h>
#include <cavan/memory.h>

#define MEM_COPY_FUNC(name, bits) \
	u##bits *name##bits(u##bits *dest, const u##bits *src, size_t count) \
	{ \
		const u##bits *end; \
		for (end = src + count; src < end; src++, dest++) \
		{ \
			*dest = *src; \
		} \
		return dest; \
	} \
	u##bits *name##_invert##bits(u##bits *dest, const u##bits *src, size_t count) \
	{ \
		const u##bits *last; \
		for (last = src + count - 1; last >= src; dest++, last--) \
		{ \
			*dest = *last; \
		} \
		return dest; \
	} \
	u##bits *name##_decrease##bits(u##bits *dest, const u##bits *src, size_t count) \
	{ \
		const u##bits *start; \
		for (start = src - count; src > start; src--, dest--) \
		{ \
			*dest = *src; \
		} \
		return ADDR_ADD(dest, ((bits) >> 3) - 1); \
	}

#define MEM_SET_FUNC(name, bits) \
	void name##bits(u##bits *mem, u##bits value, size_t count) \
	{ \
		u##bits *mem_end; \
		for (mem_end = mem + count; mem < mem_end; mem++) \
		{ \
			*mem = value; \
		} \
	}

#define NUMBER_SWAP_FUNC(name, bits) \
	void name##bits(u##bits *num1, u##bits *num2) \
	{ \
		u##bits temp; \
		temp = *num1; \
		*num1 = *num2; \
		*num2 = temp; \
	}

#define MEM_SWAP_FUNC(bits) \
	void mem_swap##bits(u##bits *dest, const u##bits *src, size_t count) \
	{ \
		const u##bits *src_end; \
		for (src_end = src + count; src < src_end; dest++, src++) \
		{ \
			*dest = SWAP##bits(*src); \
		} \
	}

#define MEM_FUNC_DECLARE(name, func) \
	func(name, 8) \
	func(name, 16) \
	func(name, 32) \
	func(name, 64)

MEM_FUNC_DECLARE(mem_copy, MEM_COPY_FUNC);
MEM_FUNC_DECLARE(mem_set, MEM_SET_FUNC);
MEM_FUNC_DECLARE(number_swap, NUMBER_SWAP_FUNC);
MEM_SWAP_FUNC(16);
MEM_SWAP_FUNC(32);
MEM_SWAP_FUNC(64);

// ================================================================================

void *mem_copy(void *dest, const void *src, size_t size)
{
	size_t count;

	if (dest == src)
	{
		return ((u8 *) dest) + size;
	}

	if (ADDR_IS_ALIGN(src, 8) && ADDR_IS_ALIGN(dest, 8))
	{
		count = size >> 3;
		dest = mem_copy64(dest, src, count);
		src = ((const u64 *) src) + count;
		count = size & 0x07;
	}
	else if (ADDR_IS_ALIGN(src, 4) && ADDR_IS_ALIGN(dest, 4))
	{
		count = size >> 2;
		dest = mem_copy32(dest, src, count);
		src = ((const u32 *) src) + count;
		count = size & 0x03;
	}
	else if (ADDR_IS_ALIGN(src, 2) && ADDR_IS_ALIGN(dest, 2))
	{
		count = size >> 1;
		dest = mem_copy16(dest, src, count);
		src = ((const u16 *) src) + count;
		count = size & 0x01;
	}
	else
	{
		count = size;
	}

	return mem_copy8(dest, src, count);
}

void *mem_copy_decrease(void *dest, const void *src, size_t size)
{
	size_t count;

	if (dest == src)
	{
		return ((u8 *) dest) + size;
	}

	if (size >= 8 && ADDR_IS_LEFT_ALIGN(src, 8) && ADDR_IS_LEFT_ALIGN(dest, 8))
	{
		count = size >> 3;
		dest = mem_copy_decrease64(ADDR_SUB(dest, 7), ADDR_SUB(src, 7), count);
		src = ADDR_SUB(src, count << 3);
		count = size & 0x07;
	}
	else if (size >= 4 && ADDR_IS_LEFT_ALIGN(src, 4) && ADDR_IS_LEFT_ALIGN(dest, 4))
	{
		count = size >> 2;
		dest = mem_copy_decrease32(ADDR_SUB(dest, 3), ADDR_SUB(src, 3), count);
		src = ADDR_SUB(src, count << 2);
		count = size & 0x03;
	}
	else if (size >= 2 && ADDR_IS_LEFT_ALIGN(src, 2) && ADDR_IS_LEFT_ALIGN(dest, 2))
	{
		count = size >> 1;
		dest = mem_copy_decrease16(ADDR_SUB(dest, 1), ADDR_SUB(src, 1), count);
		src = ADDR_SUB(src, count << 1);
		count = size & 0x01;
	}
	else
	{
		count = size;
	}

	return mem_copy_decrease8(dest, src, count);
}


void *mem_move(void *dest, const void *src, size_t size)
{
	if (dest < src)
	{
		return mem_copy(dest, src, size);
	}

	dest = ADDR_ADD(dest, size - 1);
	src = ADDR_ADD(src, size - 1);

	mem_copy_decrease(dest, src, size);

	return ADDR_ADD(dest, 1);
}

void mem_set(void *mem, int value, size_t size)
{
	size_t count;

	if (((long)mem & 0x07) == 0)
	{
		u64 value64;

		value64 = value;
		value64 |= value64 << 8;
		value64 |= value64 << 16;
		value64 |= value64 << 32;

		count = size >> 3;
		mem_set64(mem, value64, count);

		mem = (u8 *)mem + (count << 3);
		count = size & 0x07;
	}
	else if (((long)mem & 0x03) == 0)
	{
		u32 value32;

		value32 = value;
		value32 |= value32 << 8;
		value32 |= value32 << 16;

		count = size >> 2;
		mem_set32(mem, value32, count);

		mem = (u8 *)mem + (count << 2);
		count = size & 0x03;
	}
	else if (((long)mem & 0x01) == 0)
	{
		u16 value16;

		value16 = value;
		value16 |= value16 << 8;

		count = size >> 1;
		mem_set16(mem, value16, count);

		mem = (u8 *)mem + (count << 1);
		count = size & 0x01;
	}
	else
	{
		count = size;
	}

	if (count)
	{
		mem_set8(mem, value, count);
	}
}

// ================================================================================

void bits_set(char *mem, int start, int end, u32 value)
{
	int i;

	for (i = end; i >= start; i--)
	{
		int offset;
		char *mem8;

		mem8 = mem + (i >> 3);
		offset = i & 7;

		*mem8 &= ~(1 << offset);
		*mem8 |= (value & 1) << offset;

		print_char(value & 1 ? '1' : '0');

		value >>= 1;
	}
}

void mem_build_kmp_array(const char *sub, int *steps, size_t size)
{
	unsigned int i, j, k;

	for (i = 1, steps[0] = -1; i < size; i++)
	{
		j = 1;
		k = 0;

		while (j < i)
		{
			if (((char *)sub)[j] == ((char *)sub)[k])
			{
				j++;
				k++;
			}
			else if (steps[k] == -1)
			{
				j++;
				k = 0;
			}
			else
			{
				k = steps[k];
			}
		}

		if (((char *)sub)[k] == ((char *)sub)[i])
		{
			steps[i] = steps[k];
		}
		else
		{
			steps[i] = k;
		}
	}
}

char *mem_kmp_find_base(const char *mem, const char *mem_end, const char *sub, size_t sublen, const int *steps)
{
	unsigned int i = 0;

	while (mem < mem_end && i < sublen)
	{
		if (*(char *)mem == ((char *)sub)[i])
		{
			mem++;
			i++;
		}
		else if (steps[i] == -1)
		{
			mem++;
			i = 0;
		}
		else
		{
			i = steps[i];
		}
	}

	if (i < sublen)
	{
		return NULL;
	}

	return (char *)(mem - sublen);
}

char *mem_kmp_find(const char *mem, const char *sub, size_t memlen, size_t sublen)
{
	int steps[sublen];

	mem_build_kmp_array(sub, steps, sublen);

	return mem_kmp_find_base(mem, mem + memlen, sub, sublen, steps);
}

int mem_kmp_find_all(const char *mem, const char *sub, size_t memlen, size_t sublen, char **results, size_t size)
{
	int steps[sublen];
	char **result_bak, **result_end;
	const char *mem_end;

	mem_build_kmp_array(sub, steps, sublen);
	result_bak = results;
	result_end = results + size;
	mem_end = mem + memlen;

	while (results < result_end)
	{
		mem = mem_kmp_find_base(mem, mem_end, sub, sublen, steps);
		if (mem == NULL)
		{
			break;
		}

		*results = (char *)mem;
		mem += sublen;
		results++;
	}

	return results - result_bak;
}

size_t mem_delete_char_base(const char *mem_in, char *mem_out, const size_t size, const char c)
{
	const char *mem_end;
	char *mem_bak;

	mem_bak = mem_out;
	mem_end = mem_in + size;

	while (mem_in < mem_end)
	{
		if (*(const char *)mem_in == c)
		{
			mem_in++;
			continue;
		}

		*(char *)mem_out++ = *(const char *)mem_in++;
	}

	return mem_out - mem_bak;
}

int mem_is_set(const char *mem, int value, size_t size)
{
	const char *mem_end;

	for (mem_end = mem + size; mem < mem_end && *(u8 *)mem == value; mem++);

	return mem >= mem_end;
}

int mem_is_noset(const char *mem, int value, size_t size)
{
	const char *mem_end;

	for (mem_end = mem + size; mem < mem_end && *(u8 *)mem == value; mem++);

	return mem < mem_end;
}

u16 checksum16(const u16 *buff, size_t size)
{
	const u16 *buff_end;
	u32 checksum;

	for (checksum = 0, buff_end = buff + (size >> 1); buff < buff_end; buff++)
	{
		checksum += *buff;
	}

	if (size & 1)
	{
		checksum += *(u8 *)buff;
	}

	checksum = (checksum >> 16) + (checksum & 0xFFFF);

	return (checksum + (checksum >> 16)) & 0xFFFF;
}

size_t mem_byte_count(const char *mem, byte c, size_t size)
{
	const char *mem_end;
	size_t count;

	for (mem_end = mem + size, count = 0; mem < mem_end; mem++)
	{
		if ((*(byte *)mem) == c)
		{
			count++;
		}
	}

	return count;
}

void mem_reverse_simple(byte *start, byte *end)
{
	byte tmp;

	while (start < end)
	{
		tmp = *start;
		*start = *end;
		*end = tmp;

		start++;
		end--;
	}
}

void mem_reverse(byte *start, byte *end)
{
	if (start < end)
	{
		mem_reverse_simple(start, end);
	}
	else if (start > end)
	{
		mem_reverse_simple(end, start);
	}
}

bool byte_is_space(byte b)
{
	switch (b)
	{
	case ' ':
	case '\t':
	case '\f':
		return true;

	default:
		return false;
	}
}

bool byte_is_lf(byte b)
{
	switch (b)
	{
	case '\r':
	case '\n':
		return true;

	default:
		return false;
	}
}

bool byte_is_space_or_lf(byte b)
{
	switch (b)
	{
	case ' ':
	case '\t':
	case '\f':
	case '\r':
	case '\n':
		return true;

	default:
		return false;
	}
}

bool byte_is_named(byte b)
{
	switch (b)
	{
	case '-':
	case '_':
	case '0' ... '9':
	case 'a' ... 'z':
	case 'A' ... 'Z':
		return true;

	default:
		return false;
	}
}

void cavan_mem_dump(const byte *mem, size_t size, size_t width, const char *sep, const char *new_line)
{
	const byte *mem_end;

	if (sep == NULL)
	{
		sep = " ";
	}

	if (new_line == NULL)
	{
		new_line = "\n";
	}

	if (width < 1)
	{
		width = 10;
	}

	mem_end = mem + size;

	while (mem < mem_end)
	{
		const byte *line_end = mem + width - 1;
		if (line_end >= mem_end)
		{
			line_end = mem_end - 1;
		}

		while (mem < line_end)
		{
			printf("0x%02x%s", *mem++, sep);
		}

		printf("0x%02x%s", *mem++, new_line);
	}
}
