#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Sat Nov 24 18:07:06 CST 2012
 */

#include <time.h>
#include <cavan.h>
#include <cavan/thread.h>

struct cavan_timer
{
	struct timespec time;
	void *private_data;

	struct cavan_timer *next;
	struct cavan_timer *prev;

	void (*handler)(struct cavan_timer *timer);
};

struct cavan_timer_service
{
	struct cavan_thread thread;

	pthread_mutex_t lock;
	struct cavan_timer head_node;
};

int cavan_timespec_cmp(const struct timespec *t1, const struct timespec *t2);
int cavan_timespec_diff(const struct timespec *t1, const struct timespec *t2);
int cavan_real_timespec_diff(const struct timespec *time);
void cavan_timer_set_timespec(struct timespec *time, u32 timeout);

bool cavan_timer_remove_node(struct cavan_timer_service *service, struct cavan_timer *node);
int cavan_timer_add_node(struct cavan_timer_service *service, struct cavan_timer *node, u32 timeout);
int cavan_timer_service_start(struct cavan_timer_service *service);
int cavan_timer_service_stop(struct cavan_timer_service *service);
