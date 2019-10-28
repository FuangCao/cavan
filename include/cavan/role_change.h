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
#include <cavan/thread.h>
#include <cavan/network.h>

#define ROLE_CHANGE_UDP_WIN_SIZE	16

enum role_change_mode {
	ROLE_CHANGE_MODE_NONE,
	ROLE_CHANGE_MODE_LINK,
	ROLE_CHANGE_MODE_LINK2,
	ROLE_CHANGE_MODE_BURROW,
};

struct role_change_conn {
	struct network_client client;
	char command[4096];
	char *argv[20];
	int argc;
};

struct role_change_service_conn {
	struct role_change_conn conn;
	struct in_addr addr;
	int keepalive;
	time_t time;
	char *name;
	struct role_change_service_conn *up;
	struct role_change_service_conn *down;
	struct role_change_service_conn *prev;
	struct role_change_service_conn *next;
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
	int mode;
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
	bool burrow;
};

struct role_change_udp_sock;
struct role_change_udp_client;
struct role_change_udp_window;
struct role_change_udp_package;

typedef enum {
	ROLE_CHANGE_UDP_CMD_ERR,
	ROLE_CHANGE_UDP_CMD_DNS,
	ROLE_CHANGE_UDP_CMD_DNS_ACK,
	ROLE_CHANGE_UDP_CMD_CONN,
	ROLE_CHANGE_UDP_CMD_CONN_ACK,
	ROLE_CHANGE_UDP_CMD_DATA,
	ROLE_CHANGE_UDP_CMD_DATA_ACK,
	ROLE_CHANGE_UDP_CMD_DATA_NOACK,
} role_change_udp_command_t;

#pragma pack(1)

typedef struct role_change_udp_header {
	u32 dest:14;
	u32 src:14;
	u32 command:4;
	u16 index;
	u8 buff[0];
} role_change_udp_header_t;

#pragma pack()

typedef struct role_change_udp_package {
	struct role_change_udp_client *client;
	struct role_change_udp_package *next;

	u16 length;
	u8 times;
	u64 time;

	void (*on_send_timeout)(struct role_change_udp_package *pkg);
	void (*on_send_success)(struct role_change_udp_package *pkg, role_change_udp_header_t *header, u16 length);

	role_change_udp_header_t header;
} role_change_udp_package_t;

typedef struct role_change_udp_sock {
	pthread_mutex_t wlock;
	pthread_mutex_t lock;
	pthread_cond_t cond;
	int sockfd;
	u16 port;

	struct role_change_udp_package *head;
	struct role_change_udp_client *clients[1 << 14];
} role_change_udp_sock_t;

typedef struct role_change_udp_window {
	struct role_change_udp_package *packages[ROLE_CHANGE_UDP_WIN_SIZE];
	pthread_mutex_t lock;
	pthread_cond_t cond;
	u16 index;
	u8 head;
	u8 length;
} role_change_udp_window_t;

typedef struct role_change_udp_client {
	struct role_change_udp_sock *sock;
	struct sockaddr_in addr;
	u16 remote_port;
	u16 local_port;
	u64 time;

	struct role_change_udp_package *head;
	struct role_change_udp_package *tail;
	struct role_change_udp_window wr_win;
	struct role_change_udp_window rd_win;

	void (*on_disconnected)(struct role_change_udp_client *client);
} role_change_udp_client_t;

int role_change_service_run(struct cavan_dynamic_service *service);
int role_change_client_run(struct cavan_dynamic_service *service);
int role_change_proxy_run(struct cavan_dynamic_service *service);

int role_change_udp_sock_init(role_change_udp_sock_t *sock);
int role_change_udp_sock_bind(role_change_udp_sock_t *sock, u16 port);
bool role_change_udp_win_received(role_change_udp_window_t *win, struct role_change_udp_header *header, u16 length);
void role_change_udp_client_received(role_change_udp_client_t *client, struct role_change_udp_header *header, u16 length);

void *role_change_udp_send_loop(void *args);
void *role_change_udp_receive_loop(void *args);

role_change_udp_package_t *role_change_udp_package_alloc(role_change_udp_command_t command, const void *buff, u16 length);
void role_change_udp_package_send(role_change_udp_client_t *client, role_change_udp_package_t *pkg);

role_change_udp_client_t *role_change_udp_client_alloc(role_change_udp_sock_t *sock, const char *host, u16 port);
void role_change_udp_client_free(role_change_udp_client_t *client);
role_change_udp_package_t *role_change_udp_client_recv(role_change_udp_client_t *client);

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

static inline void role_change_udp_send_start(role_change_udp_sock_t *sock)
{
	cavan_pthread_run(role_change_udp_send_loop, sock);
}

static inline void role_change_udp_receive_start(role_change_udp_sock_t *sock)
{
	cavan_pthread_run(role_change_udp_receive_loop, sock);
}
