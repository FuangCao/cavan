#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Sat Apr 20 14:01:25 CST 2013
 */

#include <cavan.h>
#include <cavan/lock.h>
#include <sys/epoll.h>

#define CAVAN_THREADF_PIPE_WAKEUP		(1 << 0)

__BEGIN_DECLS;

typedef enum cavan_thread_state {
	CAVAN_THREAD_STATE_NONE,
	CAVAN_THREAD_STATE_IDLE,
	CAVAN_THREAD_STATE_RUNNING,
	CAVAN_THREAD_STATE_SLEEP,
	CAVAN_THREAD_STATE_SUSPEND,
	CAVAN_THREAD_STATE_WAIT_EVENT,
	CAVAN_THREAD_STATE_STOPPPING,
	CAVAN_THREAD_STATE_STOPPED
} cavan_thread_state_t;

typedef struct cavan_thread {
	const char *name;

	int flags;
	bool pending;
	pthread_t id;

	union {
		int pipefd[2];
		struct {
			int rd_event_fd;
			int wr_event_fd;
		};
	};

	int epoll_fd;

	pthread_cond_t cond;
	pthread_mutex_t lock;
	cavan_thread_state_t state;

	void *private_data;

	int (*wake_handker)(struct cavan_thread *thread, void *data);
	int (*handler)(struct cavan_thread *thread, void *data);
} cavan_thread_t;

int cavan_pthread_create(pthread_t *pthread, void *(*handler)(void *), void *data, bool joinable);
int cavan_thread_send_event(cavan_thread_t *thread, u32 event);
int cavan_thread_recv_event(cavan_thread_t *thread, u32 *event);
int cavan_thread_recv_event_timeout(cavan_thread_t *thread, u32 *event, u32 msec);
int cavan_thread_wait_event(cavan_thread_t *thread, u32 msec);
int cavan_thread_init(cavan_thread_t *thread, void *data, int flags);
void cavan_thread_deinit(cavan_thread_t *thread);
int cavan_thread_start(cavan_thread_t *thread);
void cavan_thread_stop(cavan_thread_t *thread);
int cavan_thread_run(cavan_thread_t *thread, void *data, int flags);
int cavan_thread_run_self(cavan_thread_t *thread, void *data, int flags);
void cavan_thread_suspend(cavan_thread_t *thread);
void cavan_thread_resume(cavan_thread_t *thread);
int cavan_thread_sleep_until(cavan_thread_t *thread, struct timespec *time);
int cavan_thread_msleep(cavan_thread_t *thread, u32 msec);
int cavan_thread_ssleep(cavan_thread_t *thread, u32 sec);

int cavan_thread_epoll_add(cavan_thread_t *thread, int fd, u32 events);
int cavan_thread_epoll_remove(cavan_thread_t *thread, int fd);
int cavan_thread_epoll_modify(cavan_thread_t *thread, int fd, u32 events);
int cavan_thread_epoll_wait(cavan_thread_t *thread, struct epoll_event *events, int count, int timeout);
int cavan_thread_epoll_wait_event(cavan_thread_t *thread, int timeout);

static inline int cavan_pthread_run(void *(handler) (void *), void *data)
{
	return cavan_pthread_create(NULL, handler, data, false);
}

static inline int cavan_pthread_join(pthread_t thread)
{
	return pthread_join(thread, NULL);
}

static inline int cavan_pthread_kill(pthread_t thread)
{
	return pthread_kill(thread, SIGUSR1);
}

static inline int cavan_thread_join(cavan_thread_t *thread)
{
	return cavan_pthread_join(thread->id);
}

static inline void cavan_thread_set_state(cavan_thread_t *thread, cavan_thread_state_t state)
{
	pthread_mutex_lock(&thread->lock);
	thread->state = state;
	pthread_mutex_unlock(&thread->lock);
}

static inline void cavan_thread_should_stop(cavan_thread_t *thread)
{
	cavan_thread_set_state(thread, CAVAN_THREAD_STATE_STOPPPING);
}

static inline int cavan_thread_wake_handler_send_event(cavan_thread_t *thread, void *data)
{
	return cavan_thread_send_event(thread, 0);
}

static inline int cavan_thread_wake_handler_none(cavan_thread_t *thread, void *data)
{
	return 0;
}

static inline int cavan_thread_wake_handler_resume(cavan_thread_t *thread, void *data)
{
	cavan_thread_resume(thread);

	return 0;
}

static inline int cavan_thread_wake_handler_empty(cavan_thread_t *thread, void *data)
{
	return 0;
}

static inline int cavan_thread_wakeup(cavan_thread_t *thread)
{
	return thread->wake_handker(thread, thread->private_data);
}

static inline void cavan_thread_exit(cavan_thread_t *thread)
{
	cavan_thread_stop(thread);
	cavan_thread_deinit(thread);
}

__END_DECLS;
