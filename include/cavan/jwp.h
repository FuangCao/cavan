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

#define JWP_DEBUG	0

#define JWP_WAIT_ENABLE				1

#define JWP_TIMER_ENABLE			1
#define JWP_TX_TIMER_ENABLE			1
#define JWP_TX_PKG_TIMER_ENABLE		1
#define JWP_RX_PKG_TIMER_ENABLE		1
#define JWP_TX_LATENCY_ENABLE		0

#define JWP_TX_QUEUE_ENABLE			1
#define JWP_RX_QUEUE_ENABLE			1
#define JWP_TX_DATA_QUEUE_ENABLE	0
#define JWP_RX_DATA_QUEUE_ENABLE	1
#define JWP_CHECKSUM_ENABLE			1

#define JWP_TX_LOOP_ENABLE			0
#define JWP_RX_LOOP_ENABLE			1
#define JWP_RX_PKG_LOOP_ENABLE		0
#define JWP_TX_DATA_LOOP_ENABLE		0

#define JWP_TX_NOTIFY_ENABLE		1
#define JWP_RX_CMD_NOTIFY_ENABLE	1
#define JWP_RX_DATA_NOTIFY_ENABLE	1
#define JWP_QUEUE_NOTIFY_ENABLE		1

#define JWP_MTU						0xFF
#define JWP_POLL_TIME				10
#define JWP_TX_RETRY				10
#define JWP_TX_TIMEOUT				2000
#define JWP_LATENCY_TIME			200
#define JWP_QUEUE_SIZE				(JWP_MTU * 3)

// ============================================================

typedef u8 jwp_u8;
typedef u16 jwp_u16;
typedef u32 jwp_u32;
typedef size_t jwp_size_t;
typedef bool jwp_bool;
typedef pthread_cond_t jwp_cond_t;
typedef pthread_mutex_t jwp_lock_t;

// ============================================================

#define jwp_msleep(msec) \
	msleep(msec);

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

typedef enum
{
	JWP_PKG_ACK,
	JWP_PKG_CMD,
	JWP_PKG_SYNC,
	JWP_PKG_DATA,
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

typedef enum
{
#if JWP_TX_TIMER_ENABLE
	JWP_TIMER_TX,
#endif
#if JWP_TX_LATENCY_ENABLE
	JWP_TIMER_TX_LATENCY,
#endif
#if JWP_TX_PKG_TIMER_ENABLE
	JWP_TIMER_TX_PKG,
#endif
#if JWP_RX_PKG_TIMER_ENABLE
	JWP_TIMER_RX_PKG,
#endif
	JWP_TIMER_COUNT
} jwp_timer_t;

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
};

struct jwp_rx_package
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

#if JWP_DEBUG
	const char *name;
#endif

	jwp_lock_t lock;

#if JWP_QUEUE_NOTIFY_ENABLE
	jwp_cond_t data_cond;
	jwp_cond_t space_cond;
#endif
};

struct jwp_timer
{
	void *handle;
	jwp_u32 msec;
	jwp_bool active;
	struct jwp_desc *jwp;

	jwp_lock_t lock;

#if JWP_DEBUG
	const char *name;
#endif

	jwp_bool (*handler)(struct jwp_timer *timer);
};

struct jwp_desc
{
	jwp_u8 tx_index;
	jwp_u8 rx_index;
	void *private_data;

#if JWP_DEBUG
	const char *name;
#endif

	struct jwp_queue queues[JWP_QUEUE_COUNT];
	struct jwp_timer timers[JWP_TIMER_COUNT];

	jwp_bool send_pendding;

	struct jwp_rx_package rx_pkg;

#if JWP_TX_QUEUE_ENABLE || JWP_TX_TIMER_ENABLE
	struct jwp_package tx_pkg;
#endif

	jwp_lock_t lock;

#if JWP_TX_NOTIFY_ENABLE
	jwp_cond_t tx_cond;
#endif

#if JWP_RX_CMD_NOTIFY_ENABLE
	jwp_cond_t command_rx_cond;
#endif

#if JWP_RX_DATA_NOTIFY_ENABLE
	jwp_cond_t data_rx_cond;
#endif

