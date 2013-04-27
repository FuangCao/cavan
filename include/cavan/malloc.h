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

#define CAVAN_NODE_ALLOCATED_MASK	0x01

#if __WORDSIZE > 32
#define CAVAN_WORD_BYTES			8
#define CAVAN_WORD_MASK				0x07
#elif __WORDSIZE > 16
#define CAVAN_WORD_BYTES			4
#define CAVAN_WORD_MASK				0x03
#else
#define CAVAN_WORD_BYTES			2
#define CAVAN_WORD_MASK				0x01
#endif

#define CAVAN_SIZE_ALIGN(size, bytes, mask) \
	(((size) + (bytes) - 1) & (~(typeof(size))(mask)))

#define CAVAN_SIZE_ALIGN_DOWN(size, mask) \
	((size) & (~(typeof(size))(mask)))

#define CAVAN_ADDR_ALIGN(addr, bytes, mask) \
	((typeof(addr))CAVAN_SIZE_ALIGN((long)(addr), bytes, mask))

#define CAVAN_ADDR_ALIGN_DOWN(addr, mask) \
	((typeof(addr))CAVAN_SIZE_ALIGN_DOWN((long)(addr), mask)))

#define CAVAN_SIZE_WORD_ALIGN(size) \
	CAVAN_SIZE_ALIGN(size, CAVAN_WORD_BYTES, CAVAN_WORD_MASK)

#define CAVAN_SIZE_WORD_ALIGN_DOWN(size) \
	CAVAN_SIZE_ALIGN_DOWN(size, CAVAN_WORD_MASK)

#define CAVAN_ADDR_WORD_ALIGN(addr) \
	CAVAN_ADDR_ALIGN(addr, CAVAN_WORD_BYTES, CAVAN_WORD_MASK)

#define CAVAN_ADDR_WORD_ALIGN_DOWN(addr) \
	CAVAN_ADDR_ALIGN_DOWN(addr, CAVAN_WORD_BYTES, CAVAN_WORD_MASK)

#define CAVAN_NODE_IS_FREE(size) \
	(((size) & CAVAN_NODE_ALLOCATED_MASK) == 0)

#define CAVAN_NODE_SET_ALLOCATED(size) \
	((size) |= CAVAN_NODE_ALLOCATED_MASK)

#define CAVAN_NODE_GET_REAL_SIZE(size) \
	((size) & (~(typeof(size))CAVAN_NODE_ALLOCATED_MASK))

struct cavan_malloc_node
{
	u32 size;
	u32 prev_size;

	struct double_link_node node;
};

struct cavan_malloc_info
{
	void *buff;
	size_t size;
	struct double_link link;
	struct cavan_malloc_node *last;

	void (*destroy)(struct cavan_malloc_info *info);
};

extern struct cavan_malloc_info cavan_global_malloc_info;

int cavan_malloc_init_base(struct cavan_malloc_info *info, void *addr, size_t size, void (*destroy)(struct cavan_malloc_info *info));
void cavan_malloc_deinit_base(struct cavan_malloc_info *info);
void *cavan_malloc_base(struct cavan_malloc_info *info, size_t size);
void cavan_free_base(struct cavan_malloc_info *info, void *addr);
void cavan_malloc_show_base(struct cavan_malloc_info *info);

static inline struct cavan_malloc_node *cavan_malloc_get_next_near_base(struct double_link_node *node, size_t size)
{
	return ADDR_ADD(node, size);
}

static inline struct cavan_malloc_node *cavan_malloc_get_next_near(struct cavan_malloc_node *node)
{
	return cavan_malloc_get_next_near_base(&node->node, node->size);
}

static inline struct double_link_node *cavan_malloc_get_prev_near(struct cavan_malloc_node *node)
{
	return ADDR_SUB(node, node->prev_size);
}

static inline size_t cavan_malloc_get_available_size(struct double_link *link, size_t size)
{
	if (size < sizeof(struct cavan_malloc_node))
	{
		return 0;
	}

	return CAVAN_SIZE_WORD_ALIGN_DOWN(size - link->offset);
}

static inline int cavan_malloc_init(void *addr, size_t size)
{
	return cavan_malloc_init_base(&cavan_global_malloc_info, addr, size, NULL);
}

static inline void cavan_malloc_deinit(void)
{
	cavan_malloc_deinit_base(&cavan_global_malloc_info);
}

static inline void *cavan_malloc(size_t size)
{
	return cavan_malloc_base(&cavan_global_malloc_info, size);
}

static inline void cavan_free(void *addr)
{
	cavan_free_base(&cavan_global_malloc_info, addr);
}

static inline void cavan_malloc_show(void)
{
	cavan_malloc_show_base(&cavan_global_malloc_info);
}
