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

#include <cavan.h>
#include <cavan/jwp.h>

#define JWP_DEBUG	0

// ============================================================

#if JWP_TX_TIMER_ENABLE && (JWP_TX_QUEUE_ENABLE == 0 && JWP_TX_LATENCY_ENABLE == 0)
#error "don't need tx timer when tx queue and tx latency disabled"
#endif

#if JWP_TX_LATENCY_ENABLE && (JWP_TX_TIMER_ENABLE == 0 || JWP_TX_DATA_QUEUE_ENABLE == 0)
#error "must enable timer and tx data queue when use tx latency"
#endif

#if JWP_TX_LATENCY_ENABLE && JWP_TX_QUEUE_ENABLE && JWP_TX_LOOP_ENABLE == 0
#error "don't enable tx latency and tx queue at the same time when tx loop disabled"
#endif

#if JWP_TX_DATA_LOOP_ENABLE && JWP_TX_DATA_QUEUE_ENABLE == 0
#error "must enable tx data queue when use tx data loop"
#endif

#if JWP_TX_DATA_LOOP_ENABLE && JWP_TX_LATENCY_ENABLE
#error "don't enable tx data loop and tx latency at the same time"
#endif

#if JWP_TX_LOOP_ENABLE && JWP_TX_QUEUE_ENABLE == 0
#error "must enable tx queue when use tx loop"
#endif

#if JWP_RX_LOOP_ENABLE && JWP_RX_QUEUE_ENABLE == 0
#error "must enable rx queue when use rx loop"
#endif

// ============================================================

static void jwp_process_rx_package(struct jwp_desc *desc, struct jwp_header *hdr);

void jwp_header_dump(const struct jwp_header *hdr)
{
	jwp_println("index = %d, type = %d, length = %d, checksum = 0x%02x", hdr->index, hdr->type, hdr->length, hdr->checksum);
}

void jwp_package_dump(const struct jwp_package *pkg)
{
	jwp_header_dump(&pkg->header);
}

