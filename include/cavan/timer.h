#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Sat Nov 24 18:07:06 CST 2012
 */

#include <cavan.h>
#include <cavan/list.h>
#include <cavan/time.h>
#include <cavan/thread.h>

__BEGIN_DECLS;

struct cavan_timer {
	struct timespec time;
	void *private_data;

	struct double_link_node node;

	int (*handler)(struct cavan_timer *timer, void *data);
};

struct cavan_timer_service {
	cavan_thread_t thread;

	u32 run_count;
	struct cavan_timer *timer_waiting;
	struct cavan_timer *timer_running;
	struct cavan_timer *timer_last_run;

	pthread_mutex_t lock;
	struct double_link link;
};

struct cavan_cursor {
	struct cavan_timer timer;

	int period;
	bool visual;
	void *private_data;
	struct cavan_timer_service *service;

	void (*set_visual)(struct cavan_cursor *cursor, bool enable, void *data);
};

struct cavan_flasher_node {
	const char *name;

	u32 delay;
	u32 count;
	u32 count_max;
	bool bright;
	bool enable;

	struct cavan_flasher_node *next;

	void (*handler)(struct cavan_flasher_node *node);
};

struct cavan_flasher {
	cavan_thread_t thread;
	cavan_lock_t lock;
	u32 delay;
	struct cavan_flasher_node *head;
};

int clock_gettime_safe(clockid_t clk, struct timespec *spec);
u64 clock_gettime_ns(clockid_t clk);
u64 clock_gettime_us(clockid_t clk);
u64 clock_gettime_ms(clockid_t clk);
u32 clock_gettime_ss(clockid_t clk);

int cavan_timespec_cmp(const struct timespec *t1, const struct timespec *t2);
s64 cavan_timespec_sub_ms(const struct timespec *t1, const struct timespec *t2);
s64 cavan_timespec_sub_us(const struct timespec *t1, const struct timespec *t2);
s64 cavan_timespec_sub_ns(const struct timespec *t1, const struct timespec *t2);
s64 cavan_real_timespec_diff_ms(const struct timespec *time);
void cavan_timer_timespec_add_ms(struct timespec *time, long msec);
int cavan_timer_set_timespec_ms(struct timespec *time, long msec);
int cavan_timer_set_timespec_ss(struct timespec *time, long sec);

int cavan_timer_insert(struct cavan_timer_service *service, struct cavan_timer *node, u32 timeout);
void cavan_timer_remove(struct cavan_timer_service *service, struct cavan_timer *timer);
int cavan_timer_service_start(struct cavan_timer_service *service);
int cavan_timer_service_stop(struct cavan_timer_service *service);

int cavan_cursor_init(struct cavan_cursor *cursor, struct cavan_timer_service *service);

void cavan_flasher_update_delay(struct cavan_flasher *flasher);
void cavan_flasher_add_node(struct cavan_flasher *flasher, struct cavan_flasher_node *node);
int cavan_flasher_run(struct cavan_flasher *flasher);

static inline void cavan_timer_init(struct cavan_timer *timer, void *data)
{
	timer->private_data = data;
	timer->handler = NULL;
	double_link_node_init(&timer->node);
}

static inline int cavan_cursor_start(struct cavan_cursor *cursor, void *data)
{
	if (cursor->set_visual == NULL) {
		return -EINVAL;
	}

	cursor->visual = false;

	if (data) {
		cursor->private_data = data;
	}

	return cavan_timer_insert(cursor->service, &cursor->timer, 0);
}

static inline void cavan_cursor_stop(struct cavan_cursor *cursor)
{
	cavan_timer_remove(cursor->service, &cursor->timer);

	if (cursor->set_visual) {
		cursor->set_visual(cursor, false, cursor->private_data);
	}
}

static inline u64 cavan_timespec_nseconds(const struct timespec *time)
{
	return (u64) time->tv_sec * 1000000000ul + time->tv_nsec;
}

static inline u64 cavan_timespec_useconds(const struct timespec *time)
{
	return (u64) time->tv_sec * 1000000ul + time->tv_nsec / 1000ul;
}

static inline u64 cavan_timespec_mseconds(const struct timespec *time)
{
	return (u64) time->tv_sec * 1000ul + time->tv_nsec / 1000000ul;
}

static inline int clock_gettime_mono(struct timespec *time)
{
	return clock_gettime_safe(CLOCK_MONOTONIC, time);
}

static inline u64 clock_gettime_mono_ns(void)
{
	return clock_gettime_ns(CLOCK_MONOTONIC);
}

static inline u64 clock_gettime_mono_us(void)
{
	return clock_gettime_us(CLOCK_MONOTONIC);
}

static inline u64 clock_gettime_mono_ms(void)
{
	return clock_gettime_ms(CLOCK_MONOTONIC);
}

static inline u32 clock_gettime_mono_ss(void)
{
	return clock_gettime_ss(CLOCK_MONOTONIC);
}

static inline int clock_gettime_real(struct timespec *time)
{
	return clock_gettime_safe(CLOCK_REALTIME, time);
}

static inline u64 clock_gettime_real_ns(void)
{
	return clock_gettime_ns(CLOCK_REALTIME);
}

static inline u64 clock_gettime_real_us(void)
{
	return clock_gettime_us(CLOCK_REALTIME);
}

static inline u64 clock_gettime_real_ms(void)
{
	return clock_gettime_ms(CLOCK_REALTIME);
}

static inline u32 clock_gettime_real_ss(void)
{
	return clock_gettime_ss(CLOCK_REALTIME);
}

__END_DECLS;
