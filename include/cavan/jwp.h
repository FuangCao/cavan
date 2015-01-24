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

#define JWP_WAIT_ENABLE				1
#define JWP_TIMER_ENABLE			1
#define JWP_TX_LATENCY_ENABLE		0

#define JWP_TX_QUEUE_ENABLE			1
#define JWP_RX_QUEUE_ENABLE			1
#define JWP_TX_DATA_QUEUE_ENABLE	1
#define JWP_RX_DATA_QUEUE_ENABLE	1
#define JWP_CHECKSUM_ENABLE			1

#define JWP_TX_DATA_LOOP_ENABLE		1
#define JWP_TX_PKG_LOOP_ENABLE		1
#define JWP_RX_PKG_LOOP_ENABLE		1

#define JWP_MTU						0xFF
#define JWP_SEND_RETRY				10
#define JWP_SEND_TIMEOUT			500
#define JWP_LATENCY_TIME			200
#define JWP_QUEUE_SIZE				(JWP_MTU * 3)

// ============================================================

typedef u8 jwp_u8;
typedef u16 jwp_u16;
typedef u32 jwp_u32;
typedef size_t jwp_size_t;
typedef bool jwp_bool;
typedef void * jwp_timer;
typedef pthread_cond_t jwp_cond_t;
typedef pthread_mutex_t jwp_lock_t;

// ============================================================

#define JWP_TIMER_INVALID	NULL

#define jwp_memcpy(dest, src, size) \
	memcpy(dest, src, size)

#define jwp_lock_init(lock) \
	pthread_mutex_init(&lock, NULL)

#define jwp_lock_acquire(lock) \
	pthread_mutex_lock(&lock)

#define jwp_lock_release(lock) \
	pthread_mutex_unlock(&lock)

#define jwp_cond_init(cond) \
	pthread_cond_init(&cond, NULL)

#define jwp_cond_wait(cond, lock) \
	do { \
		jwp_lock_acquire(lock); \
		pthread_cond_wait(&cond, &lock); \
		jwp_lock_release(lock); \
	} while (0)

#define jwp_cond_timedwait(cond, lock, msec) \
	do { \
		long __msec; \
		struct timespec __ts; \
		clock_gettime(CLOCK_REALTIME, &__ts); \
		__msec = __ts.tv_nsec / 1000000L + msec; \
		__ts.tv_sec += __msec / 1000; \
		__ts.tv_nsec = (__msec % 1000) * 1000000L; \
		jwp_lock_acquire(lock); \
		pthread_cond_timedwait(&cond, &lock, &__ts); \
		jwp_lock_release(lock); \
	} while (0)

#define jwp_cond_notify(cond) \
	pthread_cond_signal(&cond)

