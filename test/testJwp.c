/*
 * File:			testJwp.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-01-22 10:12:11
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

static void package_received(struct jwp_desc *desc, struct jwp_package *pkg)
{
	jwp_package_dump(pkg);
}

int main(int argc, char *argv[])
{
	int i;
	u8 length;
	struct jwp_desc desc =
	{
		.package_received = package_received,
	};
	struct jwp_package pkg;
	u8 buff[] = { 1, 2, 3, 4, 5, 6, JWP_MAGIC_LOW, JWP_MAGIC_HIGH, JWP_PKG_DATA, 0, 2, 0, 1, 2, 3, 4 };

	jwp_package_init(&pkg, &desc);

	length = sizeof(buff);
	jwp_println("length = %d", length);
	length = jwp_package_fill(&pkg, buff, length);
	jwp_println("length = %d", length);

	for (i = 0; i < (int) sizeof(buff); i++)
	{
		length = jwp_package_fill(&pkg, buff + i, 1);
		jwp_println("%d. length = %d", i, length);
	}

	return 0;
}
