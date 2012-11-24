#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Sat Nov 24 18:07:06 CST 2012
 */

#include <cavan.h>
#include <time.h>
#include <semaphore.h>
#include <pthread.h>

enum cavan_timer_state
{
	CAVAN_TIMER_STATE_RUNNING,
	CAVAN_TIMER_STATE_STOPPING,
	CAVAN_TIMER_STATE_STOPPED
};

struct cavan_timer
{
	struct timespec time;
	void *private_data;

	void (*handler)(struct cavan_timer *timer, void *data);

	struct cavan_timer *next;
	struct cavan_timer *prev;
};

struct cavan_timer_service
{
	sem_t sem;
	pthread_t thread;
	pthread_mutex_t lock;
	pthread_cond_t cond;

	enum cavan_timer_state state;
	struct cavan_timer head_node;
};

int cavan_time_diff(const struct timespec *t1, const struct timespec *t2);
void cavan_timer_set_time(struct timespec *time, u32 timeout);

bool cavan_timer_exists(struct cavan_timer *head, struct cavan_timer *node);
void cavan_timer_remove_node(struct cavan_timer *node);
void cavan_timer_add_node_base(struct cavan_timer *head, struct cavan_timer *node);
int cavan_timer_add_node(struct cavan_timer_service *service, struct cavan_timer *node, u32 timeout);
int cavan_timer_service_start(struct cavan_timer_service *service);
int cavan_timer_service_stop(struct cavan_timer_service *service);
