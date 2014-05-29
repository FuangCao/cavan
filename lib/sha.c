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
#include <cavan/math.h>

#define SHA1_FUNC1_BASE(A, E, V) \
	(ROL(A, 5) + (V))

#define SHA1_FUNC2_BASE(B, E, V) \
	do \
	{ \
		(E) += (V); \
		(B) = ROR(B, 2); \
	} while (0)

#define SHA1_FUNC11(A, B, C, D, E, V) \
	SHA1_FUNC1_BASE(A, E, (V) + (((D) ^ ((B) & ((C) ^ (D)))) + 0x5A827999))

#define SHA1_FUNC21(A, B, C, D, E, V) \
	SHA1_FUNC2_BASE(B, E, SHA1_FUNC11(A, B, C, D, E, V))

#define SHA1_FUNC12(A, B, C, D, E, V) \
	SHA1_FUNC1_BASE(A, E, (V) + (((B) ^ (C) ^ (D)) + 0x6ED9EBA1))

#define SHA1_FUNC22(A, B, C, D, E, V) \
	SHA1_FUNC2_BASE(B, E, SHA1_FUNC12(A, B, C, D, E, V))

#define SHA1_FUNC13(A, B, C, D, E, V) \
	SHA1_FUNC1_BASE(A, E, (V) + ((((B) & (C)) | ((D) & ((B) | (C)))) + 0x8F1BBCDC))

#define SHA1_FUNC23(A, B, C, D, E, V) \
	SHA1_FUNC2_BASE(B, E, SHA1_FUNC13(A, B, C, D, E, V))

#define SHA1_FUNC14(A, B, C, D, E, V) \
	SHA1_FUNC1_BASE(A, E, (V) + (((B) ^ (C) ^ (D)) + 0xCA62C1D6))

#define SHA1_FUNC24(A, B, C, D, E, V) \
	SHA1_FUNC2_BASE(B, E, SHA1_FUNC14(A, B, C, D, E, V))

#define SHA1_TRANSFROM(A, B, C, D, E, W, F) \
	do { \
		register const u32 *p, *ep; \
		for (p = W, ep = p + 20; p < ep; p += 5) \
		{ \
			SHA1_FUNC2##F(A, B, C, D, E, p[0]); \
			SHA1_FUNC2##F(E, A, B, C, D, p[1]); \
			SHA1_FUNC2##F(D, E, A, B, C, p[2]); \
			SHA1_FUNC2##F(C, D, E, A, B, p[3]); \
			SHA1_FUNC2##F(B, C, D, E, A, p[4]); \
		} \
	} while (0)

#define MD5_FUNC1(B, C, D) \
	(((B) & (C)) | (~(B) & (D)))

#define MD5_FUNC2(B, C, D) \
	(((B) & (D)) | ((C) & ~(D)))

#define MD5_FUNC3(B, C, D) \
	((B) ^ (C) ^ (D))

#define MD5_FUNC4(B, C, D) \
	((C) ^ ((B) | ~(D)))

#define MD5_TRANSFORM(A, B, C, D, K, S, I, F) \
	do { \
		(A) = (B) + ROL((A) + MD5_FUNC##F(B, C, D) + (K) + (I), S); \
	} while (0)

static void cavan_sha1_transform(struct cavan_sha1_context *context, const u32 *buff)
{
	int i;
	u32 W[80];
	register u32 A, B, C, D, E;

	mem_swap32(W, buff, 16);

	for(i = 16; i < 80; i++)
	{
		W[i] = ROL(W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16], 1);
	}

	A = context->state[0];
	B = context->state[1];
	C = context->state[2];
	D = context->state[3];
	E = context->state[4];

	SHA1_TRANSFROM(A, B, C, D, E, W, 1);
	SHA1_TRANSFROM(A, B, C, D, E, W + 20, 2);
	SHA1_TRANSFROM(A, B, C, D, E, W + 40, 3);
	SHA1_TRANSFROM(A, B, C, D, E, W + 60, 4);

	context->state[0] += A;
	context->state[1] += B;
	context->state[2] += C;
	context->state[3] += D;
	context->state[4] += E;
}

static void cavan_sha1_update(struct cavan_sha1_context *context, const void *buff, size_t size)
{
	size_t remain;
	const void *buff_end = ADDR_ADD(buff, size);

	if (context->remain > 0)
	{
		size_t padding;

		padding = sizeof(context->buff) - context->remain;
		if (padding <= size)
		{
			mem_copy(context->buff + context->remain, buff, padding);
			cavan_sha1_transform(context, (u32 *) context->buff);
			buff = ADDR_ADD(buff, padding);
			context->remain = 0;
		}
	}

	while (1)
	{
		remain = ADDR_SUB2(buff_end, buff);
		if (remain < sizeof(context->buff))
		{
			break;
		}

		cavan_sha1_transform(context, buff);
		buff = ADDR_ADD(buff, sizeof(context->buff));
	}

	if (remain)
	{
		mem_copy(context->buff + context->remain, buff, remain);
		context->remain += remain;
	}

	context->count += size;
}

