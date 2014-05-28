/*
 * File:		sha.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2014-05-28 17:50:42
 *
 * Copyright (c) 2014 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/sha.h>

char *cavan_shasum_tostring(const u8 *digest, size_t size, char *buff, size_t buff_size)
{
	const u8 *digest_end;
	char *buff_bak = buff;
	char *buff_end = buff + buff_size;

	for (digest_end = digest + size; digest < digest_end; digest++)
	{
		buff = simple_value2text_unsigned(*digest, buff, buff_end - buff, 16);
	}

	return buff_bak;
}

static void cavan_sha1_transform(struct cavan_sha1_context *context, const u8 *buff)
{
	int i;
	u32 W[80];
	u32 A, B, C, D, E;

	for (i = 0; i < 16; i++, buff += 4)
	{
		W[i] = BYTES_DWORD(buff[0], buff[1], buff[2], buff[3]);
	}

	for(; i < 80; i++)
	{
		u32 temp = W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16];
		W[i] = ROL(temp, 1);
	}

	A = context->state[0];
	B = context->state[1];
	C = context->state[2];
	D = context->state[3];
	E = context->state[4];

	for(i = 0; i < 80; i++)
	{
		u32 temp = ROL(A, 5) + E + W[i];

		if (i < 20)
		{
			temp += (D ^ (B & (C ^ D))) + 0x5A827999;
		}
		else if ( i < 40)
		{
			temp += (B ^ C ^ D) + 0x6ED9EBA1;
		}
		else if ( i < 60)
		{
			temp += ((B & C)|(D & (B | C))) + 0x8F1BBCDC;
		}
		else
		{
			temp += (B ^ C ^ D) + 0xCA62C1D6;
		}

		E = D;
		D = C;
		C = ROL(B, 30);
		B = A;
		A = temp;
	}

	context->state[0] += A;
	context->state[1] += B;
	context->state[2] += C;
	context->state[3] += D;
	context->state[4] += E;
}

static void cavan_sha1_update(struct cavan_sha1_context *context, const void *buff, size_t size)
{
	if (context->remain > 0)
	{
		size_t padding = sizeof(context->buff) - context->remain;

		if (padding <= size)
		{
			mem_copy(context->buff + context->remain, buff, padding);

			cavan_sha1_transform(context, context->buff);
			buff = ADDR_ADD(buff, padding);
			size -= padding;
			context->remain = 0;
		}
	}

	while (size >= sizeof(context->buff))
	{
		cavan_sha1_transform(context, buff);
		buff = ADDR_ADD(buff, sizeof(context->buff));
		size -= sizeof(context->buff);
	}

	if (size)
	{
		mem_copy(context->buff + context->remain, buff, size);
		context->remain += size;
	}
}

static u8 *cavan_sha1_finish(struct cavan_sha1_context *context, u8 *digest)
{
	int i;
	const u8 *p, *p_end;
	u8 *digest_bak = digest;
	size_t count = context->remain << 3;

	cavan_sha1_update(context, "\x80", 1);

	while (context->remain != sizeof(context->buff) - 8)
	{
		cavan_sha1_update(context, "\0", 1);
	}

	for (i = 0; i < 8; i++)
	{
		u8 temp = count >> ((7 - i) << 3);
		cavan_sha1_update(context, &temp, 1);
	}

	for (p = (u8 *)context->state, p_end = p + sizeof(context->state); p < p_end; p += 4, digest += 4)
	{
		digest[0] = p[3];
		digest[1] = p[2];
		digest[2] = p[1];
		digest[3] = p[0];
	}

	return digest_bak;
}

static void cavan_sha1_init(struct cavan_sha1_context *context)
{
	context->remain = 0;
	context->state[0] = 0x67452301;
	context->state[1] = 0xEFCDAB89;
	context->state[2] = 0x98BADCFE;
	context->state[3] = 0x10325476;
	context->state[4] = 0xC3D2E1F0;
}

u8 *cavan_sha1sum(const void *buff, size_t size, u8 *digest)
{
	struct cavan_sha1_context context;

	cavan_sha1_init(&context);
	cavan_sha1_update(&context, buff, size);

	return cavan_sha1_finish(&context, digest);
}

u8 *cavan_file_sha1sum(const char *pathname, u8 *digest)
{
	int fd;
	void *addr;
	size_t size;

	fd = file_mmap(pathname, &addr, &size, O_RDONLY);
	if (fd < 0)
	{
		pr_red_info("file_mmap");
		return NULL;
	}

	digest = cavan_sha1sum(addr, size, digest);
	file_unmap(fd, addr, size);

	return digest;
}
