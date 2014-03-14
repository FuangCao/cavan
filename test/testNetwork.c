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
#if 0
	struct network_url url;

	assert(argc > 1);

	if (network_parse_url(argv[1], &url) == NULL)
	{
		pr_red_info("web_proxy_parse_url");
	}
	else
	{
		println("%s", network_url_tostring(&url, NULL, 0, NULL));
	}
#else
	int ret;
	struct network_connect conn;

	assert(argc > 1);

	ret = network_connect_open(&conn, argv[1]);
	if (ret < 0)
	{
		pr_red_info("network_connect_open");
		return ret;
	}

	network_connect_close(&conn);
#endif

	return 0;
}
