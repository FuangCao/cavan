#pragma once

/*
 * File:			codec.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2016-01-14 12:16:39
 *
 * Copyright (c) 2016 Fuang.Cao <cavan.cfa@gmail.com>
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

#define CAVAN_LZ77_WIN_SIZE		0xFF

#pragma pack(1)

struct cavan_lz77_node {
	u8 offset;
	u8 length;
	u8 value;
};

#pragma pack()

int cavan_lz77_encode(const char *src_file, const char *dest_file);
int cavan_lz77_decode(const char *src_file, const char *dest_file);