static void cavan_sha1_finish(struct cavan_sha1_context *context, u8 digest[SHA1_DIGEST_SIZE])
{
	u8 *p, *ps;
	const u8 *q;
	u64 bits = context->count << 3;

	cavan_sha1_update(context, "\x80", 1);

	if (context->remain > sizeof(context->buff) - 8)
	{
		memset(context->buff + context->remain, 0, sizeof(context->buff) - context->remain);
		cavan_sha1_transform(context, (u32 *) context->buff);
		memset(context->buff, 0, context->remain);
	}
	else
	{
		memset(context->buff + context->remain, 0, sizeof(context->buff) - context->remain - 8);
	}

	for (p = context->buff + sizeof(context->buff) - 1, ps = p - 8, q = (u8 *) &bits; p > ps; p--, q++)
	{
		*p = *q;
	}

	cavan_sha1_transform(context, (u32 *) context->buff);
	mem_swap32((u32 *) digest, context->state, NELEM(context->state));
}

static void cavan_sha1_init(struct cavan_sha1_context *context)
{
	context->count = 0;
	context->remain = 0;
	context->state[0] = 0x67452301;
	context->state[1] = 0xEFCDAB89;
	context->state[2] = 0x98BADCFE;
	context->state[3] = 0x10325476;
	context->state[4] = 0xC3D2E1F0;
}

int cavan_sha1sum(const void *buff, size_t size, u8 digest[SHA1_DIGEST_SIZE])
{
	struct cavan_sha1_context context;

	cavan_sha1_init(&context);
	cavan_sha1_update(&context, buff, size);
	cavan_sha1_finish(&context, digest);

	return 0;
}

int cavan_file_sha1sum_mmap(const char *pathname, u8 digest[SHA1_DIGEST_SIZE])
{
	int fd;
	void *addr;
	size_t size;

	fd = file_mmap(pathname, &addr, &size, O_RDONLY);
	if (fd < 0)
	{
		// pr_red_info("file_mmap");
		return fd;
	}

	cavan_sha1sum(addr, size, digest);
	file_unmap(fd, addr, size);

	return 0;
}

int cavan_file_sha1sum(const char *pathname, u8 digest[SHA1_DIGEST_SIZE])
{
	int fd;
	int ret;
	struct cavan_sha1_context context;

	ret = cavan_file_sha1sum_mmap(pathname, digest);
	if (ret >= 0)
	{
		return ret;
	}

	fd = open(pathname, O_RDONLY);
	if (fd < 0)
	{
		pr_error_info("open file %s failed", pathname);
		return fd;
	}

	cavan_sha1_init(&context);

	while (1)
	{
		ssize_t rdlen;
		char buff[1024];

		rdlen = read(fd, buff, sizeof(buff));
		if (rdlen <= 0)
		{
			if (rdlen == 0)
			{
				break;
			}

			pr_error_info("read file %s", pathname);
			return rdlen;
		}

		cavan_sha1_update(&context, buff, rdlen);
	}

	cavan_sha1_finish(&context, digest);

	return 0;
}

// ========================================================

static void cavan_md5_init (struct cavan_md5_context *context)
{
	context->count = 0;
	context->remain = 0;
	context->state[0] = 0x67452301;
	context->state[1] = 0xEFCDAB89;
	context->state[2] = 0x98BADCFE;
	context->state[3] = 0x10325476;
}