#define jwp_println(fmt, args ...) \
	println(fmt, ##args)

#define jwp_pr_red_info(fmt, args ...) \
	pr_red_info(fmt, ##args)

// ============================================================

#define JWP_HEADER_SIZE		sizeof(struct jwp_header)
#define JWP_MAX_PAYLOAD		(JWP_MTU - JWP_HEADER_SIZE)
#define JWP_MAGIC_SIZE		2
#define JWP_MAGIC_HIGH		0x12
#define JWP_MAGIC_LOW		0x34
#define JWP_MAGIC			(JWP_MAGIC_HIGH << 8 | JWP_MAGIC_LOW)

// ============================================================

#if JWP_TIMER_ENABLE && (JWP_TX_QUEUE_ENABLE == 0 && JWP_TX_LATENCY_ENABLE == 0)
#error "must enable tx queue or tx latency when use timer"
#endif

#if JWP_TX_LATENCY_ENABLE && (JWP_TIMER_ENABLE == 0 || JWP_TX_DATA_QUEUE_ENABLE == 0)
#error "must enable timer and tx data queue when use tx latency"
#endif

#if JWP_TX_DATA_LOOP_ENABLE && JWP_TX_DATA_QUEUE_ENABLE == 0
#error "must tx data queue when use tx data loop"
#endif

#if JWP_TX_DATA_LOOP_ENABLE && JWP_TX_LATENCY_ENABLE
#error "don't enable tx data loop and tx latency at the same time"
#endif

#if JWP_TX_PKG_LOOP_ENABLE && JWP_TX_QUEUE_ENABLE == 0
#error "must tx queue when use tx package loop"
#endif

#if JWP_RX_PKG_LOOP_ENABLE && JWP_RX_QUEUE_ENABLE == 0
#error "must rx queue when use rx package loop"
#endif

// ============================================================

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
#if JWP_TX_QUEUE_ENABLE
	JWP_QUEUE_TX,
#endif
#if JWP_RX_QUEUE_ENABLE
	JWP_QUEUE_RX,
#endif
#if JWP_TX_DATA_QUEUE_ENABLE
	JWP_QUEUE_TX_DATA,
#endif
#if JWP_RX_DATA_QUEUE_ENABLE
	JWP_QUEUE_RX_DATA,
#endif
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

struct jwp_queue
{
	jwp_u8 buff[JWP_QUEUE_SIZE];
	jwp_u8 *last;
	jwp_u8 *head;
	jwp_u8 *tail;
	jwp_u8 *head_peek;
	jwp_u8 *tail_peek;

	jwp_lock_t lock;
	jwp_cond_t data_cond;
	jwp_cond_t space_cond;
};

struct jwp_desc
{
	jwp_u8 send_index;
	jwp_u8 recv_index;
	void *private_data;

	struct jwp_queue queues[JWP_QUEUE_COUNT];

	jwp_bool send_pendding;

	struct jwp_package rx_pkg;

#if JWP_TX_QUEUE_ENABLE || JWP_TIMER_ENABLE
	struct jwp_package tx_pkg;
#endif

#if JWP_TIMER_ENABLE
	jwp_timer tx_timer;
#endif

#if JWP_TX_LATENCY_ENABLE
	jwp_timer tx_latency_timer;
#endif

	jwp_lock_t lock;
	jwp_cond_t data_tx_cond;
	jwp_cond_t data_rx_cond;

	jwp_size_t (*hw_read)(struct jwp_desc *desc, void *buff, jwp_size_t size);
	jwp_size_t (*hw_write)(struct jwp_desc *desc, const void *buff, jwp_size_t size);
	void (*send_complete)(struct jwp_desc *desc);
	void (*data_received)(struct jwp_desc *desc, const void *buff, jwp_size_t size);
	void (*package_received)(struct jwp_desc *desc, struct jwp_package *pkg);
#if JWP_TIMER_ENABLE
	jwp_timer (*create_timer)(struct jwp_desc *desc, jwp_timer timer, jwp_u32 ms, void (*handler)(struct jwp_desc *desc, jwp_timer timer));
	void (*delete_timer)(struct jwp_desc *desc, jwp_timer timer);
#endif
};

// ============================================================

void jwp_header_dump(const struct jwp_header *hdr);
void jwp_package_dump(const struct jwp_package *pkg);
jwp_u8 jwp_checksum(const jwp_u8 *buff, jwp_size_t size);
jwp_u8 jwp_package_checksum(struct jwp_header *hdr);
void jwp_package_init(struct jwp_package *pkg);

// ============================================================

void jwp_queue_init(struct jwp_queue *queue);
jwp_size_t jwp_queue_inqueue_peek(struct jwp_queue *queue, const jwp_u8 *buff, jwp_size_t size);
void jwp_queue_inqueue_commit(struct jwp_queue *queue);
jwp_size_t jwp_queue_inqueue(struct jwp_queue *queue, const jwp_u8 *buff, jwp_size_t size);
jwp_bool jwp_queue_try_inqueue(struct jwp_queue *queue, const jwp_u8 *buff, jwp_size_t size);
void jwp_queue_inqueue_all(struct jwp_queue *queue, const jwp_u8 *buff, jwp_size_t size);
jwp_size_t jwp_queue_dequeue_peek(struct jwp_queue *queue, jwp_u8 *buff, jwp_size_t size);
void jwp_queue_dequeue_commit(struct jwp_queue *queue);
jwp_size_t jwp_queue_dequeue(struct jwp_queue *queue, jwp_u8 *buff, jwp_size_t size);
jwp_size_t jwp_queue_skip(struct jwp_queue *queue, jwp_size_t size);
jwp_bool jwp_queue_dequeue_package(struct jwp_queue *queue, struct jwp_header *hdr);
jwp_size_t jwp_queue_get_free_size(struct jwp_queue *queue);
jwp_size_t jwp_queue_get_fill_size(struct jwp_queue *queue);
jwp_bool jwp_queue_empty(struct jwp_queue *queue);
jwp_bool jwp_queue_full(struct jwp_queue *queue);

// ============================================================

jwp_bool jwp_init(struct jwp_desc *desc, void *data);
jwp_bool jwp_send_package(struct jwp_desc *desc, struct jwp_header *hdr, bool sync);
void jwp_send_data_ack(struct jwp_desc *desc, jwp_u8 index);
jwp_size_t jwp_send_data(struct jwp_desc *desc, const void *buff, jwp_size_t size);
jwp_size_t jwp_recv_data(struct jwp_desc *desc, void *buff, jwp_size_t size);

// ============================================================

jwp_size_t jwp_write_rx_data(struct jwp_desc *desc, const void *buff, jwp_size_t size);
void jwp_tx_data_loop(struct jwp_desc *desc);
void jwp_tx_package_loop(struct jwp_desc *desc);
void jwp_rx_package_loop(struct jwp_desc *desc);

// ============================================================

static inline jwp_bool jwp_queue_inqueue_package(struct jwp_queue *queue, const struct jwp_header *hdr)
{
	return jwp_queue_try_inqueue(queue, (const jwp_u8 *) hdr, JWP_HEADER_SIZE + hdr->length);
}

static inline void jwp_queue_wait_data(struct jwp_queue *queue)
{
	jwp_cond_wait(queue->data_cond, queue->lock);
}

static inline void jwp_queue_wait_space(struct jwp_queue *queue)
{
	jwp_cond_wait(queue->space_cond, queue->lock);
}

static inline void jwp_set_private_data(struct jwp_desc *desc, void *data)
{
	desc->private_data = data;
}

static inline void *jwp_get_private_data(struct jwp_desc *desc)
{
	return desc->private_data;
}

static inline struct jwp_queue *jwp_get_queue(struct jwp_desc *desc, jwp_queue_t id)
{
	return desc->queues + id;
}

static inline jwp_bool jwp_wait_data_tx_complete(struct jwp_desc *desc)
{
	jwp_cond_timedwait(desc->data_tx_cond, desc->lock, JWP_SEND_TIMEOUT);

	return desc->send_pendding == false;
}

static inline void jwp_wait_data_rx_complete(struct jwp_desc *desc)
{
	jwp_cond_wait(desc->data_rx_cond, desc->lock);
}
