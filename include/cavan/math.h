#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Mon Nov 26 16:08:05 CST 2012
 */

#include <cavan.h>

byte *math_memory_shrink(const byte *mem, size_t size);
void math_memory_exchange(const byte *mem, byte *res, size_t size);
void math_memory_copy(byte *dest, size_t dest_size, const byte *src, size_t src_size);
char *math_text2memory(const char *text, byte *mem, size_t mem_size, int base);
char *math_memory2text(const byte *mem, size_t mem_size, char *text, size_t text_size, int base, char fill, size_t size);
void math_memory_show(const char *prompt, const byte *mem, size_t mem_size, int base);

void math_memory_shift_left_byte(const byte *mem, size_t mem_size, size_t shift, byte *res, size_t res_size);
void math_memory_shift_right_byte(const byte *mem, size_t mem_size, size_t shift, byte *res, size_t res_size);

void math_memory_and(const byte *left, const byte *right, byte *res, size_t size);
void math_memory_or(const byte *left, const byte *right, byte *res, size_t size);
void math_memory_not(const byte *mem, byte *res, size_t size);
void math_memory_xor(const byte *left, const byte *right, byte *res, size_t size);

byte math_memory_add_single(const byte *mem, size_t mem_size, byte value, byte *res, size_t res_size);
byte math_memory_add(const byte *left, size_t lsize, const byte *right, size_t rsize, byte *res, size_t res_size);

int math_memory_cmp_base(const byte *left_last, const byte *right, const byte *right_last);
int math_memory_cmp(const byte *left, size_t lsize, const byte *right, size_t rsize);
byte math_byte_sub_carry(byte left, byte right, byte carry, byte *res);
byte math_memory_sub_single(const byte *mem, size_t mem_size, byte value, byte carry, byte *res, size_t res_size);
byte math_memory_sub_single2(byte value, byte carry, const byte *mem, size_t mem_size, byte *res, size_t res_size);
byte math_memory_sub(const byte *left, size_t lsize, const byte *right, size_t rsize, byte *res, size_t res_size);

byte math_memory_mul_single(const byte *mem, size_t mem_size, byte value, byte *res, size_t res_size);
byte math_memory_mul(const byte *left, size_t lsize, const byte *right, size_t rsize, byte *res, size_t res_size);

byte math_memory_div_single(const byte *mem, size_t mem_size, byte value, byte *res, size_t *res_size);
byte math_memory_div_once(byte *left, size_t lsize, const byte *right, size_t rsize, byte *res, size_t res_size);
size_t math_memory_div(byte *left, size_t lsize, const byte *right, size_t rsize, byte *res, size_t res_size);
size_t math_memory_div2(byte *left, size_t lsize, const byte *right, size_t rsize, byte *res, size_t res_size, int base);

int math_memory_calculator(const char *formula, byte *res, size_t res_size, int base, char fill, int size);
