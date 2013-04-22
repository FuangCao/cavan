#pragma once

// Fuang.Cao <cavan.cfa@gmail.com> Wed Apr 13 10:47:06 CST 2011

#include <cavan.h>

struct list_queue
{
	void **buffer;
	int head, tail;
	int max_count;
};

static inline int list_queue_empty(struct list_queue *queue)
{
	return queue->head == queue->tail;
}

static inline int list_queue_full(struct list_queue *queue)
{
	return ((queue->head + 1) % queue->max_count) == queue->tail;
}

int list_queue_init(struct list_queue *queue, int count);
void list_queue_free(struct list_queue *queue);
int list_queue_insert(struct list_queue *queue, void *data);
void *list_queue_remove(struct list_queue *queue);
void *list_queue_get_head_data(struct list_queue *queue);
void *list_queue_get_tail_data(struct list_queue *queue);
