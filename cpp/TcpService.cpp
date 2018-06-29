/*
 * File:		TcpService.cpp
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-03-21 10:31:13
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
#include <cavan++/TcpClient.h>
#include <cavan++/TcpService.h>

int TcpService::open(NetworkUrl *url)
{
	struct sockaddr_in addr;
	int sockfd;
	int ret;

	sockfd = openSocketTcp();
	if (sockfd < 0) {
		pr_red_info("openSocketTcp");
		return sockfd;
	}

	url->build(&addr, true);

	ret = bind(sockfd, (struct sockaddr *) &addr, sizeof(addr));
	if (ret < 0) {
		pr_err_info("bind: %d", ret);
		goto out_close_sockfd;
	}

	ret = listen(sockfd, 100);
	if (ret < 0) {
		pr_err_info("listen: %d", ret);
		goto out_close_sockfd;
	}

	mSockfd = sockfd;

	return sockfd;

out_close_sockfd:
	closeSocket(sockfd);
	return ret;
}

NetworkClient *TcpService::accept(void)
{
	struct sockaddr_in addr;
	int sockfd;

	sockfd = NetworkService::accept(&addr);
	if (sockfd < 0) {
		// pr_err_info("accept: %d", sockfd);
		return NULL;
	}

	NetworkClient *client = new TcpClient();
	if (client == NULL) {
		pr_err_info("new TcpClient");
		closeSocket(sockfd, true);
		return NULL;
	}

	client->setSockfd(sockfd);

	return client;
}

NetworkClient *TcpServicePacked::accept(void)
{
	return NULL;
}

NetworkClient *TcpServiceMasked::accept(void)
{
	return NULL;
}
