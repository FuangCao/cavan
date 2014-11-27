/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Sat Nov 24 18:07:06 CST 2012
 */

#include <cavan.h>
#include <cavan/timer.h>

u64 clock_gettime_ns(clockid_t clk)
{
	int ret;
	struct timespec time;

	ret = clock_gettime(clk, &time);
	if (ret < 0)
	{
		pr_error_info("clock_gettime");
		return ret;
	}

	return (u64) time.tv_sec * 1000 * 1000 * 1000 + time.tv_nsec;
}

u64 clock_gettime_us(clockid_t clk)
{
	int ret;
	struct timespec time;

	ret = clock_gettime(clk, &time);
	if (ret < 0)
	{
		pr_error_info("clock_gettime");
		return ret;
	}

	return (u64) time.tv_sec * 1000 * 1000 + time.tv_nsec / 1000;
}

u64 clock_gettime_ms(clockid_t clk)
{
	int ret;
	struct timespec time;

	ret = clock_gettime(clk, &time);
	if (ret < 0)
	{
		pr_error_info("clock_gettime");
		return ret;
	}

	return (u64) time.tv_sec * 1000 + time.tv_nsec / (1000 * 1000);
}

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

s64 cavan_timespec_sub_ms(const struct timespec *t1, const struct timespec *t2)
{
	return ((s64) (t1->tv_sec - t2->tv_sec)) * 1000 + (t1->tv_nsec - t2->tv_nsec) / 1000000;
}

s64 cavan_timespec_sub_us(const struct timespec *t1, const struct timespec *t2)
{
	return ((s64) (t1->tv_sec - t2->tv_sec)) * 1000000 + (t1->tv_nsec - t2->tv_nsec) / 1000;
}

s64 cavan_timespec_sub_ns(const struct timespec *t1, const struct timespec *t2)
{
	return ((s64) (t1->tv_sec - t2->tv_sec)) * 1000000000 + (t1->tv_nsec - t2->tv_nsec);
}

s64 cavan_real_timespec_diff(const struct timespec *time)
{
	struct timespec curr_time;

	clock_gettime(CLOCK_REALTIME, &curr_time);

	return cavan_timespec_sub_ms(time, &curr_time);
}

void cavan_timer_timespec_add(struct timespec *time, u32 timeout)
{
	long tmp;

	tmp = time->tv_nsec / 1000000 + timeout;
	time->tv_sec += tmp / 1000;
	time->tv_nsec = (tmp % 1000) * 1000000;
}

void cavan_timer_set_timespec(struct timespec *time, u32 timeout)
{
	clock_gettime(CLOCK_REALTIME, time);
	cavan_timer_timespec_add(time, timeout);
}

static bool cavan_timer_match_later(struct double_link *link, struct double_link_node *node, void *data)
{
	struct timespec *time = data;
	struct cavan_timer *timer = double_link_get_container(link, node);

	return cavan_timespec_cmp(&timer->time, time) > 0;
}

static int cavan_timer_insert_base(struct cavan_timer_service *service, struct cavan_timer *timer, u32 timeout)
{
	cavan_timer_set_timespec(&timer->time, timeout);
	double_link_cond_insert_append(&service->link, &timer->node, &timer->time, cavan_timer_match_later);
	cavan_thread_resume(&service->thread);

	return 0;
}

int cavan_timer_insert(struct cavan_timer_service *service, struct cavan_timer *timer, u32 timeout)
{
	if (timer->handler == NULL)
	{
		pr_red_info("p->handler == NULL");

		return -EINVAL;
	}

	pthread_mutex_lock(&service->lock);

	double_link_remove(&service->link, &timer->node);
	cavan_timer_insert_base(service, timer, timeout);

	pthread_mutex_unlock(&service->lock);

	return 0;
}

static int cavan_timer_service_handler(struct cavan_thread *thread, void *data)
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

		delay = cavan_real_timespec_diff(&timer->time);
		if (delay > 0)
		{
			pthread_mutex_unlock(&service->lock);
			cavan_thread_msleep(thread, delay);
		}
		else
		{
			double_link_remove(link, node);
			delay = timer->handler(timer, timer->private_data);
			if (delay > 0)
			{
				cavan_timer_insert_base(service, timer, delay);
			}

			pthread_mutex_unlock(&service->lock);
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
	thread->wake_handker = NULL;
	thread->handler = cavan_timer_service_handler;

	ret = pthread_mutex_init(&service->lock, NULL);
	if (ret < 0)
	{
		pr_error_info("pthread_mutex_init");
		return ret;
	}

	ret = cavan_thread_init(thread, service);
	if (ret < 0)
	{
		pr_red_info("cavan_thread_init");
		goto out_pthread_mutex_destroy;
	}

	ret = double_link_init(&service->link, MEMBER_OFFSET(struct cavan_timer, node));
	if (ret < 0)
	{
		pr_red_info("double_link_init");
		goto out_cavan_thread_deinit;
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
out_cavan_thread_deinit:
	cavan_thread_deinit(thread);
out_pthread_mutex_destroy:
	pthread_mutex_destroy(&service->lock);
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

// ============================================================

static int cavan_cursor_timer_handler(struct cavan_timer *timer, void *data)
{
	struct cavan_cursor *cursor = (struct cavan_cursor *) timer;

	cursor->visual = !cursor->visual;
	cursor->set_visual(cursor, cursor->visual, cursor->private_data);

	return cursor->period;
}

int cavan_cursor_init(struct cavan_cursor *cursor, struct cavan_timer_service *service)
{
	cavan_timer_init(&cursor->timer, cursor);
	cursor->timer.handler = cavan_cursor_timer_handler;
	cursor->service = service;
	cursor->set_visual = NULL;

	return 0;
}
