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

bool cavan_timer_remove(struct cavan_timer_service *service, struct cavan_timer *timer)
{
	return double_link_remove(&service->link, &timer->node);
}

static bool cavan_timer_match_later(struct double_link *link, struct double_link_node *node, void *data)
{
	struct timespec *time = data;
	struct cavan_timer *timer = double_link_get_container(link, node);

	return cavan_timespec_cmp(&timer->time, time) > 0;
}

static void cavan_timer_insert_base(struct double_link *link, struct cavan_timer *timer)
{
	struct double_link_node *next;

	double_link_remove(link, &timer->node);

	next = double_link_find(link, &timer->time, cavan_timer_match_later);
	if (next)
	{
		double_link_insert(link, next->prev, &timer->node);
	}
	else
	{
		double_link_append(link, &timer->node);
	}
}

int cavan_timer_insert(struct cavan_timer_service *service, struct cavan_timer *timer, u32 timeout)
{
	if (timer->handler == NULL)
	{
		pr_red_info("p->handler == NULL");

		return -EINVAL;
	}

	pthread_mutex_lock(&service->lock);

	cavan_timer_set_timespec(&timer->time, timeout);
	cavan_timer_insert_base(&service->link, timer);
	cavan_thread_resume(&service->thread);

	pthread_mutex_unlock(&service->lock);

	return 0;
}

static int cavan_timer_service_handler(struct cavan_thread *thread, u32 event, void *data)
{
	struct double_link_node *node;
	struct cavan_timer_service *service = data;
	struct double_link *link = &service->link;

	pthread_mutex_lock(&service->lock);

	node = double_link_get_first_node(link);
	if (node)
	{
		int delay;
		struct cavan_timer *timer = double_link_get_container(link, node);

		pthread_mutex_unlock(&service->lock);

		delay = cavan_real_timespec_diff(&timer->time);
		if (delay > 0)
		{
			cavan_thread_msleep(thread, delay);
		}
		else
		{
			double_link_remove(link, node);
			timer->handler(timer);
		}
	}
	else
	{
		cavan_thread_suspend(thread);

		pthread_mutex_unlock(&service->lock);
	}

	return 0;
}

int cavan_timer_service_start(struct cavan_timer_service *service)
{
	int ret;
	struct cavan_thread *thread = &service->thread;

	thread->name = "TIMER";
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

	ret = double_link_init(&service->link, MEMBER_OFFSET(struct cavan_timer, node));
	if (ret < 0)
	{
		pr_red_info("double_link_init");
		goto out_mutex_destroy;
	}

	ret = cavan_thread_start(thread);
	if (ret < 0)
	{
		pr_red_info("cavan_thread_start");
		goto out_double_link_deinit;
	}

	return 0;

out_double_link_deinit:
	double_link_deinit(&service->link);
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

	double_link_deinit(&service->link);
	cavan_thread_deinit(thread);
	pthread_mutex_destroy(&service->lock);

	return 0;
}
