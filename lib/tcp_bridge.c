/*
 * File:		tcp_bridge.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2016-09-21 17:48:31
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
#include <cavan/tcp_bridge.h>

ssize_t cavan_tcp_bridge_copy(struct network_client *src, struct network_client *dest)
{
	ssize_t rdlen;
	char buff[1024];

	rdlen = src->recv(src, buff, sizeof(buff));
	if (rdlen <= 0 || dest->send(dest, buff, rdlen) < rdlen) {
		return -1;
	}

	return rdlen;
}

void cavan_tcp_bridge_main_loop(struct network_client *client1, struct network_client *client2)
{
	struct pollfd pfds[2];

	pfds[0].events = POLLIN;
	pfds[0].fd = client1->sockfd;

	pfds[1].events = POLLIN;
	pfds[1].fd = client2->sockfd;

	while (1) {
		int ret;

		ret = poll(pfds, NELEM(pfds), -1);
		if (ret <= 0) {
			break;
		}

		if (pfds[0].revents) {
			ret = cavan_tcp_bridge_copy(client1, client2);
			if (ret < 0) {
				break;
			}
		}

		if (pfds[1].revents) {
			ret = cavan_tcp_bridge_copy(client2, client1);
			if (ret < 0) {
				break;
			}
		}
	}
}

int cavan_tcp_bridge_run(const char *url1, const char *url2)
{
	int ret;
	struct network_client client1;
	struct network_client client2;

	pd_info("%s <-> %s", url1, url2);

	ret = network_client_open2(&client1, url1, CAVAN_NET_FLAG_WAIT);
	if (ret < 0) {
		pr_red_info("network_client_open2: %s", url1);
		return ret;
	}

	pd_info("success to open: %s", url1);

	ret = network_client_open2(&client2, url2, CAVAN_NET_FLAG_WAIT);
	if (ret < 0) {
		pr_red_info("network_client_open2: %s", url2);
		goto out_network_client_close1;
	}

	pd_info("success to open: %s", url2);

	cavan_tcp_bridge_main_loop(&client1, &client2);

	network_client_close(&client2);
out_network_client_close1:
	network_client_close(&client1);
	return ret;
}