static void jwp_rx_package_init(struct jwp_rx_package *pkg)
{
	pkg->head = pkg->body;
	pkg->header_remain = sizeof(pkg->header);
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

// ============================================================

void jwp_queue_init(struct jwp_queue *queue)
{
	jwp_lock_init(queue->lock);

#if JWP_QUEUE_NOTIFY_ENABLE
	jwp_cond_init(queue->data_cond);
	jwp_cond_init(queue->space_cond);
#endif

	queue->tail = queue->head = queue->buff;
	queue->tail_peek = queue->head_peek = queue->tail;
	queue->last = queue->tail + sizeof(queue->buff) - 1;
}

jwp_size_t jwp_queue_inqueue_peek(struct jwp_queue *queue, const jwp_u8 *buff, jwp_size_t size)
{
	jwp_size_t length;

	jwp_lock_acquire(queue->lock);

	if (queue->tail < queue->head)
	{
		length = queue->head - queue->tail - 1;
		if (size > length)
		{
			size = length;
		}
	}
	else
	{
		length = queue->last - queue->tail;
		if (size > length)
		{
			if (queue->head > queue->buff)
			{
				jwp_size_t left;

				jwp_memcpy(queue->tail, buff, ++length);
				size -= length;

				left = queue->head - queue->buff - 1;
				if (size < left)
				{
					left = size;
				}

				jwp_memcpy(queue->buff, buff + length, left);
				queue->tail_peek = queue->buff + left;

				jwp_lock_release(queue->lock);

				return length + left;
			}

			size = length;
		}
	}

	jwp_memcpy(queue->tail, buff, size);
	queue->tail_peek = queue->tail + size;

	jwp_lock_release(queue->lock);

	return size;
}

void jwp_queue_inqueue_commit(struct jwp_queue *queue)
{
	jwp_lock_acquire(queue->lock);

	queue->tail = queue->tail_peek;

#if JWP_QUEUE_NOTIFY_ENABLE
	jwp_cond_notify(queue->data_cond);
#endif

	jwp_lock_release(queue->lock);
}

jwp_size_t jwp_queue_inqueue(struct jwp_queue *queue, const jwp_u8 *buff, jwp_size_t size)
{
	size = jwp_queue_inqueue_peek(queue, buff, size);
	jwp_queue_inqueue_commit(queue);

	return size;
}

jwp_bool jwp_queue_try_inqueue(struct jwp_queue *queue, const jwp_u8 *buff, jwp_size_t size)
{
	jwp_size_t wrlen;

	wrlen = jwp_queue_inqueue_peek(queue, buff, size);
	if (wrlen < size)
	{
		return false;
	}

	jwp_queue_inqueue_commit(queue);

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

jwp_size_t jwp_queue_dequeue_peek(struct jwp_queue *queue, jwp_u8 *buff, jwp_size_t size)
{
	jwp_size_t length;

	jwp_lock_acquire(queue->lock);

	if (queue->head > queue->tail)
	{
		length = queue->last - queue->head + 1;
		if (length > size)
		{
			length = size;
		}
		else
		{
			jwp_size_t left;

			jwp_memcpy(buff, queue->head, length);

			size -= length;
			left = queue->tail - queue->buff;
			if (size < left)
			{
				left = size;
			}

			jwp_memcpy(buff + length, queue->buff, left);
			queue->head_peek = queue->buff + left;

			jwp_lock_release(queue->lock);

			return length + left;
		}
	}
	else
	{
		length = queue->tail - queue->head;
		if (length > size)
		{
			length = size;
		}
	}

	jwp_memcpy(buff, queue->head, length);
	queue->head_peek = queue->head + length;

	jwp_lock_release(queue->lock);

	return length;
}

void jwp_queue_dequeue_commit(struct jwp_queue *queue)
{
	jwp_lock_acquire(queue->lock);

	queue->head = queue->head_peek;

#if JWP_QUEUE_NOTIFY_ENABLE
	jwp_cond_notify(queue->space_cond);
#endif

	jwp_lock_release(queue->lock);
}

jwp_size_t jwp_queue_dequeue(struct jwp_queue *queue, jwp_u8 *buff, jwp_size_t size)
{
	size = jwp_queue_dequeue_peek(queue, buff, size);
	jwp_queue_dequeue_commit(queue);

	return size;
}

#if JWP_RX_QUEUE_ENABLE
jwp_size_t jwp_queue_skip(struct jwp_queue *queue, jwp_size_t size)
{
	jwp_size_t length;

	jwp_lock_acquire(queue->lock);

	if (queue->head > queue->tail)
	{
		length = queue->last - queue->head + 1;
		if (length > size)
		{
			length = size;
		}
		else
		{
			jwp_size_t left;

			size -= length;
			left = queue->tail - queue->buff;
			if (size < left)
			{
				left = size;
			}

			queue->head = queue->buff + left;

			jwp_lock_release(queue->lock);

			return length + left;
		}
	}
	else
	{
		length = queue->tail - queue->head;
		if (length > size)
		{
			length = size;
		}
	}

	queue->head = queue->head + length;

	jwp_lock_release(queue->lock);

	return length;
}
#endif

jwp_bool jwp_queue_dequeue_package(struct jwp_queue *queue, struct jwp_header *hdr)
{
	jwp_size_t rdlen;

	rdlen = jwp_queue_dequeue(queue, (jwp_u8 *) hdr, JWP_HEADER_SIZE);
	if (rdlen < JWP_HEADER_SIZE)
	{
		return false;
	}

	rdlen = jwp_queue_dequeue(queue, hdr->payload, hdr->length);
	if (rdlen < hdr->length)
	{
		return false;
	}

	return true;
}

#if JWP_RX_QUEUE_ENABLE
static jwp_bool jwp_queue_fill_package(struct jwp_queue *queue, struct jwp_rx_package *pkg)
{
	jwp_size_t rdlen;

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
			pkg->header_remain -= rdlen;
			return false;
		}
		else
		{
			pkg->header_remain = 0;
			pkg->data_remain = pkg->header.length;
			pkg->head = pkg->header.payload;
		}
	}

	rdlen = jwp_queue_dequeue(queue, pkg->head, pkg->data_remain);
	if (rdlen < pkg->data_remain)
	{
		pkg->head += rdlen;
		pkg->data_remain -= rdlen;
		return false;
	}
	else
	{
		pkg->head = pkg->body;
		pkg->header_remain = sizeof(pkg->header);

		return true;
	}
}
#else
static jwp_u8 *jwp_package_find_magic(const jwp_u8 *buff, jwp_size_t size)
{
	const jwp_u8 *buff_end;

	for (buff_end = buff + size - 1; buff < buff_end; buff++)
	{
		if (buff[0] == JWP_MAGIC_LOW && buff[1] == JWP_MAGIC_HIGH)
		{
			return (jwp_u8 *) buff;
		}
	}

	if (buff[0] == JWP_MAGIC_LOW)
	{
		return (jwp_u8 *) buff;
	}

	return NULL;
}

