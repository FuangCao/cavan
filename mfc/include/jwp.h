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

#ifndef JWP_ARCH_NAME
#error "don't include this file immediately, you must include jwp-*.h"
#endif

#define JWP_QUEUE_ENABLE	(JWP_TX_HW_QUEUE_ENABLE || JWP_TX_QUEUE_ENABLE || JWP_RX_QUEUE_ENABLE || JWP_TX_DATA_QUEUE_ENABLE || JWP_RX_DATA_QUEUE_ENABLE)
#define JWP_TIMER_ENABLE	(JWP_TX_TIMER_ENABLE || JWP_TX_DATA_TIMER_ENABLE || JWP_TX_PKG_TIMER_ENABLE || JWP_RX_PKG_TIMER_ENABLE)
#define JWP_LOOP_ENABLE		(JWP_TX_LOOP_ENABLE || JWP_RX_LOOP_ENABLE || JWP_RX_PKG_LOOP_ENABLE || JWP_TX_DATA_LOOP_ENABLE)

#define jwp_signal_wait(signal, lock) \
	do { \
		jwp_lock_acquire(lock); \
		jwp_signal_wait_locked(signal, lock); \
		jwp_lock_release(lock); \
	} while (0)

#define jwp_signal_timedwait(signal, lock, msec) \
	do { \
		jwp_lock_acquire(lock); \
		jwp_signal_timedwait_locked(signal, lock, msec); \
		jwp_lock_release(lock); \
	} while (0)

#define jwp_signal_notify(signal, lock) \
	do { \
		jwp_lock_acquire(lock); \
		jwp_signal_notify_locked(signal, lock); \
		jwp_lock_release(lock); \
	} while (0)

#define jwp_pr_pos_info() \
	jwp_printf("%s => %s[%d]\n", __FUNCTION__, __LINE__, __FILE__)

// ============================================================

#define JWP_HEADER_SIZE			sizeof(struct jwp_header)
#define JWP_MAX_PAYLOAD			(JWP_MTU - JWP_HEADER_SIZE)
#define JWP_GET_PAYLOAD(hdr)	(((jwp_u8 *) hdr) + JWP_HEADER_SIZE)
#define JWP_MAGIC_SIZE			2
#define JWP_MAGIC_HIGH			0x12
#define JWP_MAGIC_LOW			0x34
#define JWP_MAGIC				(JWP_MAGIC_HIGH << 8 | JWP_MAGIC_LOW)
#define JWP_NELEM(a)			((int) (sizeof(a) / sizeof((a)[0])))

// ============================================================

typedef enum
{
	JWP_PKG_ACK,
	JWP_PKG_CMD,
	JWP_PKG_SYNC,
	JWP_PKG_DATA,
	JWP_PKG_LOG,
	JWP_PKG_COUNT
} jwp_package_t;

typedef enum
{
	JWP_STATE_INIT,
	JWP_STATE_READY,
	JWP_STATE_FAULT
} jwp_state_t;

typedef enum
{
#if JWP_TX_HW_QUEUE_ENABLE
	JWP_QUEUE_TX_HW,
#endif
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
#if JWP_TX_DATA_TIMER_ENABLE
	JWP_TIMER_TX_DATA,
#endif
#if JWP_TX_PKG_TIMER_ENABLE
	JWP_TIMER_TX_PKG,
#endif
#if JWP_RX_PKG_TIMER_ENABLE
	JWP_TIMER_RX_PKG,
#endif
	JWP_TIMER_COUNT
} jwp_timer_t;

typedef enum
{
	JWP_DEVICE_LOCAL,
	JWP_DEVICE_REMOTE,
	JWP_DEVICE_COUNT
} jwp_device_t;

#ifndef CSR101x
#pragma pack(1)
#endif
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

#ifndef _WIN32
	jwp_u8 payload[0];
#endif
#ifdef CSR101x
} __attribute__((packed));
#else
};
#pragma pack()
#endif

