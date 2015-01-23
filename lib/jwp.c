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

#define JWP_DEBUG	1

void jwp_header_dump(const struct jwp_header *hdr)
{
	jwp_println("index = %d, type = %d, length = %d", hdr->index, hdr->type, hdr->length);
}

void jwp_package_dump(const struct jwp_package *pkg)
{
	jwp_header_dump(&pkg->header);
}

void jwp_package_init(struct jwp_package *pkg)
{
	pkg->head = pkg->body;
	pkg->header_remain = sizeof(pkg->header);
}

// ============================================================

void jwp_data_queue_init(struct jwp_data_queue *queue)
{
	queue->tail = queue->head = queue->buff;
	queue->tail_peek = queue->head_peek = queue->tail;
	queue->last = queue->tail + sizeof(queue->buff) - 1;
}

jwp_size_t jwp_data_queue_inqueue_peek(struct jwp_data_queue *queue, const u8 *buff, jwp_size_t size)
{
	jwp_size_t length;

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

				memcpy(queue->tail, buff, ++length);
				size -= length;

				left = queue->head - queue->buff - 1;
				if (size < left)
				{
					left = size;
				}

				memcpy(queue->buff, buff + length, left);
				queue->tail_peek = queue->buff + left;

				return length + left;
			}

			size = length;
		}
	}

	memcpy(queue->tail, buff, size);
	queue->tail_peek = queue->tail + size;

	return size;
}

void jwp_data_queue_inqueue_commit(struct jwp_data_queue *queue)
{
	queue->tail = queue->tail_peek;
}

jwp_size_t jwp_data_queue_inqueue(struct jwp_data_queue *queue, const u8 *buff, jwp_size_t size)
{
	size = jwp_data_queue_inqueue_peek(queue, buff, size);
	jwp_data_queue_inqueue_commit(queue);

	return size;
}

jwp_bool jwp_data_queue_inqueue_full(struct jwp_data_queue *queue, const u8 *buff, jwp_size_t size)
{
	jwp_size_t wrLen;

	wrLen = jwp_data_queue_inqueue_peek(queue, buff, size);
	if (wrLen < size)
	{
		return false;
	}

	jwp_data_queue_inqueue_commit(queue);

	return true;
}

jwp_size_t jwp_data_queue_dequeue_peek(struct jwp_data_queue *queue, u8 *buff, jwp_size_t size)
{
	jwp_size_t length;

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

			memcpy(buff, queue->head, length);

			size -= length;
			left = queue->tail - queue->buff;
			if (size < left)
			{
				left = size;
			}

			memcpy(buff + length, queue->buff, left);
			queue->head_peek = queue->buff + left;

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

	memcpy(buff, queue->head, length);
	queue->head_peek = queue->head + length;

	return length;
}

void jwp_data_queue_dequeue_commit(struct jwp_data_queue *queue)
{
	queue->head = queue->head_peek;
}

jwp_size_t jwp_data_queue_dequeue(struct jwp_data_queue *queue, u8 *buff, jwp_size_t size)
{
	size = jwp_data_queue_dequeue_peek(queue, buff, size);
	jwp_data_queue_dequeue_commit(queue);

	return size;
}

jwp_size_t jwp_data_queue_skip(struct jwp_data_queue *queue, jwp_size_t size)
{
	jwp_size_t length;

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

	return length;
}

jwp_bool jwp_data_queue_dequeue_package(struct jwp_data_queue *queue, struct jwp_header *hdr)
{
	jwp_size_t rdLen;

	rdLen = jwp_data_queue_dequeue(queue, (u8 *) hdr, JWP_HEADER_SIZE);
	if (rdLen < JWP_HEADER_SIZE)
	{
		return false;
	}

	rdLen = jwp_data_queue_dequeue(queue, hdr->payload, hdr->length);
	if (rdLen < hdr->length)
	{
		return false;
	}

	return true;
}

jwp_bool jwp_data_queue_fill_package(struct jwp_data_queue *queue, struct jwp_package *pkg)
{
	jwp_size_t rdLen;

	if (pkg->header_remain)
	{
		if (pkg->head == pkg->body)
		{
			while (1)
			{
				rdLen = jwp_data_queue_dequeue_peek(queue, pkg->head, JWP_MAGIC_SIZE);
				if (rdLen < JWP_MAGIC_SIZE)
				{
					return false;
				}

				if (pkg->head[0] == JWP_MAGIC_LOW && pkg->head[1] == JWP_MAGIC_HIGH)
				{
					break;
				}

				jwp_data_queue_skip(queue, 1);
			}

			jwp_data_queue_dequeue_commit(queue);

			pkg->head = pkg->body + JWP_MAGIC_SIZE;
			pkg->header_remain = sizeof(pkg->header) - JWP_MAGIC_SIZE;
		}

		rdLen = jwp_data_queue_dequeue(queue, pkg->head, pkg->header_remain);
		if (rdLen < pkg->header_remain)
		{
			pkg->head += rdLen;
			pkg->header_remain -= rdLen;
			return false;
		}
		else
		{
			pkg->header_remain = 0;
			pkg->data_remain = pkg->header.length;
			pkg->head = pkg->header.payload;
		}
	}

	rdLen = jwp_data_queue_dequeue(queue, pkg->head, pkg->data_remain);
	if (rdLen < pkg->data_remain)
	{
		pkg->head += rdLen;
		pkg->data_remain -= rdLen;
		return false;
	}
	else
	{
		pkg->head = pkg->body;
		pkg->header_remain = sizeof(pkg->header);

		return true;
	}
}

