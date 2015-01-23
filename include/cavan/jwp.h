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
typedef u32 jwp_u32;
typedef size_t jwp_size_t;
typedef u32 jwp_time_t;
typedef bool jwp_bool;
typedef void * jwp_timer;

#define JWP_TIMER_INVALID	NULL

#define jwp_println(fmt, args ...) \
	println(fmt, ##args)

#define jwp_pr_red_info(fmt, args ...) \
	pr_red_info(fmt, ##args)

#define JWP_MTU				0xFF
#define JWP_HEADER_SIZE		sizeof(struct jwp_header)
#define JWP_MAX_PAYLOAD		(JWP_MTU - JWP_HEADER_SIZE)
#define JWP_MAGIC_SIZE		2
#define JWP_MAGIC_HIGH		0x12
#define JWP_MAGIC_LOW		0x34
#define JWP_MAGIC			(JWP_MAGIC_HIGH << 8 | JWP_MAGIC_LOW)

#define JWP_USE_TIMER		1
#define JWP_USE_TX_QUEUE	1

#define JWP_SEND_RETRY		10
#define JWP_SEND_TIMEOUT	500
#define JWP_QUEUE_SIZE		(JWP_MTU * 3)

typedef enum
{
	JWP_PKG_DATA,
	JWP_PKG_DATA_ACK,
	JWP_PKG_CMD,
	JWP_PKG_CMD_ACK,
	JWP_PKG_SYNC,
	JWP_PKG_COUNT
} jwp_package_t;

typedef enum
{
#if JWP_USE_TX_QUEUE
	JWP_QUEUE_SEND,
#endif
	JWP_QUEUE_RECV,
	JWP_QUEUE_RECV_DATA,
	JWP_QUEUE_COUNT
} jwp_queue_t;

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
	jwp_u8 *head_peek;
	jwp_u8 *tail_peek;
};

struct jwp_desc
{
	jwp_u8 send_index;
	jwp_u8 recv_index;
	void *private_data;

	struct jwp_data_queue queues[JWP_QUEUE_COUNT];

	jwp_bool send_pendding;

	struct jwp_package rx_pkg;

#if JWP_USE_TX_QUEUE || JWP_USE_TIMER
	struct jwp_package tx_pkg;
#endif

#if JWP_USE_TIMER
	jwp_timer tx_timer;
#endif

	jwp_size_t (*hw_read)(struct jwp_desc *desc, void *buff, jwp_size_t size);
	jwp_size_t (*hw_write)(struct jwp_desc *desc, const void *buff, jwp_size_t size);
	void (*send_complete)(struct jwp_desc *desc);
	void (*data_received)(struct jwp_desc *desc);
	void (*package_received)(struct jwp_desc *desc, struct jwp_package *pkg);
#if JWP_USE_TIMER
	jwp_timer (*create_timer)(struct jwp_desc *desc, jwp_timer timer, jwp_time_t ms, void (*handler)(struct jwp_desc *desc, jwp_timer timer));
	void (*delete_timer)(struct jwp_desc *desc, jwp_timer timer);
#endif
};

void jwp_header_dump(const struct jwp_header *hdr);
void jwp_package_dump(const struct jwp_package *pkg);

void jwp_package_init(struct jwp_package *pkg);

void jwp_data_queue_init(struct jwp_data_queue *queue);
jwp_size_t jwp_data_queue_inqueue_peek(struct jwp_data_queue *queue, const u8 *buff, jwp_size_t size);
void jwp_data_queue_inqueue_commit(struct jwp_data_queue *queue);
jwp_size_t jwp_data_queue_inqueue(struct jwp_data_queue *queue, const u8 *buff, jwp_size_t size);
jwp_bool jwp_data_queue_inqueue_full(struct jwp_data_queue *queue, const u8 *buff, jwp_size_t size);
jwp_size_t jwp_data_queue_dequeue_peek(struct jwp_data_queue *queue, u8 *buff, jwp_size_t size);
void jwp_data_queue_dequeue_commit(struct jwp_data_queue *queue);
jwp_size_t jwp_data_queue_dequeue(struct jwp_data_queue *queue, u8 *buff, jwp_size_t size);
jwp_size_t jwp_data_queue_skip(struct jwp_data_queue *queue, jwp_size_t size);
jwp_bool jwp_data_queue_dequeue_package(struct jwp_data_queue *queue, struct jwp_header *hdr);
jwp_bool jwp_data_queue_fill_package(struct jwp_data_queue *queue, struct jwp_package *pkg);
jwp_size_t jwp_data_queue_get_free_size(struct jwp_data_queue *queue);
jwp_size_t jwp_data_queue_get_fill_size(struct jwp_data_queue *queue);

jwp_bool jwp_init(struct jwp_desc *desc, void *data);
jwp_bool jwp_send_package(struct jwp_desc *desc, struct jwp_header *hdr, bool sync);
void jwp_send_data_ack(struct jwp_desc *desc, jwp_u8 index);
jwp_bool jwp_process_rx_data(struct jwp_desc *desc);
jwp_bool jwp_process_tx_data(struct jwp_desc *desc);
jwp_size_t jwp_write_rx_data(struct jwp_desc *desc, const void *buff, jwp_size_t size);
jwp_size_t jwp_send_data(struct jwp_desc *desc, const void *buff, jwp_size_t size);
jwp_size_t jwp_recv_data(struct jwp_desc *desc, void *buff, jwp_size_t size);

static inline jwp_bool jwp_data_queue_inqueue_package(struct jwp_data_queue *queue, const struct jwp_header *hdr)
{
	return jwp_data_queue_inqueue_full(queue, (const u8 *) hdr, JWP_HEADER_SIZE + hdr->length);
}

static inline void jwp_set_private_data(struct jwp_desc *desc, void *data)
{
	desc->private_data = data;
}

static inline void *jwp_get_private_data(struct jwp_desc *desc)
{
	return desc->private_data;
}

static inline struct jwp_data_queue *jwp_data_queue_get(struct jwp_desc *desc, jwp_queue_t id)
{
	return desc->queues + id;
}
