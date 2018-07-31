/*
 * File:		penetrator.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-07-31 10:32:29
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
#include <cavan/penetrator.h>

static int cavan_penetrate_service_main(int argc, char *argv[])
{
	struct sockaddr_in addr;
	int sockfd;
	u16 port;
	int ret;

	assert(argc > 1);

	port = atoi(argv[1]);

	sockfd = inet_socket(SOCK_DGRAM);
	if (sockfd < 0) {
		pr_err_info("inet_socket");
		return sockfd;
	}

	inet_sockaddr_init(&addr, NULL, port);

	ret = inet_bind(sockfd, &addr);
	if (ret < 0) {
		pr_err_info("inet_bind");
		goto out_close_sockfd;
	}

	while (1) {
		char buff[1024];
		int rdlen;
		int wrlen;

		rdlen = inet_recvfrom(sockfd, buff, sizeof(buff), &addr);
		if (rdlen < 0) {
			pr_err_info("inet_recvfrom");
			break;
		}

		inet_show_sockaddr(&addr);

		buff[rdlen] = 0;
		println("buff[%d] = %s", rdlen, buff);

		wrlen = inet_sendto(sockfd, buff, rdlen, &addr);
		if (wrlen < 0) {
			pr_err_info("inet_recvfrom");
			break;
		}
	}

out_close_sockfd:
	close(sockfd);
	return 0;
}

static int cavan_penetrate_client_main(int argc, char *argv[])
{
	struct sockaddr_in addr;
	int sockfd;

	assert(argc > 1);

	sockfd = inet_socket(SOCK_DGRAM);
	if (sockfd < 0) {
		pr_err_info("inet_socket");
		return sockfd;
	}

	inet_sockaddr_init_url(&addr, argv[1]);

	while (1) {
		struct sockaddr_in rd_addr;
		char buff[1024];
		int wrlen;
		int rdlen;

		if (fgets(buff, sizeof(buff), stdin) == NULL) {
			break;
		}

		wrlen = inet_sendto(sockfd, buff, strlen(buff), &addr);
		if (wrlen < 0) {
			pr_err_info("inet_sendto");
			break;
		}

		rdlen = inet_recvfrom(sockfd, buff, sizeof(buff), &rd_addr);
		if (rdlen < 0) {
			pr_err_info("inet_sendto");
			break;
		}

		inet_show_sockaddr(&rd_addr);

		println("buff[%d] = %s", rdlen, buff);
	}

	close(sockfd);

	return 0;
}

static int cavan_penetrate_proxy_main(int argc, char *argv[])
{
	pr_pos_info();
	return 0;
}

CAVAN_COMMAND_MAP_START {
	{ "service", cavan_penetrate_service_main },
	{ "client", cavan_penetrate_client_main },
	{ "proxy", cavan_penetrate_proxy_main },
} CAVAN_COMMAND_MAP_END;