jwp_size_t jwp_data_queue_get_free_size(struct jwp_data_queue *queue)
{
	if (queue->tail < queue->head)
	{
		return queue->head - queue->tail - 1;
	}

	return (queue->last - queue->tail) + (queue->head - queue->buff);
}

jwp_size_t jwp_data_queue_get_fill_size(struct jwp_data_queue *queue)
{
	if (queue->head > queue->tail)
	{
		return (queue->last - queue->head) + (queue->tail - queue->buff);
	}

	return queue->tail - queue->head;
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

#if JWP_USE_TIMER
	if (desc->create_timer == NULL || desc->delete_timer == NULL)
	{
		jwp_println("desc->create_timer == NULL || desc->delete_timer == NULL");
		return false;
	}
	desc->tx_timer = JWP_TIMER_INVALID;
#endif
#endif

#if JWP_USE_TIMER
	desc->tx_timer = JWP_TIMER_INVALID;
#endif

	desc->private_data = data;

	desc->send_index = 0;
	desc->recv_index = 0;

#if JWP_USE_TX_QUEUE || JWP_USE_TIMER
	jwp_package_init(&desc->tx_pkg);
#endif

	jwp_package_init(&desc->rx_pkg);

	for (i = 0; i < JWP_QUEUE_COUNT; i++)
	{
		jwp_data_queue_init(desc->queues + i);
	}

	return true;
}

static void jwp_process_rx_package(struct jwp_desc *desc, struct jwp_package *pkg)
{
	struct jwp_header *hdr = &pkg->header;

#if JWP_DEBUG
	jwp_header_dump(hdr);
#endif

	switch (hdr->type)
	{
	case JWP_PKG_DATA:
		jwp_send_data_ack(desc, hdr->index);
		if (hdr->index == (jwp_u8) (desc->recv_index + 1))
		{
			struct jwp_data_queue *queue = jwp_data_queue_get(desc, JWP_QUEUE_RECV_DATA);

			jwp_data_queue_inqueue(queue, hdr->payload, hdr->length);
			desc->recv_index++;
			desc->data_received(desc);
		}
#if JWP_DEBUG
		else
		{
			jwp_pr_red_info("throw data package %d, need %d", hdr->index, desc->recv_index + 1);
		}
#endif
		break;

	case JWP_PKG_DATA_ACK:
		if (hdr->index == (jwp_u8) (desc->send_index + 1))
		{
#if JWP_USE_TX_QUEUE
			struct jwp_data_queue *queue = jwp_data_queue_get(desc, JWP_QUEUE_SEND);
#endif

#if JWP_USE_TIMER
			if (desc->tx_timer != JWP_TIMER_INVALID)
			{
				desc->delete_timer(desc, desc->tx_timer);
			}
#endif

			desc->send_index++;
			desc->send_pendding = false;
#if JWP_USE_TX_QUEUE
			jwp_data_queue_dequeue_commit(queue);
#endif
			desc->send_complete(desc);
		}
#if JWP_DEBUG
		else
		{
			jwp_pr_red_info("throw ack package %d, need %d", hdr->index, desc->send_index + 1);
		}
#endif
		break;

	default:
		desc->package_received(desc, pkg);
	}
}

jwp_bool jwp_process_rx_data(struct jwp_desc *desc)
{
	struct jwp_package *pkg = &desc->rx_pkg;
	struct jwp_data_queue *queue = jwp_data_queue_get(desc, JWP_QUEUE_RECV);

	while (jwp_data_queue_fill_package(queue, pkg))
	{
		jwp_process_rx_package(desc, pkg);
	}

	return true;
}

jwp_size_t jwp_write_rx_data(struct jwp_desc *desc, const void *buff, jwp_size_t size)
{
	struct jwp_data_queue *queue = jwp_data_queue_get(desc, JWP_QUEUE_RECV);

	size = jwp_data_queue_inqueue(queue, buff, size);
	jwp_process_rx_data(desc);

	return size;
}

