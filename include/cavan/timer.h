#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Sat Nov 24 18:07:06 CST 2012
 */

#include <cavan.h>
#include <cavan/list.h>
#include <cavan/thread.h>
#include <time.h>

struct cavan_timer
{
	struct timespec time;
	void *private_data;

	struct double_link_node node;

	void (*handler)(struct cavan_timer *timer, void *data);
};

struct cavan_timer_service
{
	struct cavan_thread thread;

	pthread_mutex_t lock;
	struct double_link link;
};

int cavan_timespec_cmp(const struct timespec *t1, const struct timespec *t2);
int cavan_timespec_diff(const struct timespec *t1, const struct timespec *t2);
int cavan_real_timespec_diff(const struct timespec *time);
void cavan_timer_set_timespec(struct timespec *time, u32 timeout);

int cavan_timer_insert(struct cavan_timer_service *service, struct cavan_timer *node, u32 timeout);
int cavan_timer_service_start(struct cavan_timer_service *service);
int cavan_timer_service_stop(struct cavan_timer_service *service);

static inline void cavan_timer_init(struct cavan_timer *timer, void *data, void (*destroy)(void *pointer))
{
	timer->private_data = data;
	timer->handler = NULL;
	double_link_node_init(&timer->node, destroy);
}

static inline void cavan_timer_remove(struct cavan_timer_service *service, struct cavan_timer *timer)
{
	double_link_remove(&service->link, &timer->node);
}
