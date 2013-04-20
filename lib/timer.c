/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Sat Nov 24 18:07:06 CST 2012
 */

#include <cavan.h>
#include <cavan/timer.h>

int cavan_timespec_cmp(const struct timespec *t1, const struct timespec *t2)
{
	if (t1->tv_sec > t2->tv_sec)
	{
		return 1;
	}

	if (t1->tv_sec < t2->tv_sec)
	{
		return -1;
	}

	if (t1->tv_nsec > t2->tv_nsec)
	{
		return 1;
	}

	if (t1->tv_nsec < t2->tv_nsec)
	{
		return -1;
	}

	return 0;
}

int cavan_timespec_diff(const struct timespec *t1, const struct timespec *t2)
{
	return (t1->tv_sec - t2->tv_sec) * 1000 + (t1->tv_nsec - t2->tv_nsec) / 1000000;
}

int cavan_real_timespec_diff(const struct timespec *time)
{
	struct timespec curr_time;

	clock_gettime(CLOCK_REALTIME, &curr_time);

	return cavan_timespec_diff(time, &curr_time);
}

void cavan_timer_set_timespec(struct timespec *time, u32 timeout)
{
	long tmp;

	clock_gettime(CLOCK_REALTIME, time);

	tmp = time->tv_nsec / 1000000 + timeout;
	time->tv_sec += tmp / 1000;
	time->tv_nsec = (tmp % 1000) * 1000000;
}

static bool cavan_timer_exists(struct cavan_timer *head, struct cavan_timer *node)
{
	struct cavan_timer *p;

	for (p = head->next; p != head; p = p->next)
	{
		if (p == node)
		{
			return true;
		}
	}

	return false;
}

static void cavan_timer_remove_node_base(struct cavan_timer *node)
{
	node->prev->next = node->next;
	node->next->prev = node->prev;
	node->prev = node->next = node;
}

bool cavan_timer_remove_node(struct cavan_timer_service *service, struct cavan_timer *node)
{
	pthread_mutex_lock(&service->lock);

	if (cavan_timer_exists(&service->head_node, node) == false)
	{
		pthread_mutex_unlock(&service->lock);
		return false;
	}

	cavan_timer_remove_node_base(node);

	pthread_mutex_unlock(&service->lock);

	return true;
}

static void cavan_timer_add_node_base(struct cavan_timer *head, struct cavan_timer *node)
{
	struct cavan_timer *p;

	for (p = head->next; p != head; p = p->next)
	{
		if (cavan_timespec_cmp(&p->time, &node->time) > 0)
		{
			break;
		}
	}

	p->prev->next = node;
	node->next = p;

	node->prev = p->prev;
	p->prev = node;
}

int cavan_timer_add_node(struct cavan_timer_service *service, struct cavan_timer *node, u32 timeout)
{
	struct cavan_timer *head;

	if (node->handler == NULL)
	{
		pr_red_info("p->handler == NULL");

		return -EINVAL;
	}

	cavan_timer_set_timespec(&node->time, timeout);

	pthread_mutex_lock(&service->lock);

	head = &service->head_node;
	if (cavan_timer_exists(head, node))
	{
		cavan_timer_remove_node_base(node);
	}

	cavan_timer_add_node_base(head, node);
	cavan_thread_resume(&service->thread);

	pthread_mutex_unlock(&service->lock);

	return 0;
}

static int cavan_timer_service_handler(struct cavan_thread *thread, u32 event, void *data)
{
	struct cavan_timer *timer;
	struct cavan_timer_service *service = data;
	struct cavan_timer *head = &service->head_node;

	pthread_mutex_lock(&service->lock);

	timer = head->next;
	if (timer != head)
	{
		int diff = cavan_real_timespec_diff(&timer->time);

		pthread_mutex_unlock(&service->lock);

		if (diff > 0)
		{
			cavan_thread_msleep(thread, diff);
		}
		else
		{
			timer->handler(timer);
			cavan_timer_remove_node(service, timer);
		}
	}
	else
	{
		pthread_mutex_unlock(&service->lock);
		cavan_thread_suspend(thread);
	}

	return 0;
}

static int cavan_timer_service_wait_handler(struct cavan_thread *thread, u32 *event, void *data)
{
	return 0;
}

static int cavan_timer_service_wake_handler(struct cavan_thread *thread, u32 event, void *data)
{
	return 0;
}

int cavan_timer_service_start(struct cavan_timer_service *service)
{
	int ret;
	struct cavan_thread *thread = &service->thread;

	thread->name = "TIMER";
	thread->wait_handler = cavan_timer_service_wait_handler;
	thread->wake_handker = cavan_timer_service_wake_handler;
	thread->handler = cavan_timer_service_handler;

	ret = cavan_thread_init(thread, service);
	if (ret < 0)
	{
		pr_red_info("cavan_thread_init");
		return ret;
	}

	ret = pthread_mutex_init(&service->lock, NULL);
	if (ret < 0)
	{
		pr_error_info("pthread_mutex_init");
		goto out_cavan_thread_deinit;
	}

	service->head_node.next = &service->head_node;
	service->head_node.prev = &service->head_node;

	ret = cavan_thread_start(thread);
	if (ret < 0)
	{
		pr_red_info("cavan_thread_start");
		goto out_mutex_destroy;
	}

	return 0;

out_mutex_destroy:
	pthread_mutex_destroy(&service->lock);
out_cavan_thread_deinit:
	cavan_thread_deinit(thread);
	return ret;
}

int cavan_timer_service_stop(struct cavan_timer_service *service)
{
	struct cavan_thread *thread = &service->thread;

	pthread_mutex_lock(&service->lock);
	cavan_thread_stop(thread);
	pthread_mutex_unlock(&service->lock);

	cavan_thread_deinit(thread);
	pthread_mutex_destroy(&service->lock);

	return 0;
}
