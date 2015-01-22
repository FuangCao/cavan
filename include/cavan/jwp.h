#pragma once

/*
 * File:			jwp.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-01-22 10:11:44
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

#define jwp_println(fmt, args ...) \
	println(fmt, ##args)

#define JWP_MTU			0xFF
#define JWP_MAGIC_SIZE	2
#define JWP_MAGIC_HIGH	0x12
#define JWP_MAGIC_LOW	0x34
#define JWP_MAGIC		(JWP_MAGIC_HIGH << 8 | JWP_MAGIC_LOW)

typedef enum
{
	JWP_PKG_DATA,
	JWP_PKG_DATA_ACK,
	JWP_PKG_CMD,
	JWP_PKG_CMD_ACK,
} jwp_package_t;

#pragma pack(1)
struct jwp_header
{
	union
	{
		u16 magic;
		struct
		{
			u8 magic_low;
			u8 magic_high;
		};
	};

	u8 type;
	u8 index;
	u8 length;
	u8 checksum;

	u8 payload[0];
};
#pragma pack()

struct jwp_package
{
	union
	{
		struct jwp_header header;
		u8 body[JWP_MTU];
	};

	u8 *head;
	u8 header_remain;
	u8 data_remain;

	struct jwp_desc *desc;
};

struct jwp_desc
{
	void *private_data;
	void (*package_received)(struct jwp_desc *desc, struct jwp_package *pkg);
};

void jwp_header_dump(const struct jwp_header *hdr);
void jwp_package_dump(const struct jwp_package *pkg);

void jwp_package_init(struct jwp_package *pkg, struct jwp_desc *desc);
u8 jwp_package_fill(struct jwp_package *pkg, const u8 *buff, u8 length);

static inline void jwp_set_private_data(struct jwp_desc *desc, void *data)
{
	desc->private_data = data;
}

static inline void *jwp_get_private_data(struct jwp_desc *desc)
{
	return desc->private_data;
}
