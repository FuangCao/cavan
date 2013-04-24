#pragma once

/*
 * File:		pool.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2013-04-24 10:23:43
 *
 * Copyright (c) 2013 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/list.h>

struct cavan_data_pool_node
{
	void *private_data;
	struct double_link_node node;

	void (*destroy)(struct cavan_data_pool_node *node, void *addr);
};

struct cavan_data_pool
{
	void *buff;
	int offset;
	size_t node_size;
	struct double_link link;
};

int cavan_data_pool_init(struct cavan_data_pool *pool, int offset, size_t node_size, int count);
void cavan_data_pool_deinit(struct cavan_data_pool *pool);
void *cavan_data_pool_alloc(struct cavan_data_pool *pool);

static inline int cavan_data_pool_link_init(struct double_link *link, int offset)
{
	return double_link_init(link, offset + MOFS(struct cavan_data_pool_node, node));
}

static inline void *cavan_data_pool_get_container(struct cavan_data_pool *pool, struct cavan_data_pool_node *node)
{
	return ADDR_SUB(node, pool->offset);
}

static inline struct cavan_data_pool_node *cavan_data_pool_to_node(struct cavan_data_pool *pool, void *addr)
{
	return ADDR_ADD(addr, pool->offset);
}

static inline void cavan_data_pool_node_free(struct cavan_data_pool *pool, void *addr)
{
	struct cavan_data_pool_node *node = cavan_data_pool_to_node(pool, addr);

	node->destroy(node, addr);
}
