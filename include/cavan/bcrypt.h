#pragma once

/*
 * File:		bcrypt.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2017-05-07 18:17:37
 *
 * Copyright (c) 2017 Fuang.Cao <cavan.cfa@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <cavan.h>

#define GENSALT_DEFAULT_LOG2_ROUNDS		10
#define BCRYPT_SALT_LEN					16
#define BLOWFISH_NUM_ROUNDS				16

char *bcrypt_encode_base64(const uint8_t *data, size_t length, char *buff, size_t size);
uint8_t bcrypt_base64_value(char c);
uint8_t *bcrypt_decode_base64(const char *data, size_t length, uint8_t *buff, size_t size);
uint32_t bcrypt_build_word(const uint8_t *data, int length, int *offp);
uint8_t *bcrypt_crypt_raw(const uint8_t *key, int klen, const uint8_t *salt, int slen, int log_rounds, uint32_t *cdata, int clen, uint8_t *buff, size_t size);
char *bcrypt_hashpw(const char *password, const char *salt, char *buff, size_t size);
char *bcrypt_gensalt(int log_rounds, char *buff, size_t size);
char *bcrypt_gensalt2(int log_rounds, char *buff, size_t size);
char *bcrypt_gensalt3(char *buff, size_t size);
bool bcrypt_checkpw(const char *plaintext, const char *hashed);