struct jwp_package
{
	union
	{
		struct
		{
			struct jwp_header header;
			jwp_u8 payload[JWP_MTU - JWP_HEADER_SIZE];
		};
		jwp_u8 body[JWP_MTU];
	};
};

struct jwp_queue
{
	jwp_u8 buff[JWP_QUEUE_SIZE];
	jwp_u8 *last;
	jwp_u8 *head;
	jwp_u8 *tail;
	jwp_u8 *head_peek;
	jwp_u8 *tail_peek;

	jwp_bool hardware;

#if JWP_DEBUG_MEMBER
	const char *name;
#endif

	jwp_lock_t lock;

#if JWP_QUEUE_NOTIFY_ENABLE
	jwp_signal_t data_signal;
	jwp_signal_t space_signal;
#endif
};

struct jwp_timer
{
	union
	{
		void *handle;
		jwp_u8 handle_u8;
		jwp_u16 handle_u16;
		jwp_u32 handle_u32;
	};

	jwp_u32 msec;
	jwp_bool active;
	struct jwp_desc *jwp;

	jwp_lock_t lock;

#if JWP_DEBUG_MEMBER
	const char *name;
#endif

	jwp_bool (*handler)(struct jwp_timer *timer);
};

struct jwp_package_receiver
{
	jwp_u8 *body;
	jwp_u8 *head;
	jwp_u8 *header_start;
	jwp_u8 *payload_start;
	jwp_u8 *payload_end;
	jwp_size_t payload_max;

	jwp_u8 *data;
	jwp_u8 *data_head;
	jwp_size_t data_max;
	jwp_size_t data_remain;
	jwp_bool data_pendding;

	void *private_data;
	jwp_lock_t lock;

	jwp_size_t (*get_payload_length)(struct jwp_package_receiver *receiver);
	void (*process_package)(struct jwp_package_receiver *receiver);
	jwp_size_t (*process_data)(struct jwp_package_receiver *receiver, const jwp_u8 *data, jwp_size_t size);
};

struct jwp_desc
{
	jwp_u8 tx_index;
	jwp_u8 rx_index;
#if JWP_TX_TIMER_ENABLE
	jwp_u8 send_retry;
#endif

	void *private_data;

#if JWP_DEBUG_MEMBER
	int line;
	jwp_state_t state;
	const char *name;
#endif

#if JWP_QUEUE_ENABLE
	struct jwp_queue queues[JWP_QUEUE_COUNT];
#endif

#if JWP_TIMER_ENABLE
	struct jwp_timer timers[JWP_TIMER_COUNT];
#endif

	jwp_bool send_pendding;

	struct jwp_package rx_pkg;
	jwp_u8 rx_data[JWP_MAX_PAYLOAD];
	struct jwp_package_receiver receiver;

#if JWP_TX_TIMER_ENABLE || JWP_TX_LOOP_ENABLE
	struct jwp_package tx_pkg;
#endif

	jwp_lock_t lock;
	jwp_lock_t write_lock;

#if JWP_TX_NOTIFY_ENABLE
	jwp_signal_t tx_signal;
#endif

#if JWP_RX_CMD_NOTIFY_ENABLE
	jwp_signal_t command_rx_signal;
#endif

#if JWP_RX_DATA_NOTIFY_ENABLE
	jwp_signal_t data_rx_signal;
#endif

	jwp_size_t (*hw_read)(struct jwp_desc *jwp, void *buff, jwp_size_t size);
	jwp_size_t (*hw_write)(struct jwp_desc *jwp, const void *buff, jwp_size_t size);
	void (*send_complete)(struct jwp_desc *jwp);
	void (*remote_not_response)(struct jwp_desc *jwp);
	void (*data_received)(struct jwp_desc *jwp, const void *buff, jwp_size_t size);
	void (*command_received)(struct jwp_desc *jwp, const void *command, jwp_size_t size);
	void (*package_received)(struct jwp_desc *jwp, const struct jwp_header *hdr);

#if JWP_WRITE_LOG_ENABLE
	void (*log_received)(struct jwp_desc *jwp, jwp_device_t device, const char *log, jwp_size_t size);
#endif

#if JWP_TIMER_ENABLE
	jwp_bool (*create_timer)(struct jwp_timer *timer);
	void (*delete_timer)(struct jwp_timer *timer);
#endif
};

