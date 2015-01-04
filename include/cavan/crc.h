#pragma once

/*
 * File:		crc.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-01-04 10:54:30
 *
 * Copyright (c) 2015 Fuang.Cao <cavan.cfa@gmail.com>
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

extern const u16 cavan_crc16_table[256];
extern const u32 cavan_crc32_table[256];

u16 cavan_crc16(u16 crc, const char *buff, size_t size);
u32 cavan_crc32(u32 crc, const char *buff, size_t size);
