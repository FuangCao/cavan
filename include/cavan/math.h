#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Mon Nov 26 16:08:05 CST 2012
 */

#include <cavan.h>

byte *math_memory_shrink(const byte *mem, size_t size);
void math_memory_copy(byte *dest, size_t dest_size, const byte *src, size_t src_size);
byte *math_text2memory(const char *text, byte *mem, size_t mem_size, int base);

void math_memory_shift_left_byte(const byte *mem, size_t mem_size, size_t shift, byte *res, size_t res_size);
void math_memory_shift_right_byte(const byte *mem, size_t mem_size, size_t shift, byte *res, size_t res_size);

byte math_memory_add_single(const byte *mem, size_t mem_size, byte value, byte carry, byte *res, size_t res_size);
byte math_memory_add(const byte *left, size_t lsize, const byte *right, size_t rsize, byte *res, size_t res_size);

int math_memory_cmp(const byte *left, size_t lsize, const byte *right, size_t rsize);
byte math_byte_sub_carry(byte left, byte right, byte carry, byte *res);
byte math_memory_sub_single(const byte *mem, size_t mem_size, byte value, byte carry, byte *res, size_t res_size);
	byte math_memory_sub_single2(byte value, byte carry, const byte *mem, size_t mem_size, byte *res, size_t res_size);
byte math_memory_sub(const byte *left, size_t lsize, const byte *right, size_t rsize, byte *res, size_t res_size);

byte math_memory_mul_single(const byte *mem, size_t mem_size, byte value, byte carry, byte *res, size_t res_size);
byte math_memory_mul(const byte *left, size_t lsize, const byte *right, size_t rsize, byte *res, size_t res_size);

byte math_memory_div_single(const byte *mem, size_t mem_size, byte value, byte *res, size_t res_size);
byte math_memory_div(const byte *left, size_t lsize, const byte *right, size_t rsize, byte *res, size_t res_size);