static jwp_size_t jwp_package_write_data(struct jwp_desc *desc, struct jwp_package *pkg, const jwp_u8 *buff, jwp_size_t size)
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
			pkg->header_remain -= size;

			return size_bak;
		}

		jwp_memcpy(pkg->head, buff, pkg->header_remain);
		buff += pkg->header_remain;
		size -= pkg->header_remain;

		pkg->header_remain = 0;
		pkg->data_remain = pkg->header.length;
		pkg->head = pkg->header.payload;
	}

	if (size < pkg->data_remain)
	{
		jwp_memcpy(pkg->head, buff, size);
		pkg->head += size;
		pkg->data_remain -= size;

		return size_bak;
	}

	jwp_memcpy(pkg->head, buff, pkg->data_remain);
	jwp_process_rx_package(desc, &pkg->header);

	pkg->head = pkg->body;
	pkg->header_remain = sizeof(pkg->header);

	return size_bak - (size - pkg->data_remain);
}
#endif

jwp_size_t jwp_queue_get_free_size(struct jwp_queue *queue)
{
	if (queue->tail < queue->head)
	{
		return queue->head - queue->tail - 1;
	}

	return (queue->last - queue->tail) + (queue->head - queue->buff);
}

jwp_size_t jwp_queue_get_fill_size(struct jwp_queue *queue)
{
	if (queue->head > queue->tail)
	{
		return (queue->last - queue->head) + (queue->tail - queue->buff);
	}

	return queue->tail - queue->head;
}

jwp_bool jwp_queue_empty(struct jwp_queue *queue)
{
	return queue->head == queue->tail;
}

jwp_bool jwp_queue_full(struct jwp_queue *queue)
{
	return queue->head == queue->tail + 1 || (queue->head == queue->buff && queue->tail == queue->last);
}

// ============================================================

jwp_bool jwp_init(struct jwp_desc *desc, void *data)
{
	int i;

#if JWP_DEBUG
	if (desc->hw_read == NULL || desc->hw_write == NULL)
	{
		jwp_println("desc->hw_read == NULL || desc->hw_write == NULL");
		return false;
	}

	if (desc->data_received == NULL || desc->package_received == NULL)
	{
		jwp_println("desc->data_received == NULL || desc->package_received == NULL");
		return false;
	}

	if (desc->send_complete == NULL)
	{
		jwp_println("desc->send_complete == NULL");
		return false;
	}

#if JWP_TX_TIMER_ENABLE
	if (desc->create_timer == NULL || desc->delete_timer == NULL)
	{
		jwp_println("desc->create_timer == NULL || desc->delete_timer == NULL");
		return false;
	}
#endif
#endif

	jwp_lock_init(desc->lock);

#if JWP_TX_NOTIFY_ENABLE
	jwp_cond_init(desc->tx_cond);
#endif

#if JWP_RX_DATA_NOTIFY_ENABLE
	jwp_cond_init(desc->data_rx_cond);
#endif

#if JWP_RX_CMD_NOTIFY_ENABLE
	jwp_cond_init(desc->command_rx_cond);
#endif

#if JWP_TX_TIMER_ENABLE
	desc->tx_timer = JWP_TIMER_INVALID;
#endif

#if JWP_TX_LATENCY_ENABLE
	desc->tx_latency_timer = JWP_TIMER_INVALID;
#endif

	desc->private_data = data;

	desc->tx_index = 0;
	desc->rx_index = 0;

	jwp_rx_package_init(&desc->rx_pkg);

	for (i = 0; i < JWP_QUEUE_COUNT; i++)
	{
		jwp_queue_init(desc->queues + i);
	}

	desc->send_pendding = false;

	return true;
}

