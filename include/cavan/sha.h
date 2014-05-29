#pragma once

/*
 * File:		sha.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2014-05-28 17:50:42
 *
 * Copyright (c) 2014 Fuang.Cao <cavan.cfa@gmail.com>
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

#define SHA_DIGEST_SIZE 20

#define ROR(value, bits) \
	((value) >> (bits) | (value) << ((sizeof(value) << 3) - (bits)))

#define ROL(value, bits) \
	((value) << (bits) | (value) >> ((sizeof(value) << 3) - (bits)))

struct cavan_sha1_context
{
	u64 count;
	size_t remain;

	u8 buff[64];
	u32 state[5];
};

int cavan_sha1sum(const void *buff, size_t size, u8 *digest);
int cavan_file_sha1sum_mmap(const char *pathname, u8 *digest);
int cavan_file_sha1sum(const char *pathname, u8 *digest);

static inline char *cavan_shasum_tostring(const u8 *digest, size_t size, char *buff, size_t buff_size)
{
	mem2text_base(digest, size, buff, buff_size);
	return buff;
}
