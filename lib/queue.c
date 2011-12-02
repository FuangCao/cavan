// Fuang.Cao <cavan.cfa@gmail.com> Wed Apr 13 10:47:06 CST 2011

#include <cavan.h>
#include <cavan/queue.h>

int list_queue_init(struct list_queue *queue, int count)
{
	queue->buffer = malloc(sizeof(void *) * count);
	if (queue->buffer == NULL)
	{
		return -ENOMEM;
	}

	queue->max_count = count;
	queue->head = queue->tail = 0;

	return 0;
}

void list_queue_free(struct list_queue *queue)
{
	free(queue->buffer);
	queue->max_count = 0;
}

int list_queue_insert(struct list_queue *queue, void *data)
{
	if (list_queue_full(queue))
	{
		return -ENOMEM;
	}

	queue->buffer[queue->head] = data;
	queue->head = (queue->head + 1) % queue->max_count;

	return 0;
}

void *list_queue_remove(struct list_queue *queue)
{
	void *data;

	if (list_queue_empty(queue))
	{
		return NULL;
	}

	data = queue->buffer[queue->tail];

	queue->tail = (queue->tail + 1) % queue->max_count;

	return data;
}

void *list_queue_get_head_data(struct list_queue *queue)
{
	if (list_queue_empty(queue))
	{
		return NULL;
	}

	return queue->buffer[queue->head];
}

void *list_queue_get_tail_data(struct list_queue *queue)
{
	if (list_queue_empty(queue))
	{
		return NULL;
	}

	return queue->buffer[(queue->tail + 1) % queue->max_count];
}