void jwp_send_ack_package(struct jwp_desc *desc, jwp_u8 index)
{
	struct jwp_header hdr;

	hdr.type = JWP_PKG_ACK;
	hdr.index = index;
	hdr.length = 0;

	jwp_send_package(desc, &hdr, false);
}

void jwp_send_sync_package(struct jwp_desc *desc)
{
	struct jwp_header hdr;

	hdr.type = JWP_PKG_SYNC;
	hdr.index = desc->tx_index;
	hdr.length = 0;

	jwp_send_package(desc, &hdr, false);
}

static void jwp_hw_write(struct jwp_desc *desc, const jwp_u8 *buff, size_t size)
{
	while (size > 0)
	{
		jwp_size_t wrlen;

		wrlen = desc->hw_write(desc, buff, size);
		buff += wrlen;
		size -= wrlen;
	}
}

static inline void jwp_hw_write_package(struct jwp_desc *desc, struct jwp_header *hdr)
{
#if JWP_CHECKSUM_ENABLE
	hdr->checksum = jwp_package_checksum(hdr);
#endif

	jwp_hw_write(desc, (jwp_u8 *) hdr, JWP_HEADER_SIZE + hdr->length);
}

#if JWP_TX_LOOP_ENABLE == 0
#if JWP_TX_TIMER_ENABLE
static void jwp_send_timeout_handler(struct jwp_desc *desc, jwp_timer timer)
{
	struct jwp_header *hdr = &desc->tx_pkg.header;

	jwp_hw_write_package(desc, hdr);
	desc->tx_timer = desc->create_timer(desc, timer, JWP_SEND_TIMEOUT, jwp_send_timeout_handler);
}
#else
static jwp_bool jwp_send_and_wait_ack(struct jwp_desc *desc, struct jwp_header *hdr)
{
#if JWP_TX_QUEUE_ENABLE
	while (1)
#else
	jwp_u16 retry;

	for (retry = JWP_SEND_RETRY; retry; retry--)
#endif
	{
		jwp_hw_write_package(desc, hdr);

		if (jwp_wait_tx_complete(desc))
		{
			return true;
		}
	}

#if JWP_TX_QUEUE_ENABLE == 0
#if JWP_DEBUG
	jwp_pr_red_info("send package timeout");
#endif

	desc->send_pendding = false;

	return false;
#endif
}
#endif

#if JWP_TX_QUEUE_ENABLE
static jwp_bool jwp_process_tx_data(struct jwp_desc *desc)
{
	struct jwp_header *hdr;
	struct jwp_queue *queue;

#if JWP_WAIT_ENABLE
	while (jwp_wait_tx_complete(desc) == false);
#else
	if (desc->send_pendding)
	{
		return false;
	}
#endif

	hdr = &desc->tx_pkg.header;
	queue = jwp_get_queue(desc, JWP_QUEUE_TX);

	if (jwp_queue_dequeue_package(queue, hdr) == false)
	{
		return false;
	}

	desc->send_pendding = true;
	hdr->index = desc->tx_index + 1;

#if JWP_TX_TIMER_ENABLE
	jwp_send_timeout_handler(desc, desc->tx_timer);
#else
	jwp_send_and_wait_ack(desc, hdr);
#endif

	return true;
}
#endif
#endif

