#pragma once

// Fuang.Cao <cavan.cfa@gmail.com> Wed Apr 13 09:53:53 CST 2011

#include <cavan.h>
#include <cavan/text.h>

#define KB(a)	((a) << 10)
#define BK(a)	((a) >> 10)
#define MB(a)	((a) << 20)
#define BM(a)	((a) >> 20)
#define GB(a)	((a) << 30)
#define BG(a)	((a) >> 30)

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

void mem_copy8(u8 *dest, const u8 *src, size_t count);
void mem_copy16(u16 *dest, const u16 *src, size_t count);
void mem_copy32(u32 *dest, const u32 *src, size_t count);
void mem_copy64(u64 *dest, const u64 *src, size_t count);
void *mem_copy(void *dest, const void *src, size_t size);

void mem_move8(u8 *dest, const u8 *src, size_t count);
void mem_move16(u16 *dest, const u16 *src, size_t count);
void mem_move32(u32 *dest, const u32 *src, size_t count);
void mem_move64(u64 *dest, const u64 *src, size_t count);
void mem_move(void *dest, const void *src, size_t size);

void mem_set8(u8 *mem, u8 value, size_t count);
void mem_set16(u16 *mem, u16 value, size_t count);
void mem_set32(u32 *mem, u32 value, size_t count);
void mem_set64(u64 *mem, u64 value, size_t count);
void mem_set(void *mem, int value, size_t size);

void bits_set(char *mem, int start, int end, u32 value);

void mem_build_kmp_array(const char *sub, int *steps, const size_t size);
char *mem_kmp_find_base(const char *mem, const char *mem_end, const char *sub, const size_t sublen, const int *steps);
char *mem_kmp_find(const char *mem, const char *sub, size_t memlen, size_t sublen);
int mem_kmp_find_all(const char *mem, const char *sub, size_t memlen, size_t sublen, char **results, size_t size);

size_t mem_delete_char_base(const char *mem_in, char *mem_out, const size_t size, const char c);

void number_swap8(u8 *num1, u8 *num2);
void number_swap16(u16 *num1, u16 *num2);
void number_swap32(u32 *num1, u32 *num2);

int mem_is_set(const char *mem, int value, size_t size);
int mem_is_noset(const char *mem, int value, size_t size);

u16 checksum16(const u16 *buff, size_t size);

size_t mem_byte_count(const char *mem, byte c, size_t size);

void mem_reverse_simple(byte *start, byte *end);
void mem_reverse(byte *start, byte *end);

static inline char *mem_trans(char *text, size_t size)
{
	return text_ntrans(text, size);
}

static inline void mem_show(const char *mem, size_t size)
{
	text_show(mem, size);
}

static inline size_t mem_delete_char(char *mem, const size_t size, const char c)
{
	return mem_delete_char_base(mem, mem, size, c);
}
