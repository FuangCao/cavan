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

void jwp_header_dump(const struct jwp_header *hdr)
{
	jwp_println("index = %d, type = %d, length = %d", hdr->index, hdr->type, hdr->length);
}

void jwp_package_dump(const struct jwp_package *pkg)
{
	jwp_header_dump(&pkg->header);
}

void jwp_package_init(struct jwp_package *pkg, struct jwp_desc *desc)
{
	pkg->desc = desc;
	pkg->head = pkg->body;
	pkg->header_remain = sizeof(pkg->header);
}

static u8 *jwp_find_package_start(const u8 *buff, jwp_size_t size)
{
	const u8 *buff_end;

	for (buff_end = buff + size - 1; buff < buff_end; buff++)
	{
		if (buff[0] == JWP_MAGIC_LOW && buff[1] == JWP_MAGIC_HIGH)
		{
			return (u8 *) buff;
		}
	}

	return NULL;
}

jwp_size_t jwp_package_fill(struct jwp_package *pkg, const u8 *buff, jwp_size_t size)
{
	const u8 *buff_bak = buff;

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
				const u8 *p;

				p = jwp_find_package_start(buff, size);
				if (p == NULL)
				{
					if (buff[size - 1] == JWP_MAGIC_LOW)
					{
						pkg->head = pkg->body + 1;
						pkg->header_remain = sizeof(pkg->header) - 1;
					}
					else
					{
						pkg->head = pkg->body;
						pkg->header_remain = sizeof(pkg->header);
					}

					return size;
				}

				buff = p + JWP_MAGIC_SIZE;
				size -= (buff - buff_bak);
			}

			pkg->head = pkg->body + JWP_MAGIC_SIZE;
			pkg->header_remain = sizeof(pkg->header) - JWP_MAGIC_SIZE;
		}

		if (size < pkg->header_remain)
		{
			memcpy(pkg->head, buff, size);
			pkg->head += size;
			pkg->header_remain -= size;
			return (buff - buff_bak) + size;
		}
		else
		{
			memcpy(pkg->head, buff, pkg->header_remain);
			buff += pkg->header_remain;
			size -= pkg->header_remain;
			pkg->header_remain = 0;
			pkg->data_remain = pkg->header.length;
			pkg->head = pkg->header.payload;
		}
	}

	if (size < pkg->data_remain)
	{
		memcpy(pkg->head, buff, size);
		pkg->head += size;
		pkg->data_remain -= size;
		return (buff - buff_bak) + size;
	}
	else
	{
		struct jwp_desc *desc = pkg->desc;

		memcpy(pkg->head, buff, pkg->data_remain);
		desc->package_received(desc, pkg);

		pkg->head = pkg->body;
		pkg->header_remain = sizeof(pkg->header);

		return (buff - buff_bak) + pkg->data_remain;
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

// ============================================================

// ============================================================

// ============================================================