static void jwp_process_rx_package(struct jwp_desc *desc, struct jwp_header *hdr)
{
#if JWP_DEBUG
	jwp_header_dump(hdr);
#endif

#if JWP_CHECKSUM_ENABLE
	{
		jwp_u8 checksum = hdr->checksum;
		jwp_u8 checksum_real = jwp_package_checksum(hdr);

		if (checksum != checksum_real)
		{
#if JWP_DEBUG
			jwp_pr_red_info("checksum not match, 0x%02x != 0x%02x", checksum, checksum_real);
#endif
			return;
		}
	}
#endif

	switch (hdr->type)
	{
	case JWP_PKG_CMD:
	case JWP_PKG_DATA:
		jwp_send_ack_package(desc, hdr->index);
		jwp_lock_acquire(desc->lock);
		if (hdr->index == (jwp_u8) (desc->rx_index + 1))
		{
			desc->rx_index = hdr->index;

			jwp_lock_release(desc->lock);

			if (hdr->type == JWP_PKG_DATA)
			{
#if JWP_RX_DATA_QUEUE_ENABLE
				struct jwp_queue *queue = jwp_get_queue(desc, JWP_QUEUE_RX_DATA);

				jwp_queue_inqueue_all(queue, hdr->payload, hdr->length);
#endif

#if JWP_RX_DATA_NOTIFY_ENABLE
				jwp_cond_notify(desc->data_rx_cond);
#endif
				desc->data_received(desc, hdr->payload, hdr->length);
			}
			else
			{
#if JWP_RX_CMD_NOTIFY_ENABLE
				jwp_cond_notify(desc->command_rx_cond);
#endif
				desc->command_received(desc, hdr->payload, hdr->length);
			}
		}
		else
		{
			jwp_lock_release(desc->lock);

#if JWP_DEBUG
			jwp_pr_red_info("throw data package %d, need %d", hdr->index, desc->rx_index + 1);
#endif
		}
		break;

	case JWP_PKG_ACK:
		jwp_lock_acquire(desc->lock);
		if (hdr->index == (jwp_u8) (desc->tx_index + 1))
		{
#if JWP_TX_QUEUE_ENABLE
			struct jwp_queue *queue = jwp_get_queue(desc, JWP_QUEUE_TX);
#endif

			desc->tx_index = hdr->index;
			desc->send_pendding = false;

			jwp_lock_release(desc->lock);

#if JWP_TX_TIMER_ENABLE
			if (desc->tx_timer != JWP_TIMER_INVALID)
			{
				desc->delete_timer(desc, desc->tx_timer);
			}
#endif

#if JWP_TX_QUEUE_ENABLE
			jwp_queue_dequeue_commit(queue);
#endif

#if JWP_TX_NOTIFY_ENABLE
			jwp_cond_notify(desc->tx_cond);
#endif
			desc->send_complete(desc);
		}
		else
		{
			jwp_lock_release(desc->lock);
#if JWP_DEBUG
			jwp_pr_red_info("throw ack package %d, need %d", hdr->index, desc->tx_index + 1);
#endif
		}
		break;

	case JWP_PKG_SYNC:
		jwp_lock_acquire(desc->lock);
		desc->rx_index = hdr->index;
		jwp_lock_release(desc->lock);
		break;

	default:
		desc->package_received(desc, hdr);
	}
}

#if JWP_RX_QUEUE_ENABLE && JWP_RX_LOOP_ENABLE == 0
static jwp_bool jwp_process_rx_data(struct jwp_desc *desc)
{
	struct jwp_package *pkg = &desc->rx_pkg;
	struct jwp_queue *queue = jwp_get_queue(desc, JWP_QUEUE_RX);

	while (jwp_queue_fill_package(queue, pkg))
	{
		jwp_process_rx_package(desc, pkg);
	}

	return true;
}
#endif

jwp_size_t jwp_write_rx_data(struct jwp_desc *desc, const jwp_u8 *buff, jwp_size_t size)
{
#if JWP_RX_QUEUE_ENABLE
	struct jwp_queue *queue = jwp_get_queue(desc, JWP_QUEUE_RX);

	size = jwp_queue_inqueue(queue, buff, size);

#if JWP_RX_LOOP_ENABLE == 0
	jwp_process_rx_data(desc);
#endif
#else
	struct jwp_package *pkg = &desc->rx_pkg;
	const jwp_u8 *buff_end = buff + size;

	while (buff < buff_end)
	{
		jwp_size_t wrlen;

		wrlen = jwp_package_write_data(desc, pkg, buff, buff_end - buff);
		buff += wrlen;
	}
#endif

	return size;
}

