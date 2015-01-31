/*
 * File:		jwp_mcu.c
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
#include <cavan/jwp_mcu.h>

#define JWP_MCU_DEBUG		1

static void jwp_mcu_proccess_package(struct jwp_mcu_header *hdr)
{
#if JWP_MCU_DEBUG
	jwp_printf("jwp_mcu_header: type = %d\n", hdr->type);
#endif
}

#if JWP_RX_DATA_QUEUE_ENABLE == 0
static void jwp_mcu_rx_package_init(struct jwp_mcu_rx_package *pkg)
{
	pkg->remain = JWP_MCU_MTU;
	pkg->head = pkg->body;
}

static jwp_size_t jwp_mcu_rx_package_fill(struct jwp_mcu_rx_package *pkg, const jwp_u8 *buff, jwp_size_t size)
{
	if (size < pkg->remain)
	{
		jwp_memcpy(pkg->head, buff, size);

		pkg->head += size;
		pkg->remain -= size;
	}
	else
	{
		size = pkg->remain;
		jwp_memcpy(pkg->head, buff, size);

		jwp_mcu_proccess_rx_package(&pkg->header);
		jwp_mcu_rx_package_init(pkg);
	}

	return size;
}
#endif

static void jwp_mcu_data_received(struct jwp_desc *jwp, const void *buff, jwp_size_t size)
{
#if JWP_RX_DATA_QUEUE_ENABLE
	struct jwp_queue *queue = jwp_get_queue(jwp, JWP_QUEUE_RX_DATA);

	while (jwp_queue_get_used_size(queue) >= sizeof(struct jwp_mcu_package))
	{
		struct jwp_mcu_package pkg;

		jwp_queue_dequeue_peek(queue, (jwp_u8 *) &pkg.header.magic, sizeof(pkg.header.magic));
		if (pkg.header.magic_low == JWP_MCU_MAGIC_LOW && pkg.header.magic_high == JWP_MCU_MAGIC_HIGH)
		{
			jwp_queue_dequeue(queue, (jwp_u8 *) &pkg, sizeof(pkg));
			jwp_mcu_proccess_package(&pkg.header);
		}
		else
		{
			jwp_queue_skip(queue, 1);
		}
	}
#else
	struct jwp_mcu_desc *mcu = jwp_get_private_data(jwp);

	while (1)
	{
		jwp_size wrlen;

		wrlen = jwp_mcu_rx_package_fill(&mcu->rx_pkg, buff, size);
		if (wrlen == size)
		{
			break;
		}

		buff += wrlen;
		size -= wrlen;
	}
#endif
}

jwp_bool jwp_mcu_init(struct jwp_mcu_desc *mcu, struct jwp_desc *jwp)
{
	mcu->jwp = jwp;
	jwp_set_private_data(jwp, mcu);
	jwp->data_received = jwp_mcu_data_received;

#if JWP_RX_DATA_QUEUE_ENABLE == 0
	jwp_mcu_rx_package_init(&mcu->rx_pkg);
#endif

	return true;
}

jwp_bool jwp_mcu_send_package(struct jwp_mcu_desc *mcu, jwp_u8 type, struct jwp_mcu_package *pkg)
{
	struct jwp_mcu_header *hdr = &pkg->header;

	hdr->magic_low = JWP_MCU_MAGIC_LOW;
	hdr->magic_high = JWP_MCU_MAGIC_HIGH;
	hdr->type = type;

	return jwp_send_data_all(mcu->jwp, (jwp_u8 *) pkg, sizeof(struct jwp_mcu_package));
}