static void cavan_md5_transform (struct cavan_md5_context *context, const u32 *buff)
{
	register u32 A, B, C, D;

	A = context->state[0];
	B = context->state[1];
	C = context->state[2];
	D = context->state[3];

	/* Round 1 */

	MD5_TRANSFORM(A, B, C, D, buff[0], 7, 0xD76AA478, 1);
	MD5_TRANSFORM(D, A, B, C, buff[1], 12, 0xE8C7B756, 1);
	MD5_TRANSFORM(C, D, A, B, buff[2], 17, 0x242070DB, 1);
	MD5_TRANSFORM(B, C, D, A, buff[3], 22, 0xC1BDCEEE, 1);

	MD5_TRANSFORM(A, B, C, D, buff[4], 7, 0xF57C0FAF, 1);
	MD5_TRANSFORM(D, A, B, C, buff[5], 12, 0x4787C62A, 1);
	MD5_TRANSFORM(C, D, A, B, buff[6], 17, 0xA8304613, 1);
	MD5_TRANSFORM(B, C, D, A, buff[7], 22, 0xFD469501, 1);

	MD5_TRANSFORM(A, B, C, D, buff[8], 7, 0x698098D8, 1);
	MD5_TRANSFORM(D, A, B, C, buff[9], 12, 0x8B44F7AF, 1);
	MD5_TRANSFORM(C, D, A, B, buff[10], 17, 0xFFFF5BB1, 1);
	MD5_TRANSFORM(B, C, D, A, buff[11], 22, 0x895CD7BE, 1);

	MD5_TRANSFORM(A, B, C, D, buff[12], 7, 0x6B901122, 1);
	MD5_TRANSFORM(D, A, B, C, buff[13], 12, 0xFD987193, 1);
	MD5_TRANSFORM(C, D, A, B, buff[14], 17, 0xA679438E, 1);
	MD5_TRANSFORM(B, C, D, A, buff[15], 22, 0x49B40821, 1);

	/* Round 2 */

	MD5_TRANSFORM(A, B, C, D, buff[1], 5, 0xF61E2562, 2);
	MD5_TRANSFORM(D, A, B, C, buff[6], 9, 0xC040B340, 2);
	MD5_TRANSFORM(C, D, A, B, buff[11], 14, 0x265E5A51, 2);
	MD5_TRANSFORM(B, C, D, A, buff[0], 20, 0xE9B6C7AA, 2);

	MD5_TRANSFORM(A, B, C, D, buff[5], 5, 0xD62F105D, 2);
	MD5_TRANSFORM(D, A, B, C, buff[10], 9, 0x2441453, 2);
	MD5_TRANSFORM(C, D, A, B, buff[15], 14, 0xD8A1E681, 2);
	MD5_TRANSFORM(B, C, D, A, buff[4], 20, 0xE7D3FBC8, 2);

	MD5_TRANSFORM(A, B, C, D, buff[9], 5, 0x21E1CDE6, 2);
	MD5_TRANSFORM(D, A, B, C, buff[14], 9, 0xC33707D6, 2);
	MD5_TRANSFORM(C, D, A, B, buff[3], 14, 0xF4D50D87, 2);
	MD5_TRANSFORM(B, C, D, A, buff[8], 20, 0x455A14ED, 2);

	MD5_TRANSFORM(A, B, C, D, buff[13], 5, 0xA9E3E905, 2);
	MD5_TRANSFORM(D, A, B, C, buff[2], 9, 0xFCEFA3F8, 2);
	MD5_TRANSFORM(C, D, A, B, buff[7], 14, 0x676F02D9, 2);
	MD5_TRANSFORM(B, C, D, A, buff[12], 20, 0x8D2A4C8A, 2);

	/* Round 3 */

	MD5_TRANSFORM(A, B, C, D, buff[5], 4, 0xFFFA3942, 3);
	MD5_TRANSFORM(D, A, B, C, buff[8], 11, 0x8771F681, 3);
	MD5_TRANSFORM(C, D, A, B, buff[11], 16, 0x6D9D6122, 3);
	MD5_TRANSFORM(B, C, D, A, buff[14], 23, 0xFDE5380C, 3);

	MD5_TRANSFORM(A, B, C, D, buff[1], 4, 0xA4BEEA44, 3);
	MD5_TRANSFORM(D, A, B, C, buff[4], 11, 0x4BDECFA9, 3);
	MD5_TRANSFORM(C, D, A, B, buff[7], 16, 0xF6BB4B60, 3);
	MD5_TRANSFORM(B, C, D, A, buff[10], 23, 0xBEBFBC70, 3);

	MD5_TRANSFORM(A, B, C, D, buff[13], 4, 0x289B7EC6, 3);
	MD5_TRANSFORM(D, A, B, C, buff[0], 11, 0xEAA127FA, 3);
	MD5_TRANSFORM(C, D, A, B, buff[3], 16, 0xD4EF3085, 3);
	MD5_TRANSFORM(B, C, D, A, buff[6], 23, 0x4881D05, 3);

	MD5_TRANSFORM(A, B, C, D, buff[9], 4, 0xD9D4D039, 3);
	MD5_TRANSFORM(D, A, B, C, buff[12], 11, 0xE6DB99E5, 3);
	MD5_TRANSFORM(C, D, A, B, buff[15], 16, 0x1FA27CF8, 3);
	MD5_TRANSFORM(B, C, D, A, buff[2], 23, 0xC4AC5665, 3);

	/* Round 4 */

	MD5_TRANSFORM(A, B, C, D, buff[0], 6, 0xF4292244, 4);
	MD5_TRANSFORM(D, A, B, C, buff[7], 10, 0x432AFF97, 4);
	MD5_TRANSFORM(C, D, A, B, buff[14], 15, 0xAB9423A7, 4);
	MD5_TRANSFORM(B, C, D, A, buff[5], 21, 0xFC93A039, 4);

	MD5_TRANSFORM(A, B, C, D, buff[12], 6, 0x655B59C3, 4);
	MD5_TRANSFORM(D, A, B, C, buff[3], 10, 0x8F0CCC92, 4);
	MD5_TRANSFORM(C, D, A, B, buff[10], 15, 0xFFEFF47D, 4);
	MD5_TRANSFORM(B, C, D, A, buff[1], 21, 0x85845DD1, 4);

	MD5_TRANSFORM(A, B, C, D, buff[8], 6, 0x6FA87E4F, 4);
	MD5_TRANSFORM(D, A, B, C, buff[15], 10, 0xFE2CE6E0, 4);
	MD5_TRANSFORM(C, D, A, B, buff[6], 15, 0xA3014314, 4);
	MD5_TRANSFORM(B, C, D, A, buff[13], 21, 0x4E0811A1, 4);

	MD5_TRANSFORM(A, B, C, D, buff[4], 6, 0xF7537E82, 4);
	MD5_TRANSFORM(D, A, B, C, buff[11], 10, 0xBD3AF235, 4);
	MD5_TRANSFORM(C, D, A, B, buff[2], 15, 0x2AD7D2BB, 4);
	MD5_TRANSFORM(B, C, D, A, buff[9], 21, 0xEB86D391, 4);

	context->state[0] += A;
	context->state[1] += B;
	context->state[2] += C;
	context->state[3] += D;
}

