#pragma once

/*
 * File:		udp.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-08-16 14:50:12
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

#include <cavan/network.h>

#define CAVAN_UDP_WIN_SIZE			64
#define CAVAN_UDP_CWND				16
#define CAVAN_UDP_OVERTIME			200

typedef enum {
	CAVAN_UDP_TEST = 0,
	CAVAN_UDP_SYNC = 1,
	CAVAN_UDP_SYNC_ACK1,
	CAVAN_UDP_SYNC_ACK2,
	CAVAN_UDP_DATA,
	CAVAN_UDP_WIND,
	CAVAN_UDP_PING,
	CAVAN_UDP_DATA_ACK,
	CAVAN_UDP_ERROR,
} cavan_udp_pack_t;

#pragma pack(1)
struct cavan_udp_header {
	u16 dest_channel;
	u16 src_channel;
	u16	sequence;
	u8 type;
	u8 win;

	union {
		u8 data[0];
		u16 data16[0];
		u32 data32[0];
		u64 data64[0];
	};
};
#pragma pack()

struct cavan_udp_pack {
	u64 time;
	u16 times;
	u16 length;
	struct cavan_udp_pack *next;
	struct cavan_udp_header header;
};

struct cavan_udp_queue {
	struct cavan_udp_pack *head;
	struct cavan_udp_pack *tail;
	u16 size;
};

struct cavan_udp_win {
	struct cavan_udp_pack *packs[CAVAN_UDP_WIN_SIZE];
	pthread_cond_t cond;
	u16 length;
	u16 index;
	u16 ready;
	bool full;
};

struct cavan_udp_link {
	struct sockaddr_in addr;
	pthread_mutex_t lock;
	u16 remote_channel;
	u16 local_channel;
	u16 sequence;
	u16 ssthresh;
	u16 cwnd;
	u16 acks;
	u64 time;
	struct cavan_udp_link *prev;
	struct cavan_udp_link *next;
	struct cavan_udp_win send_win;
	struct cavan_udp_win recv_win;
};

struct cavan_udp_sock {
	struct cavan_udp_link *links[0xFFFF];
	pthread_mutex_t lock;
	pthread_cond_t cond;
	int sockfd;
	int index;
	struct cavan_udp_link *head;

	void (*on_connected)(struct cavan_udp_sock *sock, struct cavan_udp_link *link);
};

int cavan_udp_sock_open(struct cavan_udp_sock *sock, u16 port);
void cavan_udp_sock_close(struct cavan_udp_sock *sock);
void cavan_udp_sock_send_loop(struct cavan_udp_sock *sock);
void cavan_udp_sock_recv_loop(struct cavan_udp_sock *sock);
struct cavan_udp_link *cavan_udp_link_alloc(struct cavan_udp_sock *sock);
void cavan_udp_link_free(struct cavan_udp_sock *sock, struct cavan_udp_link *link);
ssize_t cavan_udp_sock_send(struct cavan_udp_sock *sock, u16 channel, const void *buff, size_t size, bool nonblock);
ssize_t cavan_udp_sock_recv(struct cavan_udp_sock *sock, u16 channel, void *buff, size_t size, bool nonblock);
int cavan_udp_sock_shutdown(struct cavan_udp_sock *sock);
int cavan_udp_sock_accept(struct cavan_udp_sock *sock);
int cavan_udp_sock_connect(struct cavan_udp_sock *sock, const char *url);
