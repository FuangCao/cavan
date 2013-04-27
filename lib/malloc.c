/*
 * File:		malloc.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2013-04-26 15:56:50
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
#include <cavan/malloc.h>

#define CAVAN_MALLOC_DEBUG	0

struct cavan_malloc_info cavan_global_malloc_info;

int cavan_malloc_init_base(struct cavan_malloc_info *info, void *addr, size_t size)
{
	int ret;
	void *last;
	struct cavan_malloc_node *node;

	ret = double_link_init(&info->link, MOFS(struct cavan_malloc_node, node));
	if (ret < 0)
	{
		pr_red_info("double_link_init");
		return ret;
	}

	info->size = size;
	info->buff = addr;

	last = ADDR_ADD(addr, size);
	addr = CAVAN_ADDR_WORD_ALIGN(addr);

	node = addr;
	node->size = cavan_malloc_get_available_size(&info->link, ADDR_SUB2(last, addr));
	if (node->size == 0)
	{
		pr_red_info("size to small");
		ERROR_RETURN(EINVAL);
	}

	node->prev_size = CAVAN_NODE_ALLOCATED_MASK;
	double_link_append(&info->link, &node->node);

	info->last = ADDR_ADD(node, node->size);

	return 0;
}

void cavan_malloc_deinit_base(struct cavan_malloc_info *info)
{
	double_link_deinit(&info->link);
}

void *cavan_malloc_base(struct cavan_malloc_info *info, size_t size)
{
	size_t size_remain;
	struct double_link_node *node, *head;
	struct cavan_malloc_node *alloc_node, *near_next;
	struct double_link *link = &info->link;

	size += link->offset;
	if (size < sizeof(struct cavan_malloc_node))
	{
		size = sizeof(struct cavan_malloc_node);
	}

	double_link_lock(link);

	head = &link->head_node;
	node = head->next;

	while (1)
	{
		if (node == head)
		{
			double_link_unlock(link);
			return NULL;
		}

		alloc_node = double_link_get_container(link, node);
		if (alloc_node->size >= size)
		{
			break;
		}

		node = node->next;
	}

	size_remain = CAVAN_SIZE_WORD_ALIGN_DOWN(alloc_node->size - size);
	if (size_remain > 0)
	{
		size = alloc_node->size - size_remain;
		alloc_node->size = size_remain;

		alloc_node = cavan_malloc_get_next_near_base(node, size_remain);
		alloc_node->size = cavan_malloc_get_available_size(link, size);
		alloc_node->prev_size = size_remain;

		node = &alloc_node->node;
	}
	else
	{
		double_link_remove_base(node);
	}

	near_next = cavan_malloc_get_next_near(alloc_node);
	CAVAN_NODE_SET_ALLOCATED(alloc_node->size);
	if (near_next < info->last)
	{
		near_next->prev_size = alloc_node->size;
	}

	double_link_unlock(link);

	return node;
}

void cavan_free_base(struct cavan_malloc_info *info, void *addr)
{
	u32 size;
	struct double_link_node *node, *head;
	struct cavan_malloc_node *free_node, *near_next;
	struct double_link *link = &info->link;

	double_link_lock(link);

	free_node = double_link_get_container(link, addr);
	free_node->size = CAVAN_NODE_GET_REAL_SIZE(free_node->size);

#if CAVAN_MALLOC_DEBUG
	pr_bold_info("free_node: addr = %p, size = %d, prev_size = %d", free_node, free_node->size, free_node->prev_size);
#endif

	if (CAVAN_NODE_IS_FREE(free_node->prev_size))
	{
		struct double_link_node *prev = cavan_malloc_get_prev_near(free_node);
		struct cavan_malloc_node *near_prev = double_link_get_container(link, prev);

#if CAVAN_MALLOC_DEBUG
		pr_bold_info("near_prev: addr = %p, size = %d, prev_size = %d", near_prev, near_prev->size, near_prev->prev_size);
#endif

		near_prev->size += free_node->size + link->offset;

		free_node = near_prev;
		double_link_remove_base(&near_prev->node);
	}

	near_next = cavan_malloc_get_next_near(free_node);
	if (near_next < info->last)
	{
#if CAVAN_MALLOC_DEBUG
		pr_bold_info("near_next: addr = %p, size = %d, prev_size = %d", near_next, near_next->size, near_next->prev_size);
#endif

		if (CAVAN_NODE_IS_FREE(near_next->size))
		{
			free_node->size += near_next->size + link->offset;
			double_link_remove_base(&near_next->node);

			near_next = cavan_malloc_get_next_near(free_node);
			if (near_next < info->last)
			{
				near_next->prev_size = free_node->size;
			}
		}
		else
		{
			near_next->prev_size = free_node->size;
		}
	}

	size = free_node->size;
	head = &link->head_node;

	for (node = head->next; node != head; node = node->next)
	{
		near_next = double_link_get_container(link, node);
		if (near_next->size >= size)
		{
			break;
		}
	}

	double_link_insert_base2(node, &free_node->node);

	double_link_unlock(link);
}

void cavan_malloc_show_base(struct cavan_malloc_info *info)
{
	struct cavan_malloc_node *node;

	print_sep(60);

	link_foreach_double(&info->link, node)
	{
		pr_bold_info("addr = %p, size = %d, prev_size = %d", node, node->size, node->prev_size);
	}
	end_link_foreach(&info->link);

	print_sep(60);
}
