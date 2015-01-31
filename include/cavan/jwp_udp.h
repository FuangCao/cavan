#pragma once

/*
 * File:		jwp_udp.h
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
#include <cavan/timer.h>
#include <cavan/network.h>
#include <cavan/jwp-linux.h>

struct jwp_udp_desc
{
	union
	{
		struct jwp_desc jwp;
		struct jwp_linux_desc jwp_linux;
	};

	int sockfd;
	socklen_t addrlen;
	struct sockaddr_in addr;
};

jwp_bool jwp_udp_init(struct jwp_udp_desc *udp, const char *hostname, u16 port, void *data);
jwp_bool jwp_udp_start(struct jwp_udp_desc *udp);