static void jwp_hw_write(struct jwp_desc *desc, const u8 *buff, size_t size)
{
	const u8 *buff_end = buff + size;

	while (buff < buff_end)
	{
		jwp_size_t wrLen;

		wrLen = desc->hw_write(desc, buff, buff_end - buff);
		buff += wrLen;
	}
}

static inline void jwp_hw_write_package(struct jwp_desc *desc, const struct jwp_header *hdr)
{
	jwp_hw_write(desc, (u8 *) hdr, JWP_HEADER_SIZE + hdr->length);
}

#if JWP_USE_TIMER
static void jwp_send_timeout_handler(struct jwp_desc *desc, jwp_timer timer)
{
	struct jwp_header *hdr = &desc->tx_pkg.header;

	jwp_hw_write_package(desc, hdr);
	desc->tx_timer = desc->create_timer(desc, timer, JWP_SEND_TIMEOUT, jwp_send_timeout_handler);
}
#else
static jwp_bool jwp_send_and_wait_ack(struct jwp_desc *desc, const struct jwp_header *hdr)
{
#if JWP_USE_TX_QUEUE
	while (1)
#else
	jwp_u16 retry;

	for (retry = JWP_SEND_RETRY; retry; retry--)
#endif
	{
		jwp_u16 count;

		jwp_hw_write_package(desc, hdr);

		for (count = JWP_SEND_TIMEOUT; count; count--)
		{
			if (desc->send_pendding == false)
			{
				return true;
			}

			msleep(1);
		}
	}

#if JWP_DEBUG
	jwp_pr_red_info("send package timeout");
#endif

	return false;
}
#endif

#if JWP_USE_TX_QUEUE
jwp_bool jwp_process_tx_data(struct jwp_desc *desc)
{
	struct jwp_header *hdr;
	struct jwp_data_queue *queue;

	if (desc->send_pendding)
	{
		return false;
	}

	hdr = &desc->tx_pkg.header;
	queue = jwp_data_queue_get(desc, JWP_QUEUE_SEND);

	if (jwp_data_queue_dequeue_package(queue, hdr) == false)
	{
		return false;
	}

	desc->send_pendding = true;
	hdr->index = desc->send_index + 1;

#if JWP_USE_TIMER
	jwp_send_timeout_handler(desc, desc->tx_timer);
#else
	jwp_send_and_wait_ack(desc, hdr);
#endif

	return true;
}
#endif

jwp_bool jwp_send_package(struct jwp_desc *desc, struct jwp_header *hdr, bool sync)
{
	hdr->magic_high = JWP_MAGIC_HIGH;
	hdr->magic_low = JWP_MAGIC_LOW;

	if (sync)
	{
#if JWP_USE_TX_QUEUE
		struct jwp_data_queue *queue = jwp_data_queue_get(desc, JWP_QUEUE_SEND);

		return jwp_data_queue_inqueue_package(queue, hdr);
#else
#if JWP_USE_TIMER == 0
		jwp_bool result;
#endif
		if (desc->send_pendding)
		{
			return false;
		}

		desc->send_pendding = true;
#if JWP_USE_TIMER
		memcpy(&desc->tx_pkg, hdr, JWP_HEADER_SIZE + hdr->length);
		jwp_send_timeout_handler(desc, desc->tx_timer);
		return true;
#else
		result = jwp_send_and_wait_ack(desc, hdr);
		desc->send_pendding = false;
		return result;
#endif
#endif
	}
	else
	{
		jwp_hw_write_package(desc, hdr);

		return true;
	}
}

void jwp_send_data_ack(struct jwp_desc *desc, jwp_u8 index)
{
	struct jwp_header hdr;

	hdr.type = JWP_PKG_DATA_ACK;
	hdr.index = index;
	hdr.length = 0;

	jwp_send_package(desc, &hdr, false);
}

jwp_size_t jwp_send_data(struct jwp_desc *desc, const void *buff, jwp_size_t size)
{
	struct jwp_package pkg;
	struct jwp_header *hdr = &pkg.header;
	const u8 *p = buff, *p_end = p + size;

	hdr->type = JWP_PKG_DATA;

	while (p < p_end)
	{
		size = p_end - p;
#if JWP_USE_TX_QUEUE == 0
		hdr->index = desc->send_index + 1;
#endif
		hdr->length = size > JWP_MAX_PAYLOAD ? JWP_MAX_PAYLOAD : size;
		memcpy(hdr->payload, buff, hdr->length);

		if (jwp_send_package(desc, hdr, true) == false)
		{
			break;
		}

		p += hdr->length;
	}

	return p - (const u8 *) buff;
}

jwp_size_t jwp_recv_data(struct jwp_desc *desc, void *buff, jwp_size_t size)
{
	struct jwp_data_queue *queue = jwp_data_queue_get(desc, JWP_QUEUE_RECV_DATA);

	return jwp_data_queue_dequeue(queue, buff, size);
}
