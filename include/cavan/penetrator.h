#pragma once

/*
 * File:		penetrator.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-07-31 10:32:06
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
#include <cavan/network.h>

struct cavan_penetrate_conn {
	char *name;
	time_t time;
	struct sockaddr_in addr;
	struct cavan_penetrate_conn *prev;
	struct cavan_penetrate_conn *next;
};

struct cavan_penetrate_service {
	int sockfd;
	struct cavan_penetrate_conn *head;
};

struct cavan_penetrate_client {
	struct network_client client;
};

struct cavan_penetrate_proxy {
	struct network_client client;
};

int cavan_penetrate_service_init(struct cavan_penetrate_service *service, u16 port);
void cavan_penetrate_service_run(struct cavan_penetrate_service *service);
void cavan_penetrate_service_deinit(struct cavan_penetrate_service *service);
int cavan_penetrate_client_init(struct cavan_penetrate_client *client);
void cavan_penetrate_client_deinit(struct cavan_penetrate_client *client);
int cavan_penetrate_proxy_init(struct cavan_penetrate_proxy *proxy);
void cavan_penetrate_proxy_deinit(struct cavan_penetrate_proxy *proxy);
