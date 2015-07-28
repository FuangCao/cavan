/*
 * File:		testMux.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-07-28 11:44:01
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
#include <cavan/mux.h>

int main(int argc, char *argv[])
{
	int i;
	int ret;
	struct cavan_mux mux;
	struct cavan_mux_package *package, *packages[10];

	ret = cavan_mux_init(&mux);
	if (ret < 0)
	{
		pr_red_info("cavan_mux_init");
		return ret;
	}

	cavan_mux_show_packages(&mux);

	for (i = 0; i < NELEM(packages); i++)
	{
		packages[i] = cavan_mux_package_alloc(&mux, (i + 1) * 100);
	}

	cavan_mux_show_packages(&mux);

	for (i = NELEM(packages) - 1; i >= 0; i--)
	{
		cavan_mux_package_free(&mux, packages[i]);
	}

	cavan_mux_show_packages(&mux);

	package = cavan_mux_package_alloc(&mux, 560);
	if (package)
	{
		println("alloc length = %d", package->length);
	}

	cavan_mux_show_packages(&mux);

	cavan_mux_deinit(&mux);

	return 0;
}
