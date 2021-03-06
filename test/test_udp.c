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
#include <cavan/udp.h>
#include <cavan/command.h>

static struct cavan_udp_sock *g_udp_sock;

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

static void *cavan_test_udp_recv_loop(void *data)
{
	struct cavan_udp_link *link = (struct cavan_udp_link *) data;
	u16 channel = link->local_channel;
	int index = 1;

	while (1) {
		char buff[1024];
		int length;
		int value;

		length = cavan_udp_sock_recv(g_udp_sock, channel, buff, sizeof(buff), false);
		if (length < 0) {
			pr_err_info("cavan_udp_sock_recv: %d", length);
			break;
		}

		buff[length] = 0;

		println("buff[%d] = %s", length, buff);

		if (sscanf(buff, "message: %d", &value) != 1) {
			pr_red_info("invalid message");
			break;
		}

		if (value != index) {
			pr_red_info("invalid message");
			break;
		}

		index++;
	}

	pr_pos_info();

	return NULL;
}

static void cavan_test_udp_on_connected(struct cavan_udp_sock *sock, struct cavan_udp_link *link)
{
	cavan_pthread_run(cavan_test_udp_recv_loop, link);
}

static int cavan_test_udp_client(int argc, char *argv[])
{
	struct cavan_udp_sock sock;
	char buff[1024];
	int channel;
	u32 count;
	int ret;

	assert(argc > 1);

	catch_sigsegv();

	g_udp_sock = &sock;

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

	cavan_pthread_run(cavan_test_udp_recv_loop, sock.links[channel]);

	count = 0;

	while (1) {
		int length = snprintf(buff, sizeof(buff), "message: %d", count + 1);

		if (cavan_udp_sock_send(&sock, channel, buff, length, false) < 0) {
			break;
		}

		// msleep(1);
		count++;
	}

	return 0;
}

static int cavan_test_udp_service(int argc, char *argv[])
{
	struct cavan_udp_sock sock;
	int ret;

	assert(argc > 1);

	catch_sigsegv();

	g_udp_sock = &sock;
	sock.on_connected = cavan_test_udp_on_connected;

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
