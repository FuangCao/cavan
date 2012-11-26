#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Mon Nov 26 16:08:05 CST 2012
 */

#include <cavan.h>

u8 *math_text2memory(const char *text, u8 *mem, size_t mem_size, int base);

u8 *math_memory_shift_left(const u8 *src, u8 *result, size_t size, int shift);
u8 *math_memory_shift_right(const u8 *src, u8 *result, size_t size, int shift);

u8 *math_memory_add_single(const u8 *left, const u8 *left_last, u8 right, u8 *result, u8 *result_last);
u8 *math_memory_add(const u8 *left, const u8 *right, u8 *result, size_t size, size_t result_size);

u8 *math_memory_mul(const u8 *left, const u8 *right, u8 *result, size_t size, size_t result_size);
u8 *math_memory_mul_single(const u8 *left, const u8 *left_last, u8 right, u8 *result, u8 *result_last);
