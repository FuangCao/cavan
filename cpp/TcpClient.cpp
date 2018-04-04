/*
 * File:		TcpClient.cpp
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-03-21 10:31:07
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

int TcpClient::open(NetworkUrl *url)
{
	struct sockaddr_in addr;
	int sockfd;
	int ret;

	sockfd = openSocketTcp();
	if (sockfd < 0) {
		pr_red_info("openSocketTcp: %d", sockfd);
		return sockfd;
	}

	url->build(&addr);

	ret = connect(sockfd, (struct sockaddr *) &addr, sizeof(addr));
	if (ret < 0) {
		pr_err_info("connect: %d", ret);
		goto out_close_sockfd;
	}

	mSockfd = sockfd;

	return true;

out_close_sockfd:
	closeSocket(sockfd);
	return ret;
}