extern struct jwp_desc *jwp_global;

// ============================================================

char *jwp_strcpy(char *dest, const char *src);
jwp_size_t jwp_strlen(const char *text);
char *jwp_value2str10(jwp_u32 value, char *buff, jwp_size_t size);
char *jwp_value2str16(jwp_u32 value, char *buff, jwp_size_t size);
void jwp_pr_value(const char *prompt, jwp_u32 value, jwp_u8 base);
char *jwp_mem_to_string(const jwp_u8 *mem, jwp_size_t mem_size, char *buff, jwp_size_t buff_size);
void jwp_dump_mem(const jwp_u8 *mem, jwp_size_t mem_size);
void jwp_printf(const char *fmt, ...);

void jwp_header_dump(const struct jwp_header *hdr);
void jwp_package_dump(const struct jwp_package *pkg);
jwp_u8 jwp_checksum(const jwp_u8 *buff, jwp_size_t size);
jwp_u8 jwp_package_checksum(struct jwp_header *hdr);

// ============================================================

void jwp_queue_clear(struct jwp_queue *queue);
void jwp_queue_init(struct jwp_queue *queue);
jwp_size_t jwp_queue_inqueue_peek(struct jwp_queue *queue, const jwp_u8 *buff, jwp_size_t size);
void jwp_queue_inqueue_commit(struct jwp_queue *queue);
jwp_size_t jwp_queue_inqueue_locked(struct jwp_queue *queue, const jwp_u8 *buff, jwp_size_t size);
jwp_size_t jwp_queue_inqueue(struct jwp_queue *queue, const jwp_u8 *buff, jwp_size_t size);
jwp_bool jwp_queue_try_inqueue(struct jwp_queue *queue, const jwp_u8 *buff, jwp_size_t size);
void jwp_queue_inqueue_all(struct jwp_queue *queue, const jwp_u8 *buff, jwp_size_t size);
jwp_size_t jwp_queue_dequeue_peek(struct jwp_queue *queue, jwp_u8 *buff, jwp_size_t size);
void jwp_queue_dequeue_commit(struct jwp_queue *queue);
jwp_size_t jwp_queue_dequeue_locked(struct jwp_queue *queue, jwp_u8 *buff, jwp_size_t size);
jwp_size_t jwp_queue_dequeue(struct jwp_queue *queue, jwp_u8 *buff, jwp_size_t size);
jwp_size_t jwp_queue_get_free_size(const struct jwp_queue *queue);
jwp_size_t jwp_queue_get_used_size(const struct jwp_queue *queue);
jwp_bool jwp_queue_is_empty(const struct jwp_queue *queue);
jwp_bool jwp_queue_is_full(const struct jwp_queue *queue);
void jwp_queue_wait_data(struct jwp_queue *queue);
void jwp_queue_wait_space(struct jwp_queue *queue);

static inline jwp_size_t jwp_queue_seek(struct jwp_queue *queue, jwp_size_t size)
{
	return jwp_queue_inqueue(queue, NULL, size);
}

static inline jwp_size_t jwp_queue_skip(struct jwp_queue *queue, jwp_size_t size)
{
	return jwp_queue_dequeue(queue, NULL, size);
}

static inline void jwp_queue_set_hardware(struct jwp_queue *queue, jwp_bool hardware)
{
	queue->hardware = hardware;
}

// ============================================================