jwp_bool jwp_send_package(struct jwp_desc *desc, struct jwp_header *hdr, bool sync)
{
	hdr->magic_high = JWP_MAGIC_HIGH;
	hdr->magic_low = JWP_MAGIC_LOW;

	if (sync)
	{
#if JWP_TX_QUEUE_ENABLE
		struct jwp_queue *queue = jwp_get_queue(desc, JWP_QUEUE_TX);

#if JWP_TX_LOOP_ENABLE
		return jwp_queue_inqueue_package(queue, hdr);
#else
		jwp_bool res = jwp_queue_inqueue_package(queue, hdr);

		jwp_process_tx_data(desc);

		return res;
#endif
#else
#if JWP_WAIT_ENABLE
		while (jwp_wait_tx_complete(desc) == false);
#else
		if (desc->send_pendding)
		{
			return false;
		}
#endif

		desc->send_pendding = true;
		hdr->index = desc->tx_index + 1;

#if JWP_TX_TIMER_ENABLE
		jwp_memcpy(&desc->tx_pkg, hdr, JWP_HEADER_SIZE + hdr->length);
		jwp_send_timeout_handler(desc, desc->tx_timer);
		return true;
#else
		return jwp_send_and_wait_ack(desc, hdr);
#endif
#endif
	}
	else
	{
		jwp_hw_write_package(desc, hdr);

		return true;
	}
}

#if JWP_TX_LATENCY_ENABLE
static void jwp_tx_lantency_handler(struct jwp_desc *desc, jwp_timer timer)
{
	struct jwp_package pkg;
	struct jwp_header *hdr = &pkg.header;
	struct jwp_queue *queue = jwp_get_queue(desc, JWP_QUEUE_TX_DATA);

	while (1)
	{
		hdr->length = jwp_queue_dequeue_peek(queue, hdr->payload, JWP_MAX_PAYLOAD);
		if (hdr->length == 0)
		{
			break;
		}

		hdr->type = JWP_PKG_DATA;

#if JWP_TX_QUEUE_ENABLE == 0
		hdr->index = desc->tx_index + 1;
#endif

		if (jwp_send_package(desc, hdr, true))
		{
			jwp_queue_dequeue_commit(queue);
		}
	}
}
#endif

jwp_size_t jwp_send_data(struct jwp_desc *desc, const void *buff, jwp_size_t size)
{
#if JWP_TX_DATA_QUEUE_ENABLE
	struct jwp_queue *queue = jwp_get_queue(desc, JWP_QUEUE_TX_DATA);

#if JWP_WAIT_ENABLE
#if JWP_TX_DATA_LOOP_ENABLE
	jwp_queue_inqueue_all(queue, buff, size);
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
	size = jwp_queue_inqueue(queue, buff, size);
#endif

#if JWP_TX_LATENCY_ENABLE
	if (jwp_queue_get_fill_size(queue) < JWP_MTU)
	{
		desc->tx_latency_timer = desc->create_timer(desc, desc->tx_latency_timer, JWP_LATENCY_TIME, jwp_tx_lantency_handler);
	}
	else
	{
		if (desc->tx_latency_timer != JWP_TIMER_INVALID)
		{
			desc->delete_timer(desc, desc->tx_latency_timer);
		}

		jwp_tx_lantency_handler(desc, desc->tx_latency_timer);
	}
#elif JWP_TX_DATA_LOOP_ENABLE == 0
#error "must enable tx data loop or tx latency when use tx data queue"
#endif

	return size;
#else
	struct jwp_package pkg;
	struct jwp_header *hdr = &pkg.header;
	const jwp_u8 *p = buff, *p_end = p + size;

	while (p < p_end)
	{
		size = p_end - p;

		hdr->type = JWP_PKG_DATA;
		hdr->length = size > JWP_MAX_PAYLOAD ? JWP_MAX_PAYLOAD : size;
		jwp_memcpy(hdr->payload, buff, hdr->length);

		if (jwp_send_package(desc, hdr, true) == false)
		{
			break;
		}

		p += hdr->length;
	}

	return p - (const jwp_u8 *) buff;
#endif
}

