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

// ================================================================================

static int cavan_data_queue_thread_handler(struct cavan_thread *thread, void *data)
{
	struct double_link_node *node;
	struct cavan_data_queue *queue = data;

	node = double_link_pop(&queue->link);
	if (node)
	{
		data = double_link_get_container(&queue->link, node);
		queue->handler(queue, data, queue->private_data);
		cavan_data_pool_node_free(&queue->pool, data);
	}
	else
	{
		cavan_thread_suspend(thread);
	}

	return 0;
}

int cavan_data_queue_init(struct cavan_data_queue *queue, int offset, size_t node_size, int pool_size, void *data)
{
	int ret;
	struct cavan_thread *thread;

	if (queue->handler == NULL)
	{
		pr_red_info("queue->handler == NULL");
		ERROR_RETURN(EINVAL);
	}

	ret = cavan_data_pool_link_init(&queue->link, offset);
	if (ret < 0)
	{
		pr_red_info("double_link_init");
		return ret;
	}

	thread = &queue->thread;
	thread->name = "QUEUE";
	thread->wake_handker = NULL;
	thread->handler = cavan_data_queue_thread_handler;

	ret = cavan_thread_init(thread, queue);
	if (ret < 0)
	{
		pr_red_info("cavan_thread_init");
		goto out_double_link_deinit;
	}

	ret = cavan_data_pool_init(&queue->pool, offset, node_size, pool_size);
	if (ret < 0)
	{
		pr_red_info("cavan_data_pool_init");
		goto out_cavan_thread_deinit;
	}

	queue->private_data = data;

	return 0;

out_cavan_thread_deinit:
	cavan_thread_deinit(thread);
out_double_link_deinit:
	double_link_deinit(&queue->link);
	return ret;
}

void cavan_data_queue_deinit(struct cavan_data_queue *queue)
{
	cavan_data_pool_deinit(&queue->pool);
	cavan_thread_deinit(&queue->thread);
	double_link_deinit(&queue->link);
}