static void cavan_md5_update(struct cavan_md5_context *context, const void *buff, size_t size)
{
	size_t remain;
	const void *buff_end = ADDR_ADD(buff, size);

	if (context->remain > 0)
	{
		size_t padding;

		padding = sizeof(context->buff) - context->remain;
		if (padding <= size)
		{
			mem_copy(context->buff + context->remain, buff, padding);
			cavan_md5_transform(context, (u32 *) context->buff);
			buff = ADDR_ADD(buff, padding);
			context->remain = 0;
		}
	}

	while (1)
	{
		remain = ADDR_SUB2(buff_end, buff);
		if (remain < sizeof(context->buff))
		{
			break;
		}

		cavan_md5_transform(context, buff);
		buff = ADDR_ADD(buff, sizeof(context->buff));
	}

	if (remain)
	{
		mem_copy(context->buff + context->remain, buff, remain);
		context->remain += remain;
	}

	context->count += size;
}

static void cavan_md5_finish(struct cavan_md5_context *context, u8 digest[MD5_DIGEST_SIZE])
{
	u64 bits = context->count << 3;

	cavan_md5_update(context, "\x80", 1);

	if (context->remain > sizeof(context->buff) - 8)
	{
		memset(context->buff + context->remain, 0, sizeof(context->buff) - context->remain);
		cavan_md5_transform(context, (u32 *) context->buff);
		memset(context->buff, 0, context->remain);
	}
	else
	{
		memset(context->buff + context->remain, 0, sizeof(context->buff) - context->remain - 8);
	}

	mem_copy(context->buff + sizeof(context->buff) - 8, (void *) &bits, sizeof(bits));

	cavan_md5_transform(context, (u32 *) context->buff);
	mem_copy(digest, context->state, sizeof(context->state));
}

int cavan_md5sum(const void *buff, size_t size, u8 digest[MD5_DIGEST_SIZE])
{
	struct cavan_md5_context context;

	cavan_md5_init(&context);
	cavan_md5_update(&context, buff, size);
	cavan_md5_finish(&context, digest);

	return 0;
}

int cavan_file_md5sum_mmap(const char *pathname, u8 digest[MD5_DIGEST_SIZE])
{
	int fd;
	void *addr;
	size_t size;

	fd = file_mmap(pathname, &addr, &size, O_RDONLY);
	if (fd < 0)
	{
		// pr_red_info("file_mmap");
		return fd;
	}

	cavan_md5sum(addr, size, digest);
	file_unmap(fd, addr, size);

	return 0;
}

int cavan_file_md5sum(const char *pathname, u8 digest[MD5_DIGEST_SIZE])
{
	return cavan_file_md5sum_mmap(pathname, digest);
}
