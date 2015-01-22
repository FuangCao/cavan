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

static u8 *jwp_find_package_start(const u8 *buff, u8 length)
{
	const u8 *buff_end;

	for (buff_end = buff + length - 1; buff < buff_end; buff++)
	{
		if (buff[0] == JWP_MAGIC_LOW && buff[1] == JWP_MAGIC_HIGH)
		{
			return (u8 *) buff;
		}
	}

	return NULL;
}

u8 jwp_package_fill(struct jwp_package *pkg, const u8 *buff, u8 length)
{
	const u8 *buff_bak = buff;

	if (pkg->header_remain)
	{
		if (pkg->head < pkg->body + JWP_MAGIC_SIZE)
		{
			if (length == 0)
			{
				return 0;
			}

			if (pkg->head > pkg->body && buff[0] == JWP_MAGIC_HIGH)
			{

				buff++;
				length--;
			}
			else
			{
				const u8 *p;

				p = jwp_find_package_start(buff, length);
				if (p == NULL)
				{
					if (buff[length - 1] == JWP_MAGIC_LOW)
					{
						pkg->head = pkg->body + 1;
						pkg->header_remain = sizeof(pkg->header) - 1;
					}
					else
					{
						pkg->head = pkg->body;
						pkg->header_remain = sizeof(pkg->header);
					}

					return length;
				}

				buff = p + JWP_MAGIC_SIZE;
				length -= (buff - buff_bak);
			}

			pkg->head = pkg->body + JWP_MAGIC_SIZE;
			pkg->header_remain = sizeof(pkg->header) - JWP_MAGIC_SIZE;
		}

		if (length < pkg->header_remain)
		{
			memcpy(pkg->head, buff, length);
			pkg->head += length;
			pkg->header_remain -= length;
			return (buff - buff_bak) + length;
		}
		else
		{
			memcpy(pkg->head, buff, pkg->header_remain);
			buff += pkg->header_remain;
			length -= pkg->header_remain;
			pkg->header_remain = 0;
			pkg->data_remain = pkg->header.length;
			pkg->head = pkg->header.payload;
		}
	}

	if (length < pkg->data_remain)
	{
		memcpy(pkg->head, buff, length);
		pkg->head += length;
		pkg->data_remain -= length;
		return (buff - buff_bak) + length;
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

// ============================================================

// ============================================================

// ============================================================

// ============================================================