jwp_size_t jwp_recv_data(struct jwp_desc *desc, void *buff, jwp_size_t size)
{
#if JWP_RX_DATA_QUEUE_ENABLE
	struct jwp_queue *queue = jwp_get_queue(desc, JWP_QUEUE_RX_DATA);

	return jwp_queue_dequeue(queue, buff, size);
#else
	return 0;
#endif
}

jwp_bool jwp_send_command(struct jwp_desc *desc, const void *command, jwp_size_t size)
{
	struct jwp_package pkg;
	struct jwp_header *hdr = &pkg.header;

#if JWP_DEBUG
	if (size > JWP_MAX_PAYLOAD)
	{
		jwp_pr_red_info("command too large!");
		return false;
	}
#endif

	hdr->type = JWP_PKG_CMD;
	hdr->length = size;
	jwp_memcpy(hdr->payload, command, size);

	return jwp_send_package(desc, hdr, true);
}

// ============================================================

jwp_bool jwp_wait_tx_complete(struct jwp_desc *desc)
{
#if JWP_TX_NOTIFY_ENABLE
	if (desc->send_pendding)
	{
		jwp_cond_timedwait(desc->tx_cond, desc->lock, JWP_SEND_TIMEOUT);

		return desc->send_pendding == false;
	}

	return true;
#else
	jwp_u32 count;

	for (count = JWP_SEND_TIMEOUT; count; count--)
	{
		if (desc->send_pendding == false)
		{
			return true;
		}

		jwp_msleep(1);
	}

	return false;
#endif
}

#if JWP_TX_DATA_LOOP_ENABLE
void jwp_tx_data_loop(struct jwp_desc *desc)
{
	struct jwp_package pkg;
	struct jwp_header *hdr = &pkg.header;
	struct jwp_queue *queue = jwp_get_queue(desc, JWP_QUEUE_TX_DATA);

	while (1)
	{
		hdr->length = jwp_queue_dequeue_peek(queue, hdr->payload, JWP_MAX_PAYLOAD);
		if (hdr->length == 0)
		{
			jwp_queue_wait_data(queue);
			continue;
		}

		hdr->type = JWP_PKG_DATA;

#if JWP_TX_QUEUE_ENABLE == 0
		hdr->index = desc->tx_index + 1;
#endif

		if (jwp_send_package(desc, hdr, true))
		{
			jwp_queue_dequeue_commit(queue);
		}
	}
}
#endif

#if JWP_TX_LOOP_ENABLE
void jwp_tx_loop(struct jwp_desc *desc)
{
	struct jwp_header *hdr = &desc->tx_pkg.header;
	struct jwp_queue *queue = jwp_get_queue(desc, JWP_QUEUE_TX);

	while (1)
	{
		if (jwp_queue_dequeue_package(queue, hdr) == false)
		{
			jwp_queue_wait_data(queue);
			continue;
		}

		hdr->index = desc->tx_index + 1;

		while (1)
		{
			desc->send_pendding = true;
			jwp_hw_write_package(desc, hdr);

			if (jwp_wait_tx_complete(desc))
			{
				break;
			}
		}
	}
}
#endif

#if JWP_RX_LOOP_ENABLE
void jwp_rx_loop(struct jwp_desc *desc)
{
	struct jwp_rx_package *pkg = &desc->rx_pkg;
	struct jwp_queue *queue = jwp_get_queue(desc, JWP_QUEUE_RX);

	while (1)
	{
		if (jwp_queue_fill_package(queue, pkg))
		{
			jwp_process_rx_package(desc, &pkg->header);
		}
		else
		{
			jwp_queue_wait_data(queue);
		}
	}
}
#endif
