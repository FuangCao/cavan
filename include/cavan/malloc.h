#pragma once

/*
 * File:		malloc.h
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
#include <cavan/list.h>

#if __WORDSIZE == 64
#define CAVAN_SIZE_WORD_ALIGN(size) \
	(((size) + 7) & (~((typeof(size))0x07)))
#else
#define CAVAN_SIZE_WORD_ALIGN(size) \
	(((size) + 3) & (~((typeof(size))0x03)))
#endif

struct cavan_malloc_node
{
	size_t size;
	struct double_link_node node;
};

extern struct double_link cavan_global_malloc_link;

int cavan_malloc_init_base(struct double_link *link, void *addr, size_t size);
void cavan_malloc_deinit_base(struct double_link *link);
void *cavan_malloc_base(struct double_link *link, size_t size);
void cavan_free_base(struct double_link *link, void *addr);
void cavan_malloc_show_base(struct double_link *link);

static inline struct cavan_malloc_node *cavan_malloc_get_next_near_base(struct double_link_node *node, size_t size)
{
	return ADDR_ADD(node, size);
}

static inline struct cavan_malloc_node *cavan_malloc_get_next_near(struct cavan_malloc_node *node)
{
	return cavan_malloc_get_next_near_base(&node->node, node->size);
}

static inline size_t cavan_malloc_get_available_size(struct double_link *link, size_t size)
{
	return size - link->offset;
}

static inline int cavan_malloc_init(void *addr, size_t size)
{
	return cavan_malloc_init_base(&cavan_global_malloc_link, addr, size);
}

static inline void cavan_malloc_deinit(void)
{
	cavan_malloc_deinit_base(&cavan_global_malloc_link);
}

static inline void *cavan_malloc(size_t size)
{
	return cavan_malloc_base(&cavan_global_malloc_link, size);
}

static inline void cavan_free(void *addr)
{
	cavan_free_base(&cavan_global_malloc_link, addr);
}

static inline void cavan_malloc_show(void)
{
	cavan_malloc_show_base(&cavan_global_malloc_link);
}
