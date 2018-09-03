/*
 * File:		udp_proxy.cpp
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-09-03 19:39:13
 *
 * Copyright (c) 2018 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/command.h>
#include <cavan++/Udp.h>

static void *cavan_udp_proxy_send_thread(void *data)
{
	((UdpSock *) data)->sendLoop();
	return NULL;
}

static int cavan_udp_proxy_client_main(int argc, char *argv[])
{
	UdpSock sock;

	assert(argc > 1);

	if (sock.open(0) < 0) {
		return -EFAULT;
	}

	cavan_pthread_run(cavan_udp_proxy_send_thread, &sock);
	sock.connect(argv[1]);
	sock.recvLoop();

	return 0;
}

static int cavan_udp_proxy_service_main(int argc, char *argv[])
{
	UdpSock sock;

	assert(argc > 1);

	if (sock.open(atoi(argv[1])) < 0) {
		return -EFAULT;
	}

	cavan_pthread_run(cavan_udp_proxy_send_thread, &sock);
	sock.recvLoop();

	return 0;
}

CAVAN_COMMAND_MAP_START {
	{ "client", cavan_udp_proxy_client_main },
	{ "service", cavan_udp_proxy_service_main },
} CAVAN_COMMAND_MAP_END