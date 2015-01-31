/*
 * File:		jwp_udp.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-01-27 20:06:25
 *
 * Copyright (c) 2015 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/service.h>
#include <cavan/jwp_udp.h>

static jwp_size_t jwp_udp_hw_read(struct jwp_desc *jwp, void *buff, jwp_size_t size)
{
	ssize_t rdlen;
	struct jwp_udp_desc *udp = (struct jwp_udp_desc *) jwp;

	rdlen = inet_recvfrom(udp->sockfd, buff, size, &udp->addr, &udp->addrlen);
	if (rdlen < 0)
	{
		pr_error_info("network_client_recv");
		return 0;
	}

	return rdlen;
}

static jwp_size_t jwp_udp_hw_write(struct jwp_desc *jwp, const void *buff, jwp_size_t size)
{
	ssize_t wrlen;
	struct jwp_udp_desc *udp = (struct jwp_udp_desc *) jwp;

	wrlen = inet_sendto(udp->sockfd, buff, size, &udp->addr);
	if (wrlen < 0)
	{
		pr_error_info("write");
		return 0;
	}

	return wrlen;
}

jwp_bool jwp_udp_init(struct jwp_udp_desc *udp, const char *hostname, u16 port, void *data)
{
	int sockfd;
	struct jwp_desc *jwp = &udp->jwp;

	if (hostname)
	{
		sockfd = inet_create_udp_service(0);
		inet_sockaddr_init(&udp->addr, hostname, port);
	}
	else
	{
		sockfd = inet_create_udp_service(port);
	}

	if (sockfd < 0)
	{
		pr_red_info("inet_create_udp_service");
		return false;
	}

	udp->sockfd = sockfd;

	jwp->hw_read = jwp_udp_hw_read,
	jwp->hw_write = jwp_udp_hw_write,
	jwp->send_complete = NULL;
	jwp->data_received = NULL;
	jwp->command_received = NULL;
	jwp->package_received = NULL;
	jwp->remote_not_response = NULL;
#if JWP_WRITE_LOG_ENABLE
	jwp->log_received = NULL;
#endif

	if (!jwp_linux_init(&udp->jwp_linux, data))
	{
		pr_red_info("jwp_linux_init");
		goto out_close_sockfd;
	}

	return true;

out_close_sockfd:
	close(sockfd);
	return false;
}

jwp_bool jwp_udp_start(struct jwp_udp_desc *udp)
{
	return jwp_linux_start(&udp->jwp_linux);
}
