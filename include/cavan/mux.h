#pragma once

/*
 * File:		mux.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-07-28 11:43:37
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
#include <cavan/queue.h>
#include <cavan/thread.h>

#define CAVAN_MUX_LINK_TABLE_MASK		0xFF
#define CAVAN_MUX_MTU					2048
#define CAVAN_MUX_MAGIC					0x1234

#define CAVAN_MUX_PACKAGE_GET_RAW(package) \
	MEMBER_TO_STRUCT(package, struct cavan_mux_package_raw, package)

struct cavan_mux_link;

struct cavan_mux_package
{
	u16 magic;
	u16 src_port;
	u16 dest_port;
	u16 length;
	char data[0];
};

struct cavan_mux_package_raw
{
	u16 length;
	struct cavan_mux_package_raw *next;
	struct cavan_mux_package package;
};

struct cavan_mux
{
	struct cavan_lock lock;
	struct cavan_mux_package_raw *packages;

	void *private_data;
	struct cavan_thread send_thread;
	struct cavan_mux_package_raw *package_head;
	struct cavan_mux_package_raw **package_tail;

	struct cavan_thread recv_thread;
	struct cavan_mem_queue recv_queue;
	struct cavan_mux_link *links[CAVAN_MUX_LINK_TABLE_MASK + 1];

	ssize_t (*send)(struct cavan_mux *mux, const void *buff, size_t size);
	ssize_t (*recv)(struct cavan_mux *mux, void *buff, size_t size);
};

struct cavan_mux_link
{
	u16 local_port;
	u16 remote_port;

	void *private_data;
	struct cavan_mux *mux;
	struct cavan_lock lock;
	struct cavan_mux_link *next;

	size_t hole_size;
	struct cavan_mux_package_raw *package_head;
	struct cavan_mux_package_raw **package_tail;

	void (*on_received)(struct cavan_mux_link *link);
};

int cavan_mux_init(struct cavan_mux *mux, void *data);
void cavan_mux_deinit(struct cavan_mux *mux);
void cavan_mux_append_package(struct cavan_mux *mux, struct cavan_mux_package_raw *package);
struct cavan_mux_package_raw *cavan_mux_dequeue_package(struct cavan_mux *mux, size_t length);
void cavan_mux_show_packages(struct cavan_mux *mux);
struct cavan_mux_package *cavan_mux_package_alloc(struct cavan_mux *mux, size_t length);
void cavan_mux_package_free(struct cavan_mux *mux, struct cavan_mux_package *package);
int cavan_mux_add_link(struct cavan_mux *mux, struct cavan_mux_link *link, u16 port);
struct cavan_mux_link *cavan_mux_find_link(struct cavan_mux *mux, u16 port);
void cavan_mux_unbind(struct cavan_mux *mux, struct cavan_mux_link *link);
int cavan_mux_find_free_port(struct cavan_mux *mux, u16 *pport);
int cavan_mux_bind(struct cavan_mux *mux, struct cavan_mux_link *link, u16 port);
u16 cavan_mux_alloc_port(struct cavan_mux *mux);
int cavan_mux_append_receive_package(struct cavan_mux *mux, struct cavan_mux_package *package);
ssize_t cavan_mux_append_receive_data(struct cavan_mux *mux, const void *buff, size_t size);
void cavan_mux_append_send_package(struct cavan_mux *mux, struct cavan_mux_package *package);

void cavan_mux_link_init(struct cavan_mux_link *link, struct cavan_mux *mux);
void cavan_mux_link_deinit(struct cavan_mux_link *link);
int cavan_mux_link_append_receive_package(struct cavan_mux_link *link, struct cavan_mux_package *package);
ssize_t cavan_mux_link_recv(struct cavan_mux_link *link, void *buff, size_t size);
ssize_t cavan_mux_link_send(struct cavan_mux_link *link, const void *buff, size_t size);

static inline size_t cavan_mux_package_get_whole_length(const struct cavan_mux_package *package)
{
	return sizeof(struct cavan_mux_package) + package->length;
}

static inline int cavan_mux_link_head_index(u16 port)
{
	return port & CAVAN_MUX_LINK_TABLE_MASK;
}

static inline int cavan_mux_link_bind(struct cavan_mux_link *link, u16 port)
{
	return cavan_mux_bind(link->mux, link, port);
}

static inline void cavan_mux_link_unbind(struct cavan_mux_link *link)
{
	return cavan_mux_unbind(link->mux, link);
}
