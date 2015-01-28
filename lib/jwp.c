/*
 * File:			jwp.c
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

// #include "stdafx.h"

#if defined(_WIN32) || defined(CSR101x)
#include "jwp.h"
#else
#include <cavan.h>
#include <cavan/jwp.h>
#endif

// ============================================================

#if JWP_QUEUE_ENABLE == 0
#if JWP_TX_QUEUE_ENABLE
#error "must enable queue when use tx queue"
#endif

#if JWP_RX_QUEUE_ENABLE
#error "must enable queue when use rx queue"
#endif

#if JWP_TX_DATA_QUEUE_ENABLE
#error "must enable queue when use tx data queue"
#endif

#if JWP_RX_DATA_QUEUE_ENABLE
#error "must enable queue when use tx data queue"
#endif
#endif

#if JWP_TIMER_ENABLE == 0
#if JWP_TX_TIMER_ENABLE
#error "must enable timer when use tx timer"
#endif

#if JWP_TX_DATA_TIMER_ENABLE
#error "must enable timer when use tx data timer"
#endif

#if JWP_TX_PKG_TIMER_ENABLE
#error "must enable timer when use tx package timer"
#endif

#if JWP_RX_PKG_TIMER_ENABLE
#error "must enable timer when use rx package timer"
#endif
#endif

#if JWP_TX_DATA_TIMER_ENABLE && JWP_TX_DATA_QUEUE_ENABLE == 0
#error "must enable tx data queue when use tx data timer"
#endif

#if JWP_TX_PKG_TIMER_ENABLE && (JWP_TX_TIMER_ENABLE == 0 || JWP_TX_QUEUE_ENABLE == 0)
#error "must enable tx timer and tx queue when use tx package timer"
#endif

#if JWP_TX_PKG_TIMER_ENABLE && JWP_TX_LOOP_ENABLE
#error "don't enable tx package timer and tx loop at the same time"
#endif

#if JWP_RX_PKG_TIMER_ENABLE && JWP_RX_PKG_LOOP_ENABLE
#error "don't enable rx package timer and rx package loop at the same time"
#endif

#if JWP_RX_PKG_TIMER_ENABLE && JWP_RX_QUEUE_ENABLE == 0
#error "must enable rx queue when use rx package timer"
#endif

#if JWP_TX_DATA_LOOP_ENABLE && JWP_TX_DATA_QUEUE_ENABLE == 0
#error "must enable tx data queue when use tx data loop"
#endif

#if JWP_TX_DATA_LOOP_ENABLE && JWP_TX_DATA_TIMER_ENABLE
#error "don't enable tx data loop and tx data timer at the same time"
#endif

#if JWP_TX_LOOP_ENABLE && JWP_TX_QUEUE_ENABLE == 0
#error "must enable tx queue when use tx loop"
#endif

#if JWP_RX_PKG_LOOP_ENABLE && JWP_RX_QUEUE_ENABLE == 0
#error "must enable rx queue when use rx package loop"
#endif

#if JWP_PRINTF_ENABLE == 0
#if JWP_DEBUG
#error "must enable printf when debug"
#endif

#if JWP_SHOW_ERROR
#error "must enable printf when need show error"
#endif
#endif

// ============================================================

static void jwp_process_rx_package(struct jwp_desc *jwp);

#if JWP_PRINTF_ENABLE
static struct jwp_desc *jwp_global;

void jwp_printf(const char *fmt, ...)
{
	va_list ap;
	jwp_size_t size;
	char buff[2048];

	if (jwp_global == NULL)
	{
		return;
	}

	va_start(ap, fmt);

#ifdef _WIN32
	size = _vsnprintf(buff, sizeof(buff), fmt, ap);
#else
	size = vsnprintf(buff, sizeof(buff), fmt, ap);
#endif

	va_end(ap);

	jwp_global->log_received(jwp_global, buff, size);
}

void jwp_header_dump(const struct jwp_header *hdr)
{
	jwp_printf("index = %d, type = %d, length = %d, checksum = 0x%02x\n", hdr->index, hdr->type, hdr->length, hdr->checksum);
}

void jwp_package_dump(const struct jwp_package *pkg)
{
	jwp_header_dump(&pkg->header);
}
#endif

static void jwp_rx_package_init(struct jwp_rx_package *pkg)
{
	pkg->head = pkg->body;
	pkg->header_remain = sizeof(pkg->header);
}

static void jwp_hw_write(struct jwp_desc *jwp, const jwp_u8 *buff, jwp_size_t size)
{
	while (size > 0)
	{
		jwp_size_t wrlen;

		wrlen = jwp->hw_write(jwp, buff, size);
		buff += wrlen;
		size -= wrlen;
	}
}

static inline void jwp_hw_write_package(struct jwp_desc *jwp, struct jwp_header *hdr)
{
#if JWP_CHECKSUM_ENABLE
	hdr->checksum = jwp_package_checksum(hdr);
#endif

	jwp_hw_write(jwp, (jwp_u8 *) hdr, JWP_HEADER_SIZE + hdr->length);
}

#if JWP_CHECKSUM_ENABLE
jwp_u8 jwp_checksum(const jwp_u8 *buff, jwp_size_t size)
{
	jwp_u16 checksum = 0;
	const jwp_u8 *buff_end;

	for (buff_end = buff + size; buff < buff_end; buff++)
	{
		checksum += *buff;
	}

	checksum = (checksum >> 8) + (checksum & 0xFF);

	return (jwp_u8) ((checksum >> 8) + checksum);
}

jwp_u8 jwp_package_checksum(struct jwp_header *hdr)
{
	hdr->checksum = 0;

	return jwp_checksum(((jwp_u8 *) hdr) + JWP_MAGIC_SIZE, JWP_HEADER_SIZE - JWP_MAGIC_SIZE + hdr->length);
}
#endif

#if (JWP_TX_QUEUE_ENABLE == 0 && (JWP_POLL_ENABLE == 0 || JWP_TX_DATA_TIMER_ENABLE)) || JWP_TX_PKG_TIMER_ENABLE
static jwp_bool jwp_check_and_set_send_pendding(struct jwp_desc *jwp)
{
	jwp_bool res;

	jwp_lock_acquire(jwp->lock);

	if (jwp->send_pendding)
	{
		res = false;
	}
	else
	{
		jwp->send_pendding = res = true;
	}

	jwp_lock_release(jwp->lock);

	return res;
}
#endif

// ============================================================

#if JWP_QUEUE_ENABLE
void jwp_queue_init(struct jwp_queue *queue)
{
	jwp_lock_init(queue->lock);

#if JWP_QUEUE_NOTIFY_ENABLE
	jwp_signal_init(queue->data_signal);
	jwp_signal_init(queue->space_signal);
#endif

	queue->tail = queue->head = queue->buff;
	queue->tail_peek = queue->head_peek = queue->tail;
	queue->last = queue->tail + sizeof(queue->buff) - 1;
}

static jwp_size_t jwp_queue_inqueue_peek_locked(struct jwp_queue *queue, const jwp_u8 *buff, jwp_size_t size)
{
	jwp_size_t rlen, length;

	if (queue->tail < queue->head)
	{
		length = rlen = queue->head - queue->tail - 1;
	}
	else
	{
		rlen = queue->last - queue->tail + 1;
		length = rlen + (queue->head - queue->buff) - 1;
	}

	if (size > length)
	{
		size = length;
	}

	if (size > rlen)
	{
		length = size - rlen;

		if (buff)
		{
			jwp_memcpy(queue->tail, buff, rlen);
			jwp_memcpy(queue->buff, buff + rlen, length);
		}

		queue->tail_peek = queue->buff + length;
	}
	else
	{
		if (buff)
		{
			jwp_memcpy(queue->tail, buff, size);
		}

		queue->tail_peek = queue->tail + size;
		if (queue->tail_peek > queue->last)
		{
			queue->tail_peek = queue->buff;
		}
	}

	return size;
}

jwp_size_t jwp_queue_inqueue_peek(struct jwp_queue *queue, const jwp_u8 *buff, jwp_size_t size)
{
	jwp_lock_acquire(queue->lock);
	size = jwp_queue_inqueue_peek_locked(queue, buff, size);
	jwp_lock_release(queue->lock);

	return size;
}

static void jwp_queue_inqueue_commit_locked(struct jwp_queue *queue)
{
	queue->tail = queue->tail_peek;

#if JWP_QUEUE_NOTIFY_ENABLE
	jwp_signal_notify_locked(queue->data_signal, queue->lock);
#endif
}

void jwp_queue_inqueue_commit(struct jwp_queue *queue)
{
	jwp_lock_acquire(queue->lock);
	jwp_queue_inqueue_commit_locked(queue);
	jwp_lock_release(queue->lock);
}

jwp_size_t jwp_queue_inqueue(struct jwp_queue *queue, const jwp_u8 *buff, jwp_size_t size)
{
	jwp_lock_acquire(queue->lock);
	size = jwp_queue_inqueue_peek_locked(queue, buff, size);
	jwp_queue_inqueue_commit_locked(queue);
	jwp_lock_release(queue->lock);

	return size;
}

jwp_bool jwp_queue_try_inqueue(struct jwp_queue *queue, const jwp_u8 *buff, jwp_size_t size)
{
	jwp_size_t wrlen;

	jwp_lock_acquire(queue->lock);

	wrlen = jwp_queue_inqueue_peek_locked(queue, buff, size);
	if (wrlen < size)
	{
		jwp_lock_release(queue->lock);
		return false;
	}

	jwp_queue_inqueue_commit_locked(queue);

	jwp_lock_release(queue->lock);

	return true;
}

void jwp_queue_inqueue_all(struct jwp_queue *queue, const jwp_u8 *buff, jwp_size_t size)
{
	while (size > 0)
	{
		jwp_size_t wrlen;

		wrlen = jwp_queue_inqueue(queue, buff, size);
		if (wrlen)
		{
			buff += wrlen;
			size -= wrlen;
		}
		else
		{
			jwp_queue_wait_space(queue);
		}
	}
}

static jwp_size_t jwp_queue_dequeue_peek_locked(struct jwp_queue *queue, jwp_u8 *buff, jwp_size_t size)
{
	jwp_size_t rlen, length;

	if (queue->head > queue->tail)
	{
		rlen = queue->last - queue->head + 1;
		length = rlen + (queue->tail - queue->buff);
	}
	else
	{
		length = rlen = queue->tail - queue->head;
	}

	if (size > length)
	{
		size = length;
	}

	if (size > rlen)
	{
		length = size - rlen;

		if (buff)
		{
			jwp_memcpy(buff, queue->head, rlen);
			jwp_memcpy(buff + rlen, queue->buff, length);
		}

		queue->head_peek = queue->buff + length;
	}
	else
	{
		if (buff)
		{
			jwp_memcpy(buff, queue->head, size);
		}

		queue->head_peek = queue->head + size;
		if (queue->head_peek > queue->last)
		{
			queue->head_peek = queue->buff;
		}
	}

	return size;
}

jwp_size_t jwp_queue_dequeue_peek(struct jwp_queue *queue, jwp_u8 *buff, jwp_size_t size)
{
	jwp_lock_acquire(queue->lock);
	size = jwp_queue_dequeue_peek_locked(queue, buff, size);
	jwp_lock_release(queue->lock);

	return size;
}

static void jwp_queue_dequeue_commit_locked(struct jwp_queue *queue)
{
	queue->head = queue->head_peek;

#if JWP_QUEUE_NOTIFY_ENABLE
	jwp_signal_notify_locked(queue->space_signal, queue->lock);
#endif
}

void jwp_queue_dequeue_commit(struct jwp_queue *queue)
{
	jwp_lock_acquire(queue->lock);
	jwp_queue_dequeue_commit_locked(queue);
	jwp_lock_release(queue->lock);
}

jwp_size_t jwp_queue_dequeue(struct jwp_queue *queue, jwp_u8 *buff, jwp_size_t size)
{
	jwp_lock_acquire(queue->lock);
	size = jwp_queue_dequeue_peek_locked(queue, buff, size);
	jwp_queue_dequeue_commit_locked(queue);
	jwp_lock_release(queue->lock);

	return size;
}

static jwp_size_t jwp_queue_get_free_size_locked(const struct jwp_queue *queue)
{
	if (queue->tail < queue->head)
	{
		return queue->head - queue->tail - 1;
	}

	return (sizeof(queue->buff) - 1) - (queue->tail - queue->head);
}

jwp_size_t jwp_queue_get_free_size(const struct jwp_queue *queue)
{
	jwp_size_t size;

	jwp_lock_acquire(((struct jwp_queue *) queue)->lock);
	size = jwp_queue_get_free_size_locked(queue);
	jwp_lock_release(((struct jwp_queue *) queue)->lock);

	return size;
}

static jwp_size_t jwp_queue_get_used_size_locked(const struct jwp_queue *queue)
{
	if (queue->head > queue->tail)
	{
		return sizeof(queue->buff) - (queue->head - queue->tail);
	}

	return queue->tail - queue->head;
}

jwp_size_t jwp_queue_get_used_size(const struct jwp_queue *queue)
{
	jwp_size_t size;

	jwp_lock_acquire(((struct jwp_queue *) queue)->lock);
	size = jwp_queue_get_used_size_locked(queue);
	jwp_lock_release(((struct jwp_queue *) queue)->lock);

	return size;
}

static jwp_bool jwp_queue_is_empty_locked(const struct jwp_queue *queue)
{
	return queue->head == queue->tail;
}

jwp_bool jwp_queue_is_empty(const struct jwp_queue *queue)
{
	jwp_bool res;

	jwp_lock_acquire(((struct jwp_queue *) queue)->lock);
	res = jwp_queue_is_empty_locked(queue);
	jwp_lock_release(((struct jwp_queue *) queue)->lock);

	return res;
}

static jwp_bool jwp_queue_is_full_locked(const struct jwp_queue *queue)
{
	return queue->head == queue->tail + 1 || (queue->head == queue->buff && queue->tail == queue->last);
}

jwp_bool jwp_queue_is_full(const struct jwp_queue *queue)
{
	jwp_bool res;

	jwp_lock_acquire(((struct jwp_queue *) queue)->lock);
	res = jwp_queue_is_full_locked(queue);
	jwp_lock_release(((struct jwp_queue *) queue)->lock);

	return res;
}

void jwp_queue_wait_data(struct jwp_queue *queue)
{
	jwp_lock_acquire(queue->lock);

#if JWP_POLL_ENABLE
	while (jwp_queue_is_empty_locked(queue))
#else
	if (jwp_queue_is_empty_locked(queue))
#endif
	{
#if JWP_QUEUE_NOTIFY_ENABLE
		jwp_signal_wait_locked(queue->data_signal, queue->lock);
#else
		jwp_lock_release(queue->lock);
		jwp_msleep(JWP_POLL_TIME);
		jwp_lock_acquire(queue->lock);
#endif
	}

	jwp_lock_release(queue->lock);
}

void jwp_queue_wait_space(struct jwp_queue *queue)
{
	jwp_lock_acquire(queue->lock);

#if JWP_POLL_ENABLE
	while (jwp_queue_is_full_locked(queue))
#else
	if (jwp_queue_is_full_locked(queue))
#endif
	{
#if JWP_QUEUE_NOTIFY_ENABLE
		jwp_signal_wait_locked(queue->space_signal, queue->lock);
#else
		jwp_lock_release(queue->lock);
		jwp_msleep(JWP_POLL_TIME);
		jwp_lock_acquire(queue->lock);
#endif
	}

	jwp_lock_release(queue->lock);
}

#if JWP_TX_LOOP_ENABLE || JWP_TX_PKG_TIMER_ENABLE
static jwp_bool jwp_queue_dequeue_package(struct jwp_queue *queue, struct jwp_header *hdr)
{
	jwp_size_t rdlen;

	rdlen = jwp_queue_dequeue(queue, (jwp_u8 *) hdr, JWP_HEADER_SIZE);
	if (rdlen < JWP_HEADER_SIZE)
	{
		return false;
	}

	rdlen = jwp_queue_dequeue(queue, JWP_GET_PAYLOAD(hdr), hdr->length);
	if (rdlen < hdr->length)
	{
		return false;
	}

	return true;
}
#endif
#endif

#if JWP_RX_DATA_QUEUE_ENABLE
static jwp_bool jwp_data_inqueue(struct jwp_desc *jwp)
{
	jwp_lock_acquire(jwp->lock);

	if (jwp->data_remain > 0)
	{
		jwp_size_t wrlen;
		struct jwp_queue *queue = jwp_get_queue(jwp, JWP_QUEUE_RX_DATA);

		jwp_lock_release(jwp->lock);

		wrlen = jwp_queue_inqueue(queue, jwp->data_head, jwp->data_remain);
		if (wrlen > 0)
		{
			jwp->data_received(jwp, jwp->data_head, wrlen);
#if JWP_RX_DATA_NOTIFY_ENABLE
			jwp_signal_notify(jwp->data_rx_signal, jwp->lock);
#endif

			jwp_lock_acquire(jwp->lock);

			if (wrlen < jwp->data_remain)
			{
				jwp->data_remain -= (jwp_u8) wrlen;
				jwp->data_head += wrlen;

				jwp_lock_release(jwp->lock);
				return false;
			}

			jwp->data_remain = 0;
			jwp_lock_release(jwp->lock);
		}
	}
	else
	{
		jwp_lock_release(jwp->lock);
	}

	return true;
}
#endif

#if JWP_RX_QUEUE_ENABLE
static jwp_bool jwp_queue_fill_package(struct jwp_desc *jwp)
{
	jwp_size_t rdlen;
	struct jwp_rx_package *pkg = &jwp->rx_pkg;
	struct jwp_queue *queue = jwp_get_queue(jwp, JWP_QUEUE_RX);

	if (pkg->header_remain)
	{
		if (pkg->head == pkg->body)
		{
			while (1)
			{
				rdlen = jwp_queue_dequeue_peek(queue, pkg->head, JWP_MAGIC_SIZE);
				if (rdlen < JWP_MAGIC_SIZE)
				{
					return false;
				}

				if (pkg->head[0] == JWP_MAGIC_LOW && pkg->head[1] == JWP_MAGIC_HIGH)
				{
					break;
				}

				jwp_queue_skip(queue, 1);
			}

			jwp_queue_dequeue_commit(queue);

			pkg->head = pkg->body + JWP_MAGIC_SIZE;
			pkg->header_remain = sizeof(pkg->header) - JWP_MAGIC_SIZE;
		}

		rdlen = jwp_queue_dequeue(queue, pkg->head, pkg->header_remain);
		if (rdlen < pkg->header_remain)
		{
			pkg->head += rdlen;
			pkg->header_remain -= (jwp_u8) rdlen;
			return false;
		}
		else
		{
			pkg->header_remain = 0;
			pkg->data_remain = pkg->header.length;
			pkg->head = pkg->payload;
		}
	}

	rdlen = jwp_queue_dequeue(queue, pkg->head, pkg->data_remain);
	if (rdlen == pkg->data_remain)
	{
		return true;
	}

	pkg->head += rdlen;
	pkg->data_remain -= (jwp_u8) rdlen;

	return false;
}
#else
static const jwp_u8 *jwp_package_find_magic(const jwp_u8 *buff, jwp_size_t size)
{
	const jwp_u8 *buff_end;

	for (buff_end = buff + size - 1; buff < buff_end; buff++)
	{
		if (buff[0] == JWP_MAGIC_LOW && buff[1] == JWP_MAGIC_HIGH)
		{
			return buff;
		}
	}

	if (buff[0] == JWP_MAGIC_LOW)
	{
		return buff;
	}

	return NULL;
}

static jwp_size_t jwp_package_write_data(struct jwp_desc *jwp, struct jwp_rx_package *pkg, const jwp_u8 *buff, jwp_size_t size)
{
	jwp_size_t size_bak = size;

	if (pkg->header_remain)
	{
		if (pkg->head < pkg->body + JWP_MAGIC_SIZE)
		{
			if (size == 0)
			{
				return 0;
			}

			if (pkg->head > pkg->body && buff[0] == JWP_MAGIC_HIGH)
			{
				buff++;
				size--;
			}
			else
			{
				const jwp_u8 *magic;

				magic = jwp_package_find_magic(buff, size);
				if (magic == NULL)
				{
					pkg->head = pkg->body;
					pkg->header_remain = sizeof(pkg->header);

					return size_bak;
				}

				size -= (magic - buff);
				if (size < JWP_MAGIC_SIZE)
				{
					pkg->head = pkg->body + size;
					pkg->header_remain = sizeof(pkg->header) - size;

					return size_bak;
				}

				buff = magic + JWP_MAGIC_SIZE;
				size -= JWP_MAGIC_SIZE;
			}

			pkg->head = pkg->body + JWP_MAGIC_SIZE;
			pkg->header_remain = sizeof(pkg->header) - JWP_MAGIC_SIZE;
		}

		if (size < pkg->header_remain)
		{
			jwp_memcpy(pkg->head, buff, size);
			pkg->head += size;
			pkg->header_remain -= (jwp_u8) size;

			return size_bak;
		}

		jwp_memcpy(pkg->head, buff, pkg->header_remain);
		buff += pkg->header_remain;
		size -= pkg->header_remain;

		pkg->header_remain = 0;
		pkg->data_remain = pkg->header.length;
		pkg->head = pkg->payload;
	}

	if (size < pkg->data_remain)
	{
		jwp_memcpy(pkg->head, buff, size);
		pkg->head += size;
		pkg->data_remain -= (jwp_u8) size;

		return size_bak;
	}

	jwp_memcpy(pkg->head, buff, pkg->data_remain);

	jwp_process_rx_package(jwp);

#if JWP_RX_DATA_QUEUE_ENABLE
	while (!jwp_data_inqueue(jwp))
	{
		jwp_msleep(JWP_POLL_TIME);
	}
#endif

	return size_bak - (size - pkg->data_remain);
}
#endif

// ============================================================

#if JWP_TIMER_ENABLE
static void jwp_timer_init(struct jwp_timer *timer, struct jwp_desc *jwp)
{
	jwp_lock_init(timer->lock);

	timer->jwp = jwp;
	timer->active = false;
	timer->handler = NULL;
}

static void jwp_hw_create_timer(struct jwp_timer *timer)
{
	struct jwp_desc *jwp = timer->jwp;

	while (!jwp->create_timer(timer))
	{
#if JWP_SHOW_ERROR
		jwp_printf("create timer fault!\n");
#endif
	}
}

static void jwp_timer_create(struct jwp_timer *timer, jwp_u32 msec)
{
	struct jwp_desc *jwp = timer->jwp;

	jwp_lock_acquire(timer->lock);

	if (timer->active)
	{
		if (msec < timer->msec)
		{
			jwp->delete_timer(timer);
		}
		else
		{
			jwp_lock_release(timer->lock);
			return;
		}
	}

	timer->msec = msec;
	jwp_hw_create_timer(timer);
	timer->active = true;

	jwp_lock_release(timer->lock);
}

#if JWP_TX_TIMER_ENABLE
static void jwp_timer_delete(struct jwp_timer *timer)
{
	struct jwp_desc *jwp = timer->jwp;

	jwp_lock_acquire(timer->lock);

	jwp->delete_timer(timer);
	timer->active = false;

	jwp_lock_release(timer->lock);
}
#endif

void jwp_timer_run(struct jwp_timer *timer)
{
	jwp_bool active = timer->handler(timer);

	jwp_lock_acquire(timer->lock);

	if (active)
	{
		jwp_hw_create_timer(timer);
	}
	else
	{
		timer->active = false;
	}

	jwp_lock_release(timer->lock);
}
#endif

#if JWP_TX_TIMER_ENABLE
static jwp_bool jwp_tx_timer_handler(struct jwp_timer *timer)
{
	struct jwp_desc *jwp = timer->jwp;
	struct jwp_header *hdr = &jwp->tx_pkg.header;

#if JWP_SHOW_ERROR
	if (timer->msec >= JWP_TX_TIMEOUT)
	{
		jwp_printf("send package timeout, tx_index = %d\n", jwp->tx_index);
	}
#endif

	jwp_hw_write_package(jwp, hdr);
	timer->msec = JWP_TX_TIMEOUT;

	return true;
}
#endif

#if JWP_TX_DATA_TIMER_ENABLE
static jwp_bool jwp_tx_data_timer_handler(struct jwp_timer *timer)
{
	struct jwp_package pkg;
	struct jwp_header *hdr = &pkg.header;
	struct jwp_desc *jwp = timer->jwp;
	struct jwp_queue *queue = jwp_get_queue(jwp, JWP_QUEUE_TX_DATA);

	hdr->length = jwp_queue_dequeue_peek(queue, hdr->payload, JWP_MAX_PAYLOAD);
	if (hdr->length == 0)
	{
		return false;
	}

	hdr->type = JWP_PKG_DATA;

#if JWP_TX_QUEUE_ENABLE == 0
	jwp_set_package_index(jwp, hdr);
#endif

	if (jwp_send_package(jwp, hdr, true))
	{
		jwp_queue_dequeue_commit(queue);
		timer->msec = 0;
	}
	else
	{
		timer->msec = JWP_POLL_TIME;
	}

	return true;
}
#endif

#if JWP_TX_PKG_TIMER_ENABLE
static jwp_bool jwp_tx_package_timer_handler(struct jwp_timer *timer)
{
	struct jwp_header *hdr;
	struct jwp_queue *queue;
	struct jwp_desc *jwp = timer->jwp;

	if (!jwp_check_and_set_send_pendding(jwp))
	{
		return false;
	}

	hdr = &jwp->tx_pkg.header;
	queue = jwp_get_queue(jwp, JWP_QUEUE_TX);

	if (jwp_queue_dequeue_package(queue, hdr))
	{
		jwp_set_package_index(jwp, hdr);
		jwp_timer_create(jwp_get_timer(jwp, JWP_TIMER_TX), 0);
	}
	else
	{
		jwp_set_send_pendding(jwp, false);
	}

	return false;
}
#endif

#if JWP_RX_PKG_TIMER_ENABLE
static jwp_bool jwp_rx_package_timer_handler(struct jwp_timer *timer)
{
	struct jwp_desc *jwp = timer->jwp;

#if JWP_RX_DATA_QUEUE_ENABLE
	if (!jwp_data_inqueue(jwp))
	{
		return true;
	}
#endif

	if (!jwp_queue_fill_package(jwp))
	{
		return false;
	}

	jwp_process_rx_package(jwp);

	return true;
}
#endif

// ============================================================

jwp_bool jwp_init(struct jwp_desc *jwp, void *data)
{
#if JWP_TIMER_ENABLE || JWP_QUEUE_ENABLE
	int i;
#endif

#if JWP_PRINTF_ENABLE
	jwp_global = jwp;
#endif

#if JWP_DEBUG_MEMBER
	jwp->state = JWP_STATE_INIT;
#endif

#if JWP_DEBUG
	if (jwp->hw_read == NULL || jwp->hw_write == NULL)
	{
		jwp_printf("jwp->hw_read == NULL || jwp->hw_write == NULL\n");
		return false;
	}

	if (jwp->data_received == NULL || jwp->package_received == NULL)
	{
		jwp_printf("jwp->data_received == NULL || jwp->package_received == NULL\n");
		return false;
	}

	if (jwp->send_complete == NULL)
	{
		jwp_printf("jwp->send_complete == NULL\n");
		return false;
	}

#if JWP_TIMER_ENABLE
	if (jwp->create_timer == NULL || jwp->delete_timer == NULL)
	{
		jwp_printf("jwp->create_timer == NULL || jwp->delete_timer == NULL\n");
		return false;
	}
#endif
#endif

	jwp_lock_init(jwp->lock);

#if JWP_TX_NOTIFY_ENABLE
	jwp_signal_init(jwp->tx_signal);
#endif

#if JWP_RX_DATA_NOTIFY_ENABLE
	jwp_signal_init(jwp->data_rx_signal);
#endif

#if JWP_RX_CMD_NOTIFY_ENABLE
	jwp_signal_init(jwp->command_rx_signal);
#endif

	jwp->private_data = data;

	jwp->tx_index = 0;
	jwp->rx_index = 0;

	jwp_rx_package_init(&jwp->rx_pkg);

#if JWP_QUEUE_ENABLE
	for (i = 0; i < JWP_QUEUE_COUNT; i++)
	{
		jwp_queue_init(jwp->queues + i);
	}
#endif

#if JWP_TIMER_ENABLE
	for (i = 0; i < JWP_TIMER_COUNT; i++)
	{
		jwp_timer_init(jwp->timers + i, jwp);
	}
#endif

#if JWP_TX_TIMER_ENABLE
	jwp->timers[JWP_TIMER_TX].handler = jwp_tx_timer_handler;
#endif

#if JWP_TX_DATA_TIMER_ENABLE
	jwp->timers[JWP_TIMER_TX_DATA].handler = jwp_tx_data_timer_handler;
#endif

#if JWP_TX_PKG_TIMER_ENABLE
	jwp->timers[JWP_TIMER_TX_PKG].handler = jwp_tx_package_timer_handler;
#endif

#if JWP_RX_PKG_TIMER_ENABLE
	jwp->timers[JWP_TIMER_RX_PKG].handler = jwp_rx_package_timer_handler;
#endif

	jwp->send_pendding = false;

#if JWP_DEBUG_MEMBER
#if JWP_TX_QUEUE_ENABLE
	jwp->queues[JWP_QUEUE_TX].name = "TX";
#endif

#if JWP_RX_QUEUE_ENABLE
	jwp->queues[JWP_QUEUE_RX].name = "RX";
#endif

#if JWP_TX_DATA_QUEUE_ENABLE
	jwp->queues[JWP_QUEUE_TX_DATA].name = "TX_DATA";
#endif

#if JWP_RX_DATA_QUEUE_ENABLE
	jwp->queues[JWP_QUEUE_RX_DATA].name = "RX_DATA";
#endif

#if JWP_TX_TIMER_ENABLE
	jwp->timers[JWP_TIMER_TX].name = "TX";
#endif

#if JWP_TX_DATA_TIMER_ENABLE
	jwp->timers[JWP_TIMER_TX_DATA].name = "TX_DATA";
#endif

#if JWP_TX_PKG_TIMER_ENABLE
	jwp->timers[JWP_TIMER_TX_PKG].name = "TX_PKG";
#endif

#if JWP_RX_PKG_TIMER_ENABLE
	jwp->timers[JWP_TIMER_RX_PKG].name = "RX_PKG";
#endif

	jwp->state = JWP_STATE_READY;
#endif

#if JWP_RX_DATA_QUEUE_ENABLE
	jwp->data_remain = 0;
#endif

	return true;
}

void jwp_send_ack_package(struct jwp_desc *jwp, jwp_u8 index)
{
	struct jwp_header hdr;

	hdr.type = JWP_PKG_ACK;
	hdr.index = index;
	hdr.length = 0;

	jwp_send_package(jwp, &hdr, false);
}

void jwp_send_sync_package(struct jwp_desc *jwp)
{
	struct jwp_header hdr;

	hdr.type = JWP_PKG_SYNC;
	hdr.index = jwp->tx_index;
	hdr.length = 0;

	jwp_send_package(jwp, &hdr, false);
}

#if JWP_TX_LOOP_ENABLE == 0 && JWP_TX_TIMER_ENABLE == 0
static jwp_bool jwp_send_and_wait_ack(struct jwp_desc *jwp, struct jwp_header *hdr)
{
#if JWP_TX_QUEUE_ENABLE
	while (1)
#else
	jwp_u16 retry;

	for (retry = JWP_TX_RETRY; retry; retry--)
#endif
	{
		jwp_hw_write_package(jwp, hdr);

		if (jwp_wait_tx_complete(jwp))
		{
			return true;
		}
	}

#if JWP_TX_QUEUE_ENABLE == 0
#if JWP_SHOW_ERROR
	jwp_printf("send package timeout, tx_index = %d\n", jwp->tx_index);
#endif

	jwp_set_send_pendding(jwp, false);

	return false;
#endif
}
#endif

static void jwp_process_rx_package(struct jwp_desc *jwp)
{
	struct jwp_rx_package *pkg = &jwp->rx_pkg;
	struct jwp_header *hdr = &pkg->header;

	jwp_rx_package_init(pkg);

#if JWP_DEBUG
	jwp_header_dump(hdr);
#endif

#if JWP_CHECKSUM_ENABLE
	{
		jwp_u8 checksum = hdr->checksum;
		jwp_u8 checksum_real = jwp_package_checksum(hdr);

		if (checksum != checksum_real)
		{
#if JWP_SHOW_ERROR
			jwp_printf("checksum not match, 0x%02x != 0x%02x\n", checksum, checksum_real);
#endif
			return;
		}
	}
#endif

	switch (hdr->type)
	{
	case JWP_PKG_CMD:
	case JWP_PKG_DATA:
		jwp_send_ack_package(jwp, hdr->index);
		jwp_lock_acquire(jwp->lock);
		if (hdr->index == (jwp_u8) (jwp->rx_index + 1))
		{
			jwp->rx_index = hdr->index;

			if (hdr->type == JWP_PKG_DATA)
			{
#if JWP_RX_DATA_QUEUE_ENABLE
				jwp->data_head = JWP_GET_PAYLOAD(hdr);
				jwp->data_remain = hdr->length;
				jwp_lock_release(jwp->lock);
#else
				jwp_lock_release(jwp->lock);
				jwp->data_received(jwp, JWP_GET_PAYLOAD(hdr), hdr->length);
#if JWP_RX_DATA_NOTIFY_ENABLE
				jwp_signal_notify(jwp->data_rx_signal, jwp->lock);
#endif
#endif
			}
			else
			{
				jwp_lock_release(jwp->lock);

				jwp->command_received(jwp, JWP_GET_PAYLOAD(hdr), hdr->length);
#if JWP_RX_CMD_NOTIFY_ENABLE
				jwp_signal_notify(jwp->command_rx_signal, jwp->lock);
#endif
			}
		}
		else
		{
			jwp_lock_release(jwp->lock);
#if JWP_SHOW_ERROR
			jwp_printf("throw data package %d, need %d\n", hdr->index, jwp->rx_index + 1);
#endif
		}
		break;

	case JWP_PKG_ACK:
		jwp_lock_acquire(jwp->lock);
		if (hdr->index == (jwp_u8) (jwp->tx_index + 1))
		{
			jwp->tx_index = hdr->index;
			jwp->send_pendding = false;

			jwp_lock_release(jwp->lock);

#if JWP_TX_TIMER_ENABLE
			jwp_timer_delete(jwp_get_timer(jwp, JWP_TIMER_TX));
#endif

#if JWP_TX_PKG_TIMER_ENABLE
			jwp_timer_create(jwp_get_timer(jwp, JWP_TIMER_TX_PKG), 0);
#endif

#if JWP_TX_NOTIFY_ENABLE
			jwp_signal_notify(jwp->tx_signal, jwp->lock);
#endif
			jwp->send_complete(jwp);
		}
		else
		{
			jwp_lock_release(jwp->lock);
#if JWP_SHOW_ERROR
			jwp_printf("throw ack package %d, need %d\n", hdr->index, jwp->tx_index + 1);
#endif
		}
		break;

	case JWP_PKG_SYNC:
		jwp_lock_acquire(jwp->lock);
		jwp->rx_index = hdr->index;
		jwp_lock_release(jwp->lock);
		break;

	default:
		jwp->package_received(jwp, hdr);
	}
}

jwp_size_t jwp_write_rx_data(struct jwp_desc *jwp, const jwp_u8 *buff, jwp_size_t size)
{
#if JWP_RX_QUEUE_ENABLE
	struct jwp_queue *queue = jwp_get_queue(jwp, JWP_QUEUE_RX);

	size = jwp_queue_inqueue(queue, buff, size);

#if JWP_RX_PKG_TIMER_ENABLE
	jwp_timer_create(jwp_get_timer(jwp, JWP_TIMER_RX_PKG), 0);
#elif JWP_RX_PKG_LOOP_ENABLE == 0
#error "must enable rx package loop or tx package timer when use rx queue"
#endif
#else
	struct jwp_rx_package *pkg = &jwp->rx_pkg;
	const jwp_u8 *buff_end = buff + size;

	while (buff < buff_end)
	{
		jwp_size_t wrlen;

		wrlen = jwp_package_write_data(jwp, pkg, buff, buff_end - buff);
		buff += wrlen;
	}
#endif

	return size;
}

jwp_bool jwp_send_package(struct jwp_desc *jwp, struct jwp_header *hdr, bool sync)
{
	hdr->magic_high = JWP_MAGIC_HIGH;
	hdr->magic_low = JWP_MAGIC_LOW;

	if (sync)
	{
#if JWP_TX_QUEUE_ENABLE
		struct jwp_queue *queue = jwp_get_queue(jwp, JWP_QUEUE_TX);

#if JWP_TX_LOOP_ENABLE
		return jwp_queue_inqueue_package(queue, hdr);
#elif JWP_TX_PKG_TIMER_ENABLE
		jwp_bool res = jwp_queue_inqueue_package(queue, hdr);
		jwp_timer_create(jwp_get_timer(jwp, JWP_TIMER_TX_PKG), 0);
		return res;
#else
#error "must enable tx loop or tx package timer when tx queue enabled"
#endif
#else
#if JWP_POLL_ENABLE && JWP_TX_DATA_TIMER_ENABLE == 0
		while (!jwp_wait_and_set_send_pendding(jwp));
#else
		if (!jwp_check_and_set_send_pendding(jwp))
		{
			return false;
		}
#endif

		jwp_set_package_index(jwp, hdr);

#if JWP_TX_TIMER_ENABLE
		jwp_memcpy(&jwp->tx_pkg, hdr, JWP_HEADER_SIZE + hdr->length);
		jwp_timer_create(jwp_get_timer(jwp, JWP_TIMER_TX), 0);
		return true;
#else
		return jwp_send_and_wait_ack(jwp, hdr);
#endif
#endif
	}
	else
	{
		jwp_hw_write_package(jwp, hdr);

		return true;
	}
}

jwp_size_t jwp_send_data(struct jwp_desc *jwp, const void *buff, jwp_size_t size)
{
#if JWP_TX_DATA_QUEUE_ENABLE
	struct jwp_queue *queue = jwp_get_queue(jwp, JWP_QUEUE_TX_DATA);

#if JWP_POLL_ENABLE
#if JWP_TX_DATA_LOOP_ENABLE
	jwp_queue_inqueue_all(queue, (jwp_u8 *) buff, size);
#else
	if (size < JWP_QUEUE_SIZE)
	{
		jwp_queue_inqueue_all(queue, buff, size);
	}
	else
	{
		size = jwp_queue_inqueue(queue, buff, size);
	}
#endif
#else
	size = jwp_queue_inqueue(queue, (const jwp_u8 *) buff, size);
#endif

#if JWP_TX_DATA_TIMER_ENABLE
#if JWP_TX_LATENCY > 0
	jwp_timer_create(jwp_get_timer(jwp, JWP_TIMER_TX_DATA), jwp_queue_get_used_size(queue) < JWP_MAX_PAYLOAD ? JWP_TX_LATENCY : JWP_POLL_TIME);
#else
	jwp_timer_create(jwp_get_timer(jwp, JWP_TIMER_TX_DATA), JWP_POLL_TIME);
#endif
#elif JWP_TX_DATA_LOOP_ENABLE == 0
#error "please disable tx data queue when not use tx data loop and tx data timer"
#endif

	return size;
#else
	struct jwp_package pkg;
	struct jwp_header *hdr = &pkg.header;
	const jwp_u8 *p = (const jwp_u8 *) buff, *p_end = p + size;

	while (p < p_end)
	{
		jwp_size_t length;

		length = p_end - p;
		if (length > JWP_MAX_PAYLOAD)
		{
			length = JWP_MAX_PAYLOAD;
		}

		hdr->type = JWP_PKG_DATA;
		hdr->length = (jwp_u8) length;
		jwp_memcpy(JWP_GET_PAYLOAD(hdr), p, length);

		if (!jwp_send_package(jwp, hdr, true))
		{
			jwp_msleep(JWP_POLL_TIME);
			break;
		}

		p += length;
	}

	return p - (const jwp_u8 *) buff;
#endif
}

jwp_size_t jwp_recv_data(struct jwp_desc *jwp, void *buff, jwp_size_t size)
{
#if JWP_RX_DATA_QUEUE_ENABLE
	struct jwp_queue *queue = jwp_get_queue(jwp, JWP_QUEUE_RX_DATA);

	return jwp_queue_dequeue(queue, (jwp_u8 *) buff, size);
#else
	return 0;
#endif
}

jwp_bool jwp_send_command(struct jwp_desc *jwp, const void *command, jwp_size_t size)
{
	struct jwp_package pkg;
	struct jwp_header *hdr = &pkg.header;

#if JWP_SHOW_ERROR
	if (size > JWP_MAX_PAYLOAD)
	{
		jwp_printf("command too large!\n");
		return false;
	}
#endif

	hdr->type = JWP_PKG_CMD;
	hdr->length = (jwp_u8) size;
	jwp_memcpy(JWP_GET_PAYLOAD(hdr), command, size);

	return jwp_send_package(jwp, hdr, true);
}

// ============================================================

jwp_bool jwp_wait_tx_complete(struct jwp_desc *jwp)
{
#if JWP_TX_NOTIFY_ENABLE
	jwp_bool res;

	jwp_lock_acquire(jwp->lock);

	if (jwp->send_pendding)
	{
		jwp_signal_timedwait_locked(jwp->tx_signal, jwp->lock, JWP_TX_TIMEOUT);
		res = !jwp->send_pendding;
	}
	else
	{
		res = true;
	}

	jwp_lock_release(jwp->lock);

	return res;
#else
	jwp_u32 count;

	for (count = JWP_TX_TIMEOUT / JWP_POLL_TIME; count; count--)
	{
		jwp_lock_acquire(jwp->lock);

		if (!jwp->send_pendding)
		{
			jwp_lock_release(jwp->lock);
			return true;
		}

		jwp_lock_release(jwp->lock);
		jwp_msleep(JWP_POLL_TIME);
	}

	return false;
#endif
}

#if JWP_TX_DATA_LOOP_ENABLE
void jwp_tx_data_loop(struct jwp_desc *jwp)
{
	struct jwp_package pkg;
	struct jwp_header *hdr = &pkg.header;
	struct jwp_queue *queue = jwp_get_queue(jwp, JWP_QUEUE_TX_DATA);

	while (1)
	{
		hdr->length = (jwp_u8) jwp_queue_dequeue_peek(queue, JWP_GET_PAYLOAD(hdr), JWP_MAX_PAYLOAD);
		if (hdr->length == 0)
		{
			jwp_queue_wait_data(queue);

#if JWP_TX_LATENCY > 0
			if (jwp_queue_get_used_size(queue) < JWP_MAX_PAYLOAD)
			{
				jwp_msleep(JWP_TX_LATENCY);
			}
#endif

			continue;
		}

		hdr->type = JWP_PKG_DATA;

#if JWP_TX_QUEUE_ENABLE == 0
		jwp_set_package_index(jwp, hdr);
#endif

		if (jwp_send_package(jwp, hdr, true))
		{
			jwp_queue_dequeue_commit(queue);
		}
	}
}
#endif

#if JWP_TX_LOOP_ENABLE
void jwp_tx_loop(struct jwp_desc *jwp)
{
	struct jwp_header *hdr = &jwp->tx_pkg.header;
	struct jwp_queue *queue = jwp_get_queue(jwp, JWP_QUEUE_TX);

	while (1)
	{
		if (!jwp_queue_dequeue_package(queue, hdr))
		{
			jwp_queue_wait_data(queue);
			continue;
		}

		jwp_lock_acquire(jwp->lock);

		hdr->index = jwp->tx_index + 1;
		jwp->send_pendding = true;

		jwp_lock_release(jwp->lock);

		while (1)
		{
			jwp_hw_write_package(jwp, hdr);

			if (jwp_wait_tx_complete(jwp))
			{
				break;
			}

#if JWP_SHOW_ERROR
			jwp_printf("send package timeout, tx_index = %d\n", jwp->tx_index);
#endif
		}
	}
}
#endif

#if JWP_RX_LOOP_ENABLE
void jwp_rx_loop(struct jwp_desc *jwp)
{
	while (1)
	{
		jwp_size_t rdlen;
		jwp_u8 buff[JWP_MTU], *p;

		rdlen = jwp->hw_read(jwp, buff, sizeof(buff));
		p = buff;

		while (1)
		{
			jwp_size_t wrlen;

			wrlen = jwp_write_rx_data(jwp, p, rdlen);
			if (wrlen == rdlen)
			{
				break;
			}

			p += wrlen;
			rdlen -= wrlen;

			jwp_msleep(JWP_POLL_TIME);
		}
	}
}
#endif

#if JWP_RX_PKG_LOOP_ENABLE
void jwp_rx_package_loop(struct jwp_desc *jwp)
{
	struct jwp_queue *queue = jwp_get_queue(jwp, JWP_QUEUE_RX);
#if JWP_RX_DATA_QUEUE_ENABLE
	struct jwp_queue *data_queue = jwp_get_queue(jwp, JWP_QUEUE_RX_DATA);
#endif

	while (1)
	{
		while (!jwp_queue_fill_package(jwp))
		{
			jwp_queue_wait_data(queue);
		}

		jwp_process_rx_package(jwp);

#if JWP_RX_DATA_QUEUE_ENABLE
		while (!jwp_data_inqueue(jwp))
		{
			jwp_queue_wait_space(data_queue);
		}
#endif
	}
}
#endif
