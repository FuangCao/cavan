#pragma once

// Fuang.Cao <cavan.cfa@gmail.com> Wed Apr 13 10:47:06 CST 2011

#include <cavan.h>
#include <cavan/list.h>
#include <cavan/pool.h>
#include <cavan/thread.h>

struct list_queue
{
	void **buffer;
	int head, tail;
	int max_count;
};

struct cavan_data_queue
{
	void *private_data;
	struct double_link link;
	struct cavan_thread thread;
	struct cavan_data_pool pool;

	void (*handler)(void *addr, void *data);
};

int list_queue_init(struct list_queue *queue, int count);
void list_queue_free(struct list_queue *queue);
int list_queue_insert(struct list_queue *queue, void *data);
void *list_queue_remove(struct list_queue *queue);
void *list_queue_get_head_data(struct list_queue *queue);
void *list_queue_get_tail_data(struct list_queue *queue);

int cavan_data_queue_init(struct cavan_data_queue *queue, int offset, size_t node_size, int pool_size, void *data);
void cavan_data_queue_deinit(struct cavan_data_queue *queue);

static inline int list_queue_empty(struct list_queue *queue)
{
	return queue->head == queue->tail;
}

static inline int list_queue_full(struct list_queue *queue)
{
	return ((queue->head + 1) % queue->max_count) == queue->tail;
}

static inline int cavan_data_queue_start(struct cavan_data_queue *queue)
{
	return cavan_thread_start(&queue->thread);
}

static inline void cavan_data_queue_stop(struct cavan_data_queue *queue)
{
	cavan_thread_stop(&queue->thread);
}

static inline int cavan_data_queue_run(struct cavan_data_queue *queue, int offset, size_t node_size, int pool_size, void *data)
{
	int ret;

	ret = cavan_data_queue_init(queue, offset, node_size, pool_size, data);
	if (ret < 0)
	{
		return ret;
	}

	return cavan_data_queue_start(queue);
}

static inline void cavan_data_queue_append(struct cavan_data_queue *queue, struct cavan_data_pool_node *node)
{
	double_link_append(&queue->link, &node->node);
	cavan_thread_resume(&queue->thread);
}

static inline void *cavan_data_queue_get_node(struct cavan_data_queue *queue)
{
	return cavan_data_pool_alloc(&queue->pool);
}

static inline void *cavan_data_queue_get_container(struct cavan_data_queue *queue, struct cavan_data_pool_node *node)
{
	return double_link_get_container(&queue->link, &node->node);
}
