/*
 * File:		redpacket.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2016-09-18 12:01:05
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
#include <cavan/command.h>
#include <cavan/network.h>

#define REDPACKET_IP	"224.0.0.1"
#define REDPACKET_PORT	9898

static int redpacket_udp_sender_cmdline(struct network_client *client)
{
	int wrlen = 0;
	char pack[1024];

	while (1) {
		int length;
		char code[1024];

		print_ntext("> ", 2);

		if (fgets(code, sizeof(code), stdin) == NULL) {
			pr_err_info("fgets");
			return -EFAULT;
		}

		length = strlen(code);

		while (length > 0 && code[length - 1] == '\n') {
			code[--length] = 0;
		}

		if (length > 0) {
			println("code[%d] = %s", length, code);
			wrlen = snprintf(pack, sizeof(pack), "RedPacketCode: %s", code);
		}

		if (wrlen > 0) {
			int ret;

			println("pack[%d] = %s", wrlen, pack);

			ret = client->send(client, pack, wrlen);
			if (ret <= 0) {
				pr_err_info("client->send: %d", ret);
				return ret;
			}
		}
	}

	return 0;
}

static int redpacket_udp_sender_main(int argc, char *argv[])
{
	int ret;
	struct network_url url;
	struct network_client client;

	network_url_init(&url, "udp", REDPACKET_IP, REDPACKET_PORT, NULL);

	ret = network_client_open(&client, &url, 0);
	if (ret < 0) {
		pr_err_info("network_client_open: %d", ret);
		return ret;
	}

	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			ret = network_client_send_text(&client, argv[i]);
			if (ret <= 0) {
				pr_err_info("network_client_send_text: %d", ret);
				break;
			}
		}
	} else {
		ret = redpacket_udp_sender_cmdline(&client);
	}

	network_client_close(&client);

	return ret;
}

static int redpacket_udp_receiver_main(int argc, char *argv[])
{
	int ret;
	struct network_url url;
	struct network_service service;

	network_url_init(&url, "udp", "any", REDPACKET_PORT, NULL);

	ret = network_service_open(&service, &url, 0);
	if (ret < 0) {
		pr_err_info("network_service_open: %d", ret);
		return ret;
	}

	while (1) {
		char buff[1024];
		struct sockaddr addr;

		ret = service.recvfrom(&service, buff, sizeof(buff), &addr);
		if (ret <= 0) {
			pr_err_info("service.recvfrom: %d", ret);
			break;
		}

		inet_show_sockaddr((struct sockaddr_in *) &addr);

		buff[ret] = 0;
		println("buff[%d] = %s", ret, buff);
	}

	network_service_close(&service);

	return 0;
}

CAVAN_COMMAND_MAP_START {
	{ "udp_sender", redpacket_udp_sender_main },
	{ "udp_receiver", redpacket_udp_receiver_main },
} CAVAN_COMMAND_MAP_END;
