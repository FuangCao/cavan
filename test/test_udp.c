/*
 * File:		test_udp.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-08-03 13:48:06
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
#include <cavan/network.h>
#include <cavan/command.h>

static void *cavan_test_udp_send_thread(void *data)
{
	cavan_udp_sock_send_loop((struct cavan_udp_sock *) data);
	return NULL;
}

static void *cavan_test_udp_recv_thread(void *data)
{
	cavan_udp_sock_recv_loop((struct cavan_udp_sock *) data);
	return NULL;
}

static int cavan_test_udp_client(int argc, char *argv[])
{
	struct cavan_udp_sock sock;
	char buff[1024];
	int channel;
	int ret;

	assert(argc > 1);

	ret = cavan_udp_sock_open(&sock, 0);
	if (ret < 0) {
		pr_red_info("cavan_udp_sock_open");
		return ret;
	}

	cavan_pthread_run(cavan_test_udp_send_thread, &sock);
	cavan_pthread_run(cavan_test_udp_recv_thread, &sock);

	channel = cavan_udp_sock_connect(&sock, argv[1]);
	if (channel < 0) {
		pr_red_info("cavan_udp_sock_connect");
		return channel;
	}

	memset(buff, 'A', sizeof(buff));

	while (1) {
		if (cavan_udp_sock_send(&sock, channel, buff, sizeof(buff)) < 0) {
			// pr_err_info("cavan_udp_sock_send");
			// msleep(200);
		}
	}

	return 0;
}

static int cavan_test_udp_service(int argc, char *argv[])
{
	struct cavan_udp_sock sock;
	int ret;

	assert(argc > 1);

	ret = cavan_udp_sock_open(&sock, atoi(argv[1]));
	if (ret < 0) {
		pr_red_info("cavan_udp_sock_open");
		return ret;
	}

	cavan_pthread_run(cavan_test_udp_recv_thread, &sock);
	cavan_udp_sock_send_loop(&sock);

	return 0;
}

CAVAN_COMMAND_MAP_START {
	{ "client", cavan_test_udp_client },
	{ "service", cavan_test_udp_service },
} CAVAN_COMMAND_MAP_END;
