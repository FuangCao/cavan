#pragma once

/*
 * File:		role_change.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2017-12-15 19:07:42
 *
 * Copyright (c) 2017 Fuang.Cao <cavan.cfa@gmail.com>
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

struct role_change_conn {
	struct network_client client;
	char command[4096];
	char *argv[20];
	int argc;
};

struct role_change_service_conn {
	struct role_change_conn conn;
	struct in_addr addr;
	char *name;
	struct role_change_service_conn *up;
	struct role_change_service_conn *down;
	struct role_change_service_conn *left;
	struct role_change_service_conn *right;
};

struct role_change_service {
	struct network_service service;
	struct network_url url;
	struct role_change_service_conn *head;
	pthread_mutex_t lock;
};

struct role_change_client_conn {
	struct role_change_conn conn;
	struct network_client client;
	int keepalive;
	struct role_change_client_conn *prev;
	struct role_change_client_conn *next;
};

struct role_change_client {
	struct network_url url;
	char name[1024];
	struct role_change_client_conn *head;
	pthread_mutex_t lock;
};

struct role_change_proxy {
	struct network_service service;
	struct network_url url_local;
	struct network_url url_remote;
	const char *name;
	const char *url;
};

int role_change_service_run(struct cavan_dynamic_service *service);
int role_change_client_run(struct cavan_dynamic_service *service);
int role_change_proxy_run(struct cavan_dynamic_service *service);

static inline void role_change_service_lock(struct role_change_service *service)
{
	pthread_mutex_lock(&service->lock);
}

static inline void role_change_service_unlock(struct role_change_service *service)
{
	pthread_mutex_unlock(&service->lock);
}

static inline void role_change_client_lock(struct role_change_client *client)
{
	pthread_mutex_lock(&client->lock);
}

static inline void role_change_client_unlock(struct role_change_client *client)
{
	pthread_mutex_unlock(&client->lock);
}
