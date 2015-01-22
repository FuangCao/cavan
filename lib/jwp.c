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

jwp_bool jwp_package_fill(struct jwp_package *pkg, struct jwp_data_queue *queue)
{
	jwp_size_t rdLen;

	if (pkg->header_remain)
	{
		if (pkg->head == pkg->body)
		{
			while (1)
			{
				rdLen = jwp_data_queue_peek(queue, pkg->head, JWP_MAGIC_SIZE);
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

			jwp_data_queue_commit(queue);

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

// ============================================================

void jwp_data_queue_init(struct jwp_data_queue *queue)
{
	queue->head = queue->tail = queue->buff;
	queue->peek = queue->head;
	queue->last = queue->head + sizeof(queue->buff) - 1;
}

jwp_size_t jwp_data_queue_inqueue(struct jwp_data_queue *queue, const u8 *buff, jwp_size_t size)
{
	jwp_size_t length;

	if (queue->head < queue->tail)
	{
		length = queue->tail - queue->head - 1;
		if (size > length)
		{
			size = length;
		}
	}
	else
	{
		length = queue->last - queue->head;
		if (size > length)
		{
			if (queue->tail > queue->buff)
			{
				jwp_size_t left;

				memcpy(queue->head, buff, ++length);
				size -= length;

				left = queue->tail - queue->buff - 1;
				if (size < left)
				{
					left = size;
				}

				memcpy(queue->buff, buff + length, left);
				queue->head = queue->buff + left;

				return length + left;
			}

			size = length;
		}
	}

	memcpy(queue->head, buff, size);
	queue->head += size;

	return size;
}

jwp_size_t jwp_data_queue_peek(struct jwp_data_queue *queue, u8 *buff, jwp_size_t size)
{
	jwp_size_t length;

	if (queue->tail > queue->head)
	{
		length = queue->last - queue->tail + 1;
		if (length > size)
		{
			length = size;
		}
		else
		{
			jwp_size_t left;

			memcpy(buff, queue->tail, length);

			size -= length;
			left = queue->head - queue->buff;
			if (size < left)
			{
				left = size;
			}

			memcpy(buff + length, queue->buff, left);
			queue->peek = queue->buff + left;

			return length + left;
		}
	}
	else
	{
		length = queue->head - queue->tail;
		if (length > size)
		{
			length = size;
		}
	}

	memcpy(buff, queue->tail, length);
	queue->peek = queue->tail + length;

	return length;
}

void jwp_data_queue_commit(struct jwp_data_queue *queue)
{
	queue->tail = queue->peek;
}

jwp_size_t jwp_data_queue_dequeue(struct jwp_data_queue *queue, u8 *buff, jwp_size_t size)
{
	size = jwp_data_queue_peek(queue, buff, size);
	jwp_data_queue_commit(queue);

	return size;
}

jwp_size_t jwp_data_queue_skip(struct jwp_data_queue *queue, jwp_size_t size)
{
	jwp_size_t length;

	if (queue->tail > queue->head)
	{
		length = queue->last - queue->tail + 1;
		if (length > size)
		{
			length = size;
		}
		else
		{
			jwp_size_t left;

			size -= length;
			left = queue->head - queue->buff;
			if (size < left)
			{
				left = size;
			}

			queue->tail = queue->buff + left;

			return length + left;
		}
	}
	else
	{
		length = queue->head - queue->tail;
		if (length > size)
		{
			length = size;
		}
	}

	queue->tail = queue->tail + length;

	return length;
}

jwp_size_t jwp_data_queue_get_free_size(struct jwp_data_queue *queue)
{
	if (queue->head < queue->tail)
	{
		return queue->tail - queue->head - 1;
	}

	return (queue->last - queue->head) + (queue->tail - queue->buff);
}

jwp_size_t jwp_data_queue_get_fill_size(struct jwp_data_queue *queue)
{
	if (queue->tail > queue->head)
	{
		return (queue->last - queue->tail) + (queue->head - queue->buff);
	}

	return queue->head - queue->tail;
}

// ============================================================

jwp_bool jwp_init(struct jwp_desc *desc, void *data)
{
	if (desc->hw_read == NULL || desc->hw_write == NULL)
	{
		return false;
	}

	if (desc->data_received == NULL || desc->package_received == NULL)
	{
		return false;
	}

#if JWP_USE_TIMER
	if (desc->create_timer == NULL || desc->delete_timer == NULL)
	{
		return false;
	}

	desc->send_timer = JWP_TIMER_INVALID;
#endif

	desc->private_data = data;

	desc->send_index = 0;
	desc->recv_index = 0;

	jwp_package_init(&desc->pkg_send);
	jwp_package_init(&desc->pkg_recv);

	jwp_data_queue_init(&desc->send_queue);
	jwp_data_queue_init(&desc->recv_queue);

	return true;
}

void jwp_send_queue_flush(struct jwp_desc *desc)
{
	while (1)
	{
		jwp_size_t rdLen;
		jwp_u8 buff[JWP_MTU];
		const jwp_u8 *p, *p_end;

		rdLen = jwp_data_queue_peek(&desc->send_queue, buff, sizeof(buff));
		if (rdLen == 0)
		{
			break;
		}

		for (p = buff, p_end = p + rdLen; p < p_end; p += desc->hw_write(desc, p, p_end - p));

		jwp_data_queue_commit(&desc->send_queue);
	}
}

static void jwp_send_data_real(struct jwp_desc *desc, const void *data, jwp_size_t size)
{
	const u8 *p = data, *p_end = p + size;

	while (p < p_end)
	{
		size = jwp_data_queue_inqueue(&desc->send_queue, p, p_end - p);
		if (size)
		{
			p += size;
		}
		else
		{
			jwp_send_queue_flush(desc);
		}
	}
}

static void jwp_send_package_real(struct jwp_desc *desc, struct jwp_header *hdr, const void *data)
{
	hdr->magic_high = JWP_MAGIC_HIGH;
	hdr->magic_low = JWP_MAGIC_LOW;

	jwp_send_data_real(desc, (u8 *) hdr, sizeof(struct jwp_header));

	if (data && hdr->length > 0)
	{
		jwp_send_data_real(desc, data, hdr->length);
	}

	jwp_send_queue_flush(desc);
}

void jwp_send_package(struct jwp_desc *desc, struct jwp_header *hdr, jwp_u8 type, const void *data, jwp_size_t size)
{
	hdr->type = type;
	hdr->length = size;
	jwp_send_package_real(desc, hdr, data);
}

void jwp_send_data_ack(struct jwp_desc *desc, jwp_u8 index)
{
	struct jwp_header hdr;

	hdr.index = index;

	jwp_send_package(desc, &hdr, JWP_PKG_DATA_ACK, NULL, 0);
}

#if JWP_USE_TIMER
static void jwp_send_timeout_handler(struct jwp_desc *desc, jwp_timer timer)
{
	struct jwp_header *hdr = &desc->pkg_send.header;

	jwp_send_package_real(desc, hdr, hdr->payload);
	desc->send_timer = desc->create_timer(desc, timer, JWP_SEND_TIEMOUT, jwp_send_timeout_handler);
}
#endif

jwp_bool jwp_send_package_sync(struct jwp_desc *desc, jwp_u8 type, const void *data, jwp_size_t size)
{
	struct jwp_package *pkg = &desc->pkg_send;
	struct jwp_header *hdr;

	if (pkg->in_use)
	{
		return false;
	}

	pkg->in_use = true;

	hdr = &pkg->header;
	hdr->type = type;
	hdr->length = size;
	hdr->index = desc->send_index + 1;
	memcpy(hdr->payload, data, size);

#if JWP_USE_TIMER
	jwp_send_timeout_handler(desc, desc->send_timer);
#else
	while (1)
	{
		int i;

		jwp_send_package_real(desc, hdr, hdr->payload);

		for (i = 0; i < JWP_SEND_TIEMOUT; i++)
		{
			if (pkg->in_use == false)
			{
				return true;
			}

			msleep(1);
		}
	}
#endif

	return true;
}

static void jwp_process_package(struct jwp_desc *desc)
{
	struct jwp_package *pkg = &desc->pkg_recv;
	struct jwp_header *hdr = &pkg->header;

	while (jwp_package_fill(pkg, &desc->recv_queue))
	{
#if JWP_DEBUG
		jwp_header_dump(hdr);
#endif
		switch (hdr->type)
		{
		case JWP_PKG_DATA:
			jwp_send_data_ack(desc, hdr->index);
			if (hdr->index == desc->recv_index + 1)
			{
				desc->recv_index++;
				desc->data_received(desc, hdr->payload, hdr->length);
			}
#if JWP_DEBUG
			else
			{
				pr_red_info("throw data package %d, need %d", hdr->index, desc->recv_index + 1);
			}
#endif
			break;

		case JWP_PKG_DATA_ACK:
			if (hdr->index == desc->send_index + 1)
			{
#if JWP_USE_TIMER
				if (desc->send_timer != JWP_TIMER_INVALID)
				{
					desc->delete_timer(desc, desc->send_timer);
				}
#endif
				desc->send_index++;
				desc->pkg_send.in_use = false;
			}
#if JWP_DEBUG
			else
			{
				pr_red_info("throw ack package %d, need %d", hdr->index, desc->send_index + 1);
			}
#endif
			break;

		default:
			desc->package_received(desc, pkg);
		}
	}
}

void jwp_write_data(struct jwp_desc *desc, const void *buff, jwp_size_t size)
{
	const u8 *p = buff, *p_end = p + size;

	while (p < p_end)
	{
		jwp_size_t wrLen;

		wrLen = jwp_data_queue_inqueue(&desc->recv_queue, p, p_end - p);
		if (wrLen)
		{
			p += wrLen;
		}
		else
		{
			jwp_process_package(desc);
		}
	}

	jwp_process_package(desc);
}

void jwp_process_rx_data(struct jwp_desc *desc)
{
	jwp_size_t rdLen;
	jwp_u8 buff[JWP_MTU];

	while (1)
	{
		rdLen = desc->hw_read(desc, buff, sizeof(buff));
		if (rdLen == 0)
		{
			break;
		}

		jwp_write_data(desc, buff, rdLen);
	}
}
