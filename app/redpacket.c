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

#define REDPACKET_IP			"224.0.0.1"
#define REDPACKET_PORT			9898
#define REDPACKET_PREFIX		"RedPacketCode: "
#define REDPACKET_PREFIX_LEN	(sizeof(REDPACKET_PREFIX) - 1)

static inline int redpacket_build_pack(char *buff, size_t size, const char *code)
{
	return snprintf(buff, size, REDPACKET_PREFIX "%s", code);
}

static int redpacket_sender_cmdline(struct network_client *client)
{
	int wrlen = 0;
	char pack[1024];

	while (1) {
		char *p;
		char code[1024];

		print_ntext("> ", 2);

		if (fgets(code, sizeof(code), stdin) == NULL) {
			pr_err_info("fgets");
			return -EFAULT;
		}

		p = text_strip(code, strlen(code), code, sizeof(code));
		if (p > code) {
			println("code = %s", code);
			wrlen = redpacket_build_pack(pack, sizeof(pack), code);
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

static int redpacket_sender_main(int argc, char *argv[])
{
	int ret;
	struct network_url url;
	struct network_client client;

	network_url_init(&url, "udp", REDPACKET_IP, REDPACKET_PORT, NULL);

	ret = network_url_parse_cmdline(&url, NULL, argc, argv);
	if (ret < 0) {
		return ret;
	}

	ret = network_client_open(&client, &url, 0);
	if (ret < 0) {
		pr_err_info("network_client_open: %d", ret);
		return ret;
	}

	if (optind < argc) {
		int i;

		for (i = optind; i < argc; i++) {
			char buff[1024];
			int length = redpacket_build_pack(buff, sizeof(buff), argv[i]);

			ret = client.send(&client, buff, length);
			if (ret <= 0) {
				pr_err_info("client.send: %d", ret);
				break;
			}
		}
	} else {
		ret = redpacket_sender_cmdline(&client);
	}

	network_client_close(&client);

	return ret;
}

static int redpacket_receiver_main(int argc, char *argv[])
{
	int ret;
	struct network_url url;
	struct network_service service;

	network_url_init(&url, "udp", "any", REDPACKET_PORT, NULL);

	ret = network_url_parse_cmdline(&url, NULL, argc, argv);
	if (ret < 0) {
		return ret;
	}

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
		print_ntext(buff, ret);
		print_char('\n');
	}

	network_service_close(&service);

	return 0;
}

CAVAN_COMMAND_MAP_START {
	{ "sender", redpacket_sender_main },
	{ "receiver", redpacket_receiver_main },
} CAVAN_COMMAND_MAP_END;
