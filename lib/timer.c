/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Sat Nov 24 18:07:06 CST 2012
 */

#include <cavan.h>
#include <cavan/math.h>
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
	long msec;

	msec = time->tv_nsec / 1000000UL + timeout;
	time->tv_sec += msec / 1000UL;
	time->tv_nsec = (msec % 1000UL) * 1000000UL;
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
		pr_red_info("timer->handler == NULL");

		return -EINVAL;
	}

	pthread_mutex_lock(&service->lock);

	double_link_remove(&service->link, &timer->node);
	cavan_timer_insert_base(service, timer, timeout);

	pthread_mutex_unlock(&service->lock);

	return 0;
}

void cavan_timer_remove(struct cavan_timer_service *service, struct cavan_timer *timer)
{
	pthread_mutex_lock(&service->lock);

	double_link_remove(&service->link, &timer->node);
	cavan_thread_resume(&service->thread);

	pthread_mutex_unlock(&service->lock);
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
			service->timer_waiting = timer;

			pthread_mutex_unlock(&service->lock);
			cavan_thread_msleep(thread, delay);
			pthread_mutex_lock(&service->lock);

			service->timer_waiting = NULL;
			pthread_mutex_unlock(&service->lock);
		}
		else
		{
			double_link_remove(link, node);
			service->timer_running = timer;

			pthread_mutex_unlock(&service->lock);
			delay = timer->handler(timer, timer->private_data);
			pthread_mutex_lock(&service->lock);

			service->run_count++;
			service->timer_running = NULL;
			service->timer_last_run = timer;

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

	service->run_count++;
	service->timer_waiting = NULL;
	service->timer_running = NULL;
	service->timer_last_run = NULL;

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

// ============================================================

void cavan_flasher_update_delay(struct cavan_flasher *flasher)
{
	u32 delay = 0;
	struct cavan_flasher_node *node;

	cavan_lock_acquire(&flasher->lock);

	for (node = flasher->head; node; node = node->next)
	{
		if (!node->enable)
		{
			continue;
		}

		if (delay > 0)
		{
			delay = math_get_greatest_common_divisor_single(delay, node->delay);
		}
		else
		{
			delay = node->delay;
		}
	}

	if (delay > 0)
	{
		for (node = flasher->head; node; node = node->next)
		{
			u32 max;

			if (!node->enable)
			{
				continue;
			}

			max = node->count_max;
			node->count_max = node->delay / delay;

			if (max > 0)
			{
				node->count = node->count * node->count_max / max;
			}
			else
			{
				node->count = 0;
			}
		}
	}

	flasher->delay = delay;

	cavan_lock_release(&flasher->lock);

	cavan_thread_resume(&flasher->thread);
}

void cavan_flasher_add_node(struct cavan_flasher *flasher, struct cavan_flasher_node *node)
{
	struct cavan_flasher_node **prev;

	node->count = 0;
	node->count_max = 0;
	node->bright = false;
	node->next = NULL;

	cavan_lock_acquire(&flasher->lock);

	for (prev = &flasher->head; *prev; prev = &(*prev)->next);

	*prev = node;

	cavan_lock_release(&flasher->lock);

	cavan_flasher_update_delay(flasher);
}

static int cavan_flasher_thread_handler(struct cavan_thread *thread, void *data)
{
	struct cavan_flasher *flasher = data;

	if (flasher->delay > 0)
	{
		struct cavan_flasher_node *node;

		if (cavan_thread_msleep(thread, flasher->delay) == 0)
		{
			return 0;
		}

		cavan_lock_acquire(&flasher->lock);

		for (node = flasher->head; node; node = node->next)
		{
			if (node->enable)
			{
				if (node->count > 1)
				{
					node->count--;
				}
				else
				{
					node->count = node->count_max;
					node->bright = !node->bright;
					node->handler(node);
				}
			}
		}

		cavan_lock_release(&flasher->lock);
	}
	else
	{
		cavan_thread_suspend(thread);
	}

	return 0;
}

int cavan_flasher_run(struct cavan_flasher *flasher)
{
	struct cavan_thread *thread = &flasher->thread;

	cavan_lock_init(&flasher->lock, FALSE);

	flasher->head = NULL;
	flasher->delay = 0;

	thread->name = "FLASHER";
	thread->handler = cavan_flasher_thread_handler;
	thread->wake_handker = NULL;

	return cavan_thread_run(thread, flasher);
}
