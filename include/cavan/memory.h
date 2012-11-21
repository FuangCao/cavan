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

char *mem_copy(char *dest, const char *src, size_t size);
char *mem_copy2(char *dest, const char *src, const char *src_end);

char *mem_copy16(char *dest, const char *src, size_t size);
char *mem_copy32(char *dest, const char *src, size_t size);
char *mem_copy64(char *dest, const char *src, size_t size);

char *mem_set16(char *mem, int value, size_t size);
char *mem_set32(char *mem, int value, size_t size);
char *mem_set64(char *mem, int value, size_t size);

void bits_set(char *mem, int start, int end, u32 value);

void mem_build_kmp_array(const char *sub, int *steps, const size_t size);
char *mem_kmp_find_base(const char *mem, const char *mem_end, const char *sub, const size_t sublen, const int *steps);
char *mem_kmp_find(const char *mem, const char *sub, const size_t memlen, const size_t sublen);
int mem_kmp_find_all(const char *mem, const char *sub, size_t memlen, size_t sublen, char **results, size_t size);

size_t mem_delete_char_base(const char *mem_in, char *mem_out, const size_t size, const char c);

void number_swap8(u8 *num1, u8 *num2);
void number_swap16(u16 *num1, u16 *num2);
void number_swap32(u32 *num1, u32 *num2);

int mem_is_set(const char *mem, int value, size_t size);
int mem_is_noset(const char *mem, int value, size_t size);

u16 checksum16(const u16 *buff, size_t size);

size_t mem_byte_count(const char *mem, byte c, size_t size);

static inline char *mem_trans(char *text, size_t size)
{
	return text_ntrans(text, size);
}

static inline void mem_show(const char *mem, size_t size)
{
	text_show(mem, size);
}

static inline void mem_set8(void *mem, int value, size_t size)
{
	text_set8(mem, value, size);
}

static inline size_t mem_delete_char(char *mem, const size_t size, const char c)
{
	return mem_delete_char_base(mem, mem, size, c);
}

