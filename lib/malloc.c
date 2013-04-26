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

struct double_link cavan_global_malloc_link;

int cavan_malloc_init_base(struct double_link *link, void *addr, size_t size)
{
	int ret;
	struct cavan_malloc_node *node;

	ret = double_link_init(link, MOFS(struct cavan_malloc_node, node));
	if (ret < 0)
	{
		pr_red_info("double_link_init");
		return ret;
	}

	node = addr;
	node->size = cavan_malloc_get_available_size(link, size);
	double_link_append(link, &node->node);

	return 0;
}

void cavan_malloc_deinit_base(struct double_link *link)
{
	double_link_deinit(link);
}

void *cavan_malloc_base(struct double_link *link, size_t size)
{
	size_t size_remain;
	struct double_link_node *node, *head;
	struct cavan_malloc_node *free_node;

	if (size < sizeof(struct double_link_node))
	{
		size = sizeof(struct double_link_node);
	}
	else
	{
		size = CAVAN_SIZE_WORD_ALIGN(size);
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

		free_node = double_link_get_container(link, node);
		if (free_node->size >= size)
		{
			break;
		}

		node = node->next;
	}

	size_remain = free_node->size - size;
	if (size_remain > sizeof(struct cavan_malloc_node))
	{
		free_node->size = cavan_malloc_get_available_size(link, size_remain);;

		free_node = cavan_malloc_get_next_near(free_node);
		free_node->size = size;
		node = &free_node->node;
	}
	else
	{
		double_link_remove_base(node);
	}

	double_link_unlock(link);

	return node;
}

void cavan_free_base(struct double_link *link, void *addr)
{
	int count = 0;
	struct cavan_malloc_node *free_node;
	struct double_link_node *next, *prev, *head;

	double_link_lock(link);

	free_node = double_link_get_container(link, addr);

	for (head = &link->head_node, next = head->next; next != head; next = next->next)
	{
		if (next > (struct double_link_node *)addr)
		{
			break;
		}
	}

	prev = next->prev;

	if (prev != head)
	{
		struct cavan_malloc_node *free_prev = double_link_get_container(link, prev);

		if (free_node == cavan_malloc_get_next_near_base(prev, free_prev->size))
		{
			free_prev->size += free_node->size + link->offset;
			free_node = free_prev;
			count++;
		}
	}

	if (next != head)
	{
		struct cavan_malloc_node *free_next = double_link_get_container(link, next);
		struct cavan_malloc_node *near_next = cavan_malloc_get_next_near(free_node);

		if (free_next == near_next)
		{
			double_link_insert_base(next->prev, next->next, &free_node->node);
			free_node->size += near_next->size + link->offset;
			count++;
		}
	}

	if (count == 0)
	{
		double_link_insert_base2(next, &free_node->node);
	}

	double_link_unlock(link);
}

void cavan_malloc_show_base(struct double_link *link)
{
	struct cavan_malloc_node *node;

	print_sep(60);

	link_foreach_double(link, node)
	{
#if __WORDSIZE == 64
		pr_bold_info("addr = %p, size = %ld", node, node->size);
#else
		pr_bold_info("addr = %p, size = %d", node, node->size);
#endif
	}
	end_link_foreach(link);

	print_sep(60);
}
