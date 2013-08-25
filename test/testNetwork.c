/*
 * File:		testNetwork.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2013-08-22 09:48:48
 *
 * Copyright (c) 2013 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/tcp_proxy.h>
#include <cavan/network.h>

int main(int argc, char *argv[])
{
	char protocol[8];
	char hostname[512];

	assert(argc > 1);

	if (web_proxy_parse_url(argv[1], protocol, sizeof(protocol), hostname, sizeof(hostname)) == NULL)
	{
		pr_red_info("web_proxy_parse_url");
	}
	else
	{
		println("hostname = %s, protocol = %s", hostname, protocol);
	}

	return 0;
}
