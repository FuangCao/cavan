#pragma once

/*
 * File:		reliable_udp.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2016-10-17 11:14:59
 *
 * Copyright (c) 2016 Fuang.Cao <cavan.cfa@gmail.com>
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

#define RELIABLE_UDP_WIN_SIZE		32

#define RELIABLE_UDP_FLAG_SYN		(1 << 0)
#define RELIABLE_UDP_FLAG_FIN		(1 << 1)
#define RELIABLE_UDP_FLAG_ACK		(1 << 2)
#define RELIABLE_UDP_FLAG_RST		(1 << 3)

typedef enum {
	RELIABLE_UDP_STATE_CLOSED,
	RELIABLE_UDP_STATE_SYN_SEND,
	RELIABLE_UDP_STATE_SYN_RECV,
	RELIABLE_UDP_STATE_CONNECTED,
} reliable_udp_state_t;

#pragma pack(1)

struct reliable_udp_header {
	u16 index;
	u16	index_ack;
	u16 checksum;
	u8 channel;
	u8 flags;
};

#pragma pack()

struct reliable_udp_package {
	u64 time;
	u64 send_time;
	u8 send_count;
	bool need_ack;
	bool ack_received;
	struct reliable_udp_package *next;

	u16 size;
	struct reliable_udp_header package[0];
};

struct reliable_udp_client {
	u16 rtt;
	u16 index;
	u16	index_ack;
	u16 index_send;
	void *client_data;
	reliable_udp_state_t state;
	struct reliable_udp_package *send_head;
	struct reliable_udp_header *recv_packages[RELIABLE_UDP_WIN_SIZE];

	void (*wait)(struct reliable_udp_client *client);
	void (*msleep)(struct reliable_udp_client *client, u32 msec);
	void (*wakeup)(struct reliable_udp_client *client);
	void (*lock_send)(struct reliable_udp_client *client);
	void (*unlock_send)(struct reliable_udp_client *client);
	void (*lock_recv)(struct reliable_udp_client *client);
	void (*unlock_recv)(struct reliable_udp_client *client);
	void *(*alloc_mem)(size_t size);
	void (*free_mem)(void *mem);
	ssize_t (*send_packet)(struct reliable_udp_client *client, const void *pacakge, size_t size);
	ssize_t (*recv_packet)(struct reliable_udp_client *client, void *pacakge, size_t size);
};

void reliable_udp_client_init(struct reliable_udp_client *client, void *data);
void reliable_udp_send_main_loop(struct reliable_udp_client *client);
void reliable_udp_recv_main_loop(struct reliable_udp_client *client);
