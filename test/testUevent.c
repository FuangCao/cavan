/*
 * File:		testUevent.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-08-04 18:03:21
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
#include <cavan/network.h>

int main(int argc, char *argv[])
{
	int ret;
	struct network_client client;

	ret = network_client_open2(&client, "uevent:", 0);
	if (ret < 0)
	{
		pr_red_info("network_client_open2");
		return ret;
	}

	while (1)
	{
		ssize_t rdlen;
		char buff[1024];

		rdlen = client.recv(&client, buff, sizeof(buff));
		if (rdlen < 0)
		{
			pr_red_info("client.recv");
			break;
		}

		buff[rdlen] = 0;
		println("buff[%ld] = %s", rdlen, buff);
	}

	network_client_close(&client);

	return 0;
}
