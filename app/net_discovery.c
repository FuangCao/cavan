/*
 * File:		net_discovery.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2016-04-06 19:13:31
 *
 * Copyright (c) 2016 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/network.h>

int main(int argc, char *argv[])
{
	u16 port;
	int count;

	if (argc > 1) {
		port = text2value_unsigned(argv[1], NULL, 10);
	} else {
		port = CAVAN_DISCOVERY_PORT;
	}

	count = network_discovery_client_run(port, NULL, NULL);
	if (count <= 0) {
		pr_red_info("No service found");
	}

	return 0;
}
