/*
 * File:		ping.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2014-07-11 11:35:30
 *
 * Copyright (c) 2014 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/timer.h>
#include <cavan/network.h>

#pragma pack(1)
struct ping_send_package
{
	struct icmp_header icmp;
	struct ping_header ping;
	u64 time;
};

struct ping_recv_package
{
	struct ip_header ip;
	struct ping_send_package pkg;
};
#pragma pack()

static void *ping_recv_thread(void *data)
{
	u16 seq = 0;
	struct network_client *client = data;
	struct ping_recv_package pkg;
	struct ip_header *ip = &pkg.ip;
	struct icmp_header *icmp = &pkg.pkg.icmp;
	struct ping_header *ping = &pkg.pkg.ping;

	while (1)
	{
		ssize_t rdlen;

		rdlen = client->recv(client, &pkg, sizeof(pkg));
		if (rdlen < 1)
		{
			pr_red_info("client->recv");
			break;
		}

		if (rdlen != sizeof(pkg))
		{
			continue;
		}

		if (ip->protocol_type != IPPROTO_ICMP || icmp->type != 0)
		{
			continue;
		}

		if (ping->seq <= seq)
		{
			continue;
		}

		seq = ping->seq;

		println("%" PRINT_FORMAT_SIZE " bytes from %s: icmp_seq=%d ttl=%d time=%lf ms",
			rdlen, inet_ntoa(*(struct in_addr *) &ip->src_ip), seq, ip->ttl, (double) (clock_gettime_ns_mono() - pkg.pkg.time) / (1000 * 1000));
	}

	return NULL;
}

int main(int argc, char *argv[])
{
	int ret;
	u16 seq;
	pthread_t thread_recv;
	struct network_url url;
	struct network_client client;
	struct ping_send_package pkg;
	struct icmp_header *icmp = &pkg.icmp;
	struct ping_header *ping = &pkg.ping;

	assert(argc > 1);

	url.protocol = "icmp";
	url.hostname = argv[1];
	url.port = NETWORK_PORT_INVALID;
	url.pathname = NULL;

	ret = network_client_open(&client, &url, 0);
	if (ret < 0)
	{
		pr_red_info("network_client_open");
		return ret;
	}

	pthread_create(&thread_recv, NULL, ping_recv_thread, &client);

	ping->id = getpid();
	icmp->type = 8;
	icmp->code = 0;

	for (seq = 1; ; seq++)
	{
		ssize_t wrlen;

		ping->seq = seq;
		pkg.time = clock_gettime_ns_mono();
		icmp->checksum = 0;
		icmp->checksum = mem_checksum16(&pkg, sizeof(pkg));

		wrlen = client.send(&client, &pkg, sizeof(pkg));
		if (wrlen != sizeof(pkg))
		{
			pr_red_info("client.send");
			break;
		}

		msleep(500);
	}

	network_client_close(&client);

	return 0;
}
