/*
 * File:		pool.c
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
#include <cavan/pool.h>

static void cavan_data_pool_node_destory(struct cavan_data_pool_node *node, void *addr)
{
	struct cavan_data_pool *pool = node->private_data;

	double_link_append(&pool->link, &node->node);
}

static void cavan_data_pool_node_destory2(struct cavan_data_pool_node *node, void *addr)
{
	free(addr);
}

int cavan_data_pool_init(struct cavan_data_pool *pool, int offset, size_t node_size, int count)
{
	int ret;
	size_t size;
	byte *buff, *buff_end;
	struct double_link *link = &pool->link;

	ret = cavan_data_pool_link_init(link, offset);
	if (ret < 0)
	{
		pr_red_info("cavan_data_link_init");
		return ret;
	}

	size = node_size * count;

	buff = malloc(size);
	if (buff == NULL)
	{
		pr_error_info("malloc");
		goto out_double_link_deinit;
	}

	pool->buff = buff;
	pool->offset = offset;
	pool->node_size = node_size;

	for (buff_end = buff + size; buff < buff_end; buff += node_size)
	{
		struct cavan_data_pool_node *data = cavan_data_pool_to_node(pool, buff);

		data->private_data = pool;
		data->destroy = cavan_data_pool_node_destory;
		double_link_node_init(&data->node);
		double_link_append(link, &data->node);
	}

	return 0;

out_double_link_deinit:
	double_link_deinit(link);
	return ret;
}

void cavan_data_pool_deinit(struct cavan_data_pool *pool)
{
	free(pool->buff);
	double_link_deinit(&pool->link);
}

void *cavan_data_pool_alloc(struct cavan_data_pool *pool)
{
	void *data;
	struct double_link_node *node;
	struct cavan_data_pool_node *pool_node;

	node = double_link_pop(&pool->link);
	if (node)
	{
		return double_link_get_container(&pool->link, node);
	}

	data = malloc(pool->node_size);
	if (data == NULL)
	{
		pr_error_info("malloc");
		return NULL;
	}

	pool_node = cavan_data_pool_to_node(pool, data);
	pool_node->destroy = cavan_data_pool_node_destory2;
	double_link_node_init(&pool_node->node);

	return data;
}
