#pragma once

/*
 * File:		jwp_mcu.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-01-31 14:05:28
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
#include <cavan/jwp-linux.h>

#define JWP_MCU_MTU				20

#define JWP_MCU_MAGIC_HIGH		0x12
#define JWP_MCU_MAGIC_LOW		0x34
#define JWP_MCU_MAGIC			(JWP_MCU_MAGIC_HIGH << 8 | JWP_MCU_MAGIC_LOW)
#define JWP_MCU_HEADER_SIZE		sizeof(struct jwp_mcu_header)
#define JWP_MCU_MAX_PAYLOAD		(JWP_MCU_MTU - JWP_MCU_HEADER_SIZE)

#pragma pack(1)
struct jwp_mcu_header
{
	union
	{
		jwp_u16 magic;
		struct
		{
			jwp_u8 magic_low;
			jwp_u8 magic_high;
		};
	};

	jwp_u8 type;
	jwp_u8 payload[0];
};
#pragma pack()

struct jwp_mcu_package
{
	union
	{
		struct
		{
			struct jwp_mcu_header header;
			jwp_u8 payload[JWP_MCU_MAX_PAYLOAD];
		};

		jwp_u8 body[JWP_MCU_MTU];
	};
};

struct jwp_mcu_rx_package
{
	union
	{
		struct
		{
			struct jwp_mcu_header header;
			jwp_u8 payload[JWP_MCU_MAX_PAYLOAD];
		};

		struct jwp_mcu_package package;
		jwp_u8 body[JWP_MCU_MTU];
	};

	jwp_u8 *head;
	jwp_u8 remain;
};

struct jwp_mcu_desc
{
	struct jwp_desc *jwp;
#if JWP_RX_DATA_QUEUE_ENABLE == 0
	struct jwp_mcu_rx_package rx_pkg;
#endif
};

jwp_bool jwp_mcu_init(struct jwp_mcu_desc *mcu, struct jwp_desc *jwp);
jwp_bool jwp_mcu_send_package(struct jwp_mcu_desc *mcu, jwp_u8 type, struct jwp_mcu_package *pkg);
