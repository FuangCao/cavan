#define CAVAN_CMD_NAME penetrator

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
	struct cavan_penetrate_service service;
	int ret;

	assert(argc > 1);

	ret = cavan_penetrate_service_init(&service, atoi(argv[1]));
	if (ret < 0) {
		pr_red_info("cavan_penetrate_service_init");
		return ret;
	}

	cavan_penetrate_service_run(&service);
	cavan_penetrate_service_deinit(&service);

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
		char buff[1024];
		int wrlen;
		int rdlen;

		if (fgets(buff, sizeof(buff), stdin) == NULL) {
			break;
		}

		rdlen = strlen(buff);
		if (rdlen > 0) {
			println("send[%d] = %s", rdlen, buff);

			wrlen = inet_sendto(sockfd, buff, rdlen, &addr);
			if (wrlen < 0) {
				pr_err_info("inet_sendto");
				break;
			}
		}

		rdlen = inet_recvfrom_timeout(sockfd, buff, sizeof(buff), &addr, 1000);
		if (rdlen < 0) {
			pr_err_info("inet_sendto");
			if (errno != ETIMEDOUT) {
				break;
			}
		} else {
			inet_show_sockaddr(&addr);
			buff[rdlen] = 0;
			println("recv[%d] = %s", rdlen, buff);
		}
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
