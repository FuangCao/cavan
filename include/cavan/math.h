#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Mon Nov 26 16:08:05 CST 2012
 */

#include <cavan.h>

void math_memory_copy(u8 *dest, const u8 *src, size_t dest_size, size_t src_size);
u8 *math_text2memory(const char *text, u8 *mem, size_t mem_size, int base);

u8 *math_memory_shift_left(u8 *mem, size_t size, size_t shift);
u8 *math_memory_shift_left2(const u8 *src, u8 *result, size_t size, size_t shift);

u8 *math_memory_shift_right(u8 *mem, size_t size, size_t shift);
u8 *math_memory_shift_right2(const u8 *src, u8 *result, size_t size, size_t shift);

u8 *math_memory_add_single(u8 *mem, u8 *mem_last, u8 value);
u8 *math_memory_add_single2(u8 *mem, size_t size, u8 value);
u8 *math_memory_add_single3(const u8 *mem, u8 *result, size_t mem_size, size_t result_size, u8 value);

u8 *math_memory_add(u8 *left, u8 *left_last, const u8 *right, const u8 *right_last);
u8 *math_memory_add2(u8 *left, const u8 *right, size_t lsize, size_t rsize);
u8 *math_memory_add3(const u8 *left, const u8 *right, u8 *result, size_t size, size_t result_size);

u8 *math_memory_mul_single(u8 *mem, u8 *mem_last, u8 *mem_end, u8 value);
u8 *math_memory_mul_single2(u8 *mem, size_t size, u8 value);
u8 *math_memory_mul_single3(const u8 *mem, u8 *result, size_t mem_size, size_t result_size, u8 value);

u8 *math_memory_mul(const u8 *left, const u8 *right, u8 *result, size_t size, size_t result_size);
u8 *math_memory_mul2(u8 *left, const u8 *right, size_t lsize, size_t rsize);
