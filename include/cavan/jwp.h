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

typedef u8 jwp_u8;
typedef u16 jwp_u16;
typedef size_t jwp_size_t;
typedef u32 jwp_time_t;
typedef bool jwp_bool;
typedef void * jwp_timer;

#define JWP_TIMER_INVALID	NULL

#define jwp_println(fmt, args ...) \
	println(fmt, ##args)

#define JWP_MTU				0xFF
#define JWP_MAX_PAYLOAD		(JWP_MTU - sizeof(struct jwp_header))
#define JWP_MAGIC_SIZE		2
#define JWP_MAGIC_HIGH		0x12
#define JWP_MAGIC_LOW		0x34
#define JWP_MAGIC			(JWP_MAGIC_HIGH << 8 | JWP_MAGIC_LOW)

#define JWP_USE_TIMER		1
#define JWP_SEND_TIEMOUT	200
#define JWP_QUEUE_SIZE		(JWP_MTU * 3)

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
		jwp_u16 magic;
		struct
		{
			jwp_u8 magic_low;
			jwp_u8 magic_high;
		};
	};

	jwp_u8 type;
	jwp_u8 index;
	jwp_u8 length;
	jwp_u8 checksum;

	jwp_u8 payload[0];
};
#pragma pack()

struct jwp_package
{
	union
	{
		struct jwp_header header;
		jwp_u8 body[JWP_MTU];
	};

	jwp_bool in_use;

	jwp_u8 *head;
	jwp_u8 header_remain;
	jwp_u8 data_remain;
};

struct jwp_data_queue
{
	jwp_u8 buff[JWP_QUEUE_SIZE];
	jwp_u8 *last;
	jwp_u8 *head;
	jwp_u8 *tail;
	jwp_u8 *peek;
};

struct jwp_desc
{
	jwp_u8 send_index;
	jwp_u8 recv_index;
	void *private_data;

	struct jwp_data_queue send_queue;
	struct jwp_data_queue recv_queue;
	struct jwp_data_queue data_queue;
	struct jwp_package pkg_send;
	struct jwp_package pkg_recv;

#if JWP_USE_TIMER
	jwp_timer send_timer;
#endif

	jwp_size_t (*hw_read)(struct jwp_desc *desc, void *buff, jwp_size_t size);
	jwp_size_t (*hw_write)(struct jwp_desc *desc, const void *buff, jwp_size_t size);
	void (*data_received)(struct jwp_desc *desc, const void *data, jwp_size_t size);
	void (*package_received)(struct jwp_desc *desc, struct jwp_package *pkg);
#if JWP_USE_TIMER
	jwp_timer (*create_timer)(struct jwp_desc *desc, jwp_timer timer, jwp_time_t ms, void (*handler)(struct jwp_desc *desc, jwp_timer timer));
	void (*delete_timer)(struct jwp_desc *desc, jwp_timer timer);
#endif
};

void jwp_header_dump(const struct jwp_header *hdr);
void jwp_package_dump(const struct jwp_package *pkg);

void jwp_package_init(struct jwp_package *pkg);
jwp_bool jwp_package_fill(struct jwp_package *pkg, struct jwp_data_queue *queue);

void jwp_data_queue_init(struct jwp_data_queue *queue);
jwp_size_t jwp_data_queue_inqueue(struct jwp_data_queue *queue, const u8 *buff, jwp_size_t size);
jwp_size_t jwp_data_queue_peek(struct jwp_data_queue *queue, u8 *buff, jwp_size_t size);
void jwp_data_queue_commit(struct jwp_data_queue *queue);
jwp_size_t jwp_data_queue_dequeue(struct jwp_data_queue *queue, u8 *buff, jwp_size_t size);
jwp_size_t jwp_data_queue_skip(struct jwp_data_queue *queue, jwp_size_t size);
jwp_size_t jwp_data_queue_get_free_size(struct jwp_data_queue *queue);
jwp_size_t jwp_data_queue_get_fill_size(struct jwp_data_queue *queue);

jwp_bool jwp_init(struct jwp_desc *desc, void *data);
void jwp_send_queue_flush(struct jwp_desc *desc);
void jwp_send_package(struct jwp_desc *desc, struct jwp_header *hdr, jwp_u8 type, const void *data, jwp_size_t size);
void jwp_send_data_ack(struct jwp_desc *desc, jwp_u8 index);
jwp_bool jwp_send_package_sync(struct jwp_desc *desc, jwp_u8 type, const void *data, jwp_size_t size);
jwp_size_t jwp_send_data(struct jwp_desc *desc, const void *buff, jwp_size_t size);
void jwp_write_data(struct jwp_desc *desc, const void *buff, jwp_size_t size);
void jwp_process_rx_data(struct jwp_desc *desc);

static inline void jwp_set_private_data(struct jwp_desc *desc, void *data)
{
	desc->private_data = data;
}

static inline void *jwp_get_private_data(struct jwp_desc *desc)
{
	return desc->private_data;
}
