/*
 * File:		sha1sum.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2014-05-28 17:51:51
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

int main(int argc, char *argv[])
{
	int i;
	char buff[1024];
	u8 digest[SHA1_DIGEST_SIZE];
	struct cavan_sha_context context;

	cavan_sha1_context_init(&context);

	for (i = 1; i < argc; i++)
	{
		int ret;

		ret = cavan_file_shasum(&context, argv[i], digest);
		if (ret < 0)
		{
			pr_red_info("cavan_file_sha1sum %s", argv[i]);
		}
		else
		{
			println("%s  %s", cavan_shasum_tostring(digest, sizeof(digest), buff, sizeof(buff)), argv[i]);
		}
	}

	return 0;
}