	jwp_size_t (*hw_read)(struct jwp_desc *jwp, void *buff, jwp_size_t size);
	jwp_size_t (*hw_write)(struct jwp_desc *jwp, const void *buff, jwp_size_t size);
	void (*send_complete)(struct jwp_desc *jwp);
	void (*data_received)(struct jwp_desc *jwp, const void *buff, jwp_size_t size);
	void (*command_received)(struct jwp_desc *jwp, const void *command, jwp_size_t size);
	void (*package_received)(struct jwp_desc *jwp, const struct jwp_header *hdr);

#if JWP_TIMER_ENABLE
	jwp_bool (*create_timer)(struct jwp_timer *timer);
	void (*delete_timer)(struct jwp_timer *timer);
#endif
};

// ============================================================

void jwp_header_dump(const struct jwp_header *hdr);
void jwp_package_dump(const struct jwp_package *pkg);
jwp_u8 jwp_checksum(const jwp_u8 *buff, jwp_size_t size);
jwp_u8 jwp_package_checksum(struct jwp_header *hdr);

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
jwp_size_t jwp_queue_get_free_size(struct jwp_queue *queue);
jwp_size_t jwp_queue_get_fill_size(struct jwp_queue *queue);
jwp_bool jwp_queue_empty(struct jwp_queue *queue);
jwp_bool jwp_queue_full(struct jwp_queue *queue);

// ============================================================

jwp_bool jwp_init(struct jwp_desc *jwp, void *data);
jwp_bool jwp_send_package(struct jwp_desc *jwp, struct jwp_header *hdr, bool sync);
void jwp_send_ack_package(struct jwp_desc *jwp, jwp_u8 index);
void jwp_send_sync_package(struct jwp_desc *jwp);
jwp_size_t jwp_send_data(struct jwp_desc *jwp, const void *buff, jwp_size_t size);
jwp_size_t jwp_recv_data(struct jwp_desc *jwp, void *buff, jwp_size_t size);
jwp_bool jwp_send_command(struct jwp_desc *jwp, const void *command, jwp_size_t size);

// ============================================================

void jwp_timer_run(struct jwp_timer *timer);
jwp_bool jwp_wait_tx_complete(struct jwp_desc *jwp);
jwp_size_t jwp_write_rx_data(struct jwp_desc *jwp, const jwp_u8 *buff, jwp_size_t size);
void jwp_tx_data_loop(struct jwp_desc *jwp);
void jwp_tx_loop(struct jwp_desc *jwp);
void jwp_rx_loop(struct jwp_desc *jwp);
void jwp_rx_package_loop(struct jwp_desc *jwp);

// ============================================================

static inline jwp_bool jwp_queue_inqueue_package(struct jwp_queue *queue, const struct jwp_header *hdr)
{
	return jwp_queue_try_inqueue(queue, (const jwp_u8 *) hdr, JWP_HEADER_SIZE + hdr->length);
}

static inline void jwp_queue_wait_data(struct jwp_queue *queue)
{
#if JWP_QUEUE_NOTIFY_ENABLE
	jwp_cond_wait(queue->data_cond, queue->lock);
#else
	jwp_msleep(JWP_POLL_TIME);
#endif
}

static inline void jwp_queue_wait_space(struct jwp_queue *queue)
{
#if JWP_QUEUE_NOTIFY_ENABLE
	jwp_cond_wait(queue->space_cond, queue->lock);
#else
	jwp_msleep(JWP_POLL_TIME);
#endif
}

static inline void jwp_set_private_data(struct jwp_desc *jwp, void *data)
{
	jwp->private_data = data;
}

static inline void *jwp_get_private_data(struct jwp_desc *jwp)
{
	return jwp->private_data;
}

static inline struct jwp_queue *jwp_get_queue(struct jwp_desc *jwp, jwp_queue_t queue)
{
	return jwp->queues + queue;
}

static inline struct jwp_timer *jwp_get_timer(struct jwp_desc *jwp, jwp_timer_t timer)
{
	return jwp->timers + timer;
}

static inline void jwp_wait_data_rx_complete(struct jwp_desc *jwp)
{
#if JWP_RX_DATA_NOTIFY_ENABLE
	jwp_cond_wait(jwp->data_rx_cond, jwp->lock);
#else
	jwp_msleep(JWP_POLL_TIME);
#endif
}

static inline void jwp_wait_command_rx_complete(struct jwp_desc *jwp)
{
#if JWP_RX_DATA_NOTIFY_ENABLE
	jwp_cond_wait(jwp->command_rx_cond, jwp->lock);
#else
	jwp_msleep(JWP_POLL_TIME);
#endif
}
