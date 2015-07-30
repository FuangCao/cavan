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
		queue->handler(data, queue->private_data);
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

// ================================================================================

int cavan_mem_queue_init(struct cavan_mem_queue *queue, size_t size)
{
	queue->mem = malloc(size + 1);
	if (queue->mem == NULL)
	{
		pr_error_info("malloc");
		return -ENOMEM;
	}

	cavan_lock_init(&queue->lock, false);

	queue->head = queue->tail = queue->mem;
	queue->last = queue->mem + size;

	return 0;
}

void cavan_mem_queue_deinit(struct cavan_mem_queue *queue)
{
	if (queue->mem)
	{
		free(queue->mem);
		queue->mem = NULL;
	}

	cavan_lock_deinit(&queue->lock);
}

size_t cavan_mem_queue_inqueue_peek(struct cavan_mem_queue *queue, const void *buff, size_t size)
{
	size_t rlen, length;

	cavan_lock_acquire(&queue->lock);

	if (queue->tail < queue->head)
	{
		length = rlen = queue->head - queue->tail - 1;
	}
	else
	{
		if (queue->head > queue->mem)
		{
			rlen = queue->last - queue->tail + 1;
			length = rlen + (queue->head - queue->mem - 1);
		}
		else
		{
			length = rlen = queue->last - queue->tail;
		}
	}

	if (length > size)
	{
		length = size;
	}

	if (length > rlen)
	{
		size_t llen = length - rlen;

		if (buff)
		{
			memcpy(queue->tail, buff, rlen);
			memcpy(queue->mem, ADDR_ADD(buff, rlen), llen);
		}

		queue->tail_peek = queue->mem + llen;
	}
	else
	{
		if (buff)
		{
			memcpy(queue->tail, buff, length);
		}

		queue->tail_peek = queue->tail + length;
		if (queue->tail_peek > queue->last)
		{
			queue->tail_peek = queue->mem;
		}
	}

	cavan_lock_release(&queue->lock);

	return length;
}

void cavan_mem_queue_inqueue_commit(struct cavan_mem_queue *queue)
{
	cavan_lock_acquire(&queue->lock);
	queue->tail = queue->tail_peek;
	cavan_lock_release(&queue->lock);
}

size_t cavan_mem_queue_inqueue(struct cavan_mem_queue *queue, const void *buff, size_t size)
{
	size_t length;

	cavan_lock_acquire(&queue->lock);
	length = cavan_mem_queue_inqueue_peek(queue, buff, size);
	cavan_mem_queue_inqueue_commit(queue);
	cavan_lock_release(&queue->lock);

	return length;
}

size_t cavan_mem_queue_dequeue_peek(struct cavan_mem_queue *queue, void *buff, size_t size)
{
	size_t rlen, length;

	cavan_lock_acquire(&queue->lock);

	if (queue->head > queue->tail)
	{
		rlen = queue->last - queue->head + 1;
		length = rlen + (queue->tail - queue->mem);
	}
	else
	{
		length = rlen = queue->tail - queue->head;
	}

	if (length > size)
	{
		length = size;
	}

	if (length > rlen)
	{
		size_t llen = length - rlen;

		if (buff)
		{
			memcpy(buff, queue->head, rlen);
			memcpy(ADDR_ADD(buff, rlen), queue->mem, llen);
		}

		queue->head_peek = queue->mem + llen;
	}
	else
	{
		if (buff)
		{
			memcpy(buff, queue->head, length);
		}

		queue->head_peek = queue->head + length;
		if (queue->head_peek > queue->last)
		{
			queue->head_peek = queue->mem;
		}
	}

	cavan_lock_release(&queue->lock);

	return length;
}

void cavan_mem_queue_dequeue_commit(struct cavan_mem_queue *queue)
{
	cavan_lock_acquire(&queue->lock);
	queue->head = queue->head_peek;
	cavan_lock_release(&queue->lock);
}

size_t cavan_mem_queue_dequeue(struct cavan_mem_queue *queue, void *buff, size_t size)
{
	size_t length;

	cavan_lock_acquire(&queue->lock);
	length = cavan_mem_queue_dequeue_peek(queue, buff, size);
	cavan_mem_queue_dequeue_commit(queue);
	cavan_lock_release(&queue->lock);

	return length;
}

size_t cavan_mem_queue_get_used_size(struct cavan_mem_queue *queue)
{
	size_t length;

	cavan_lock_acquire(&queue->lock);

	if (queue->head > queue->tail)
	{
		length = (queue->last - queue->head) + (queue->tail - queue->mem) + 1;
	}
	else
	{
		length = queue->tail - queue->head;
	}

	cavan_lock_release(&queue->lock);

	return length;
}

size_t cavan_mem_queue_get_free_size(struct cavan_mem_queue *queue)
{
	size_t length;

	cavan_lock_acquire(&queue->lock);

	if (queue->tail < queue->head)
	{
		length = queue->head - queue->tail - 1;
	}
	else
	{
		length = (queue->last - queue->tail) + (queue->head - queue->mem);
	}

	cavan_lock_release(&queue->lock);

	return length;
}

bool cavan_mem_queue_is_empty(struct cavan_mem_queue *queue)
{
	bool res;

	cavan_lock_acquire(&queue->lock);
	res = (queue->head == queue->tail);
	cavan_lock_release(&queue->lock);

	return res;
}

bool cavan_mem_queue_has_data(struct cavan_mem_queue *queue)
{
	bool res;

	cavan_lock_acquire(&queue->lock);
	res = (queue->head != queue->tail);
	cavan_lock_release(&queue->lock);

	return res;
}
