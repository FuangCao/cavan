#pragma once

// Fuang.Cao <cavan.cfa@gmail.com> Wed Apr 13 09:53:53 CST 2011

#include <cavan/text.h>

#define NUMBER_SWAP(num1, num2) \
	do { \
		typeof(num1) temp; \
		temp = num1; \
		num1 = num2; \
		num2 = temp; \
	} while (0)

#define assign(addr, value, type) \
	(*(type *)(addr) = (type)(value))

#define assign8(addr, value) \
	assign(addr, value, u8)

#define assign16(addr, value) \
	assign(addr, value, u16)

#define assign32(addr, value) \
	assign(addr, value, u32)

#define assign64(addr, value) \
	assign(addr, value, u64)

#define mem_set mem_set8

void *mem_copy(void *dest, const void *src, size_t size);
void *mem_copy2(void *dest, const void *src, const void *src_end);

void *mem_copy16(void *dest, const void *src, size_t size);
void *mem_copy32(void *dest, const void *src, size_t size);
void *mem_copy64(void *dest, const void *src, size_t size);

void *mem_set16(void *mem, int value, size_t size);
void *mem_set32(void *mem, int value, size_t size);
void *mem_set64(void *mem, int value, size_t size);

void bits_set(void *mem, int start, int end, u32 value);

void mem_build_kmp_array(const void *sub, int *steps, const size_t size);
void *mem_kmp_find_base(const void *mem, const void *mem_end, const void *sub, const size_t sublen, const int *steps);
void *mem_kmp_find(const void *mem, const void *sub, const size_t memlen, const size_t sublen);
int mem_kmp_find_all(const void *mem, const void *sub, size_t memlen, size_t sublen, void **results, size_t size);

size_t mem_delete_char_base(const void *mem_in, void *mem_out, const size_t size, const char c);

void number_swap8(u8 *num1, u8 *num2);
void number_swap16(u16 *num1, u16 *num2);
void number_swap32(u32 *num1, u32 *num2);

int mem_is_set(const void *mem, int value, size_t size);
int mem_is_noset(const void *mem, int value, size_t size);

u16 checksum16(const u16 *buff, size_t size);

size_t mem_byte_count(const void *mem, byte c, size_t size);

static inline void *mem_trans(void *text, size_t size)
{
	return text_ntrans(text, size);
}

static inline void mem_show(const void *mem, size_t size)
{
	text_show(mem, size);
}

static inline void mem_set8(void *mem, int value, size_t size)
{
	text_set8(mem, value, size);
}

static inline size_t mem_delete_char(void *mem, const size_t size, const char c)
{
	return mem_delete_char_base(mem, mem, size, c);
}

