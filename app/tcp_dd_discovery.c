/*
 * File:		tcp_dd_discovery.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2016-05-03 19:10:33
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
#include <cavan/tcp_dd.h>

static void tcp_dd_discovery_handler(const char *message, void *data)
{
	pr_green_info("message = %s", message);
}

int main(int argc, char *argv[])
{
	u16 port;

	if (argc > 1) {
		port = text2value_unsigned(argv[1], NULL, 10);
	} else {
		port = TCP_DD_DEFAULT_PORT;
	}

	return tcp_dd_discovery(port, NULL, tcp_dd_discovery_handler);
}