void jwp_package_receiver_init(struct jwp_package_receiver *receiver, jwp_size_t magic_size, jwp_size_t header_size, jwp_size_t size);
jwp_bool jwp_package_receiver_copy_data(struct jwp_package_receiver *receiver, const jwp_u8 *buff, jwp_size_t size);
jwp_size_t jwp_package_receiver_write(struct jwp_package_receiver *receiver, const jwp_u8 *buff, jwp_size_t size);
void jwp_package_receiver_fill(struct jwp_package_receiver *receiver, const jwp_u8 *buff, jwp_size_t size);
jwp_bool jwp_package_receiver_fill_by_queue(struct jwp_package_receiver *receiver, struct jwp_queue *queue);

static inline void jwp_package_receiver_set_private_data(struct jwp_package_receiver *receiver, void *data)
{
	receiver->private_data = data;
}

static inline void *jwp_package_receiver_get_private_data(struct jwp_package_receiver *receiver)
{
	return receiver->private_data;
}

// ============================================================

jwp_bool jwp_init(struct jwp_desc *jwp, void *data);
jwp_bool jwp_send_package(struct jwp_desc *jwp, struct jwp_header *hdr, jwp_bool sync);
void jwp_send_empty_package(struct jwp_desc *jwp, jwp_u8 type, jwp_u8 index);
void jwp_send_sync(struct jwp_desc *jwp);
jwp_size_t jwp_send_data(struct jwp_desc *jwp, const void *buff, jwp_size_t size);
jwp_size_t jwp_recv_data(struct jwp_desc *jwp, void *buff, jwp_size_t size);
jwp_bool jwp_send_data_all(struct jwp_desc *jwp, const jwp_u8 *buff, jwp_size_t size);
jwp_bool jwp_send_command(struct jwp_desc *jwp, const void *command, jwp_size_t size);
void jwp_send_log(struct jwp_desc *jwp, const char *log, jwp_size_t size);

static inline void jwp_send_ack_package(struct jwp_desc *jwp, jwp_u8 index)
{
	jwp_send_empty_package(jwp, JWP_PKG_ACK, index);
}

static inline void jwp_send_sync_package(struct jwp_desc *jwp)
{
	jwp_send_empty_package(jwp, JWP_PKG_SYNC, 0);
}

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

static inline void jwp_set_private_data(struct jwp_desc *jwp, void *data)
{
	jwp->private_data = data;
}

static inline void *jwp_get_private_data(struct jwp_desc *jwp)
{
	return jwp->private_data;
}

#if JWP_QUEUE_ENABLE
static inline struct jwp_queue *jwp_get_queue(struct jwp_desc *jwp, jwp_queue_t queue)
{
	return jwp->queues + queue;
}
#endif

#if JWP_TIMER_ENABLE
static inline struct jwp_timer *jwp_get_timer(struct jwp_desc *jwp, jwp_timer_t timer)
{
	return jwp->timers + timer;
}
#endif

static inline void jwp_wait_data_rx_complete(struct jwp_desc *jwp)
{
#if JWP_RX_DATA_NOTIFY_ENABLE
	jwp_signal_wait(jwp->data_rx_signal, jwp->lock);
#else
	jwp_msleep(JWP_POLL_TIME);
#endif
}

static inline void jwp_wait_command_rx_complete(struct jwp_desc *jwp)
{
#if JWP_RX_DATA_NOTIFY_ENABLE
	jwp_signal_wait(jwp->command_rx_signal, jwp->lock);
#else
	jwp_msleep(JWP_POLL_TIME);
#endif
}

static inline void jwp_set_send_pendding(struct jwp_desc *jwp, jwp_bool pendding)
{
	jwp_lock_acquire(jwp->lock);
	jwp->send_pendding = pendding;
	jwp_lock_release(jwp->lock);
}

static inline void jwp_set_package_index(struct jwp_desc *jwp, struct jwp_header *hdr)
{
	jwp_lock_acquire(jwp->lock);
	hdr->index = jwp->tx_index + 1;
	jwp_lock_release(jwp->lock);
}

static inline jwp_bool jwp_wait_and_set_send_pendding(struct jwp_desc *jwp)
{
	if (!jwp_wait_tx_complete(jwp))
	{
		return false;
	}

	jwp_set_send_pendding(jwp, true);

	return true;
}
