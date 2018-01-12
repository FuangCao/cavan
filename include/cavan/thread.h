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
int cavan_thread_send_event(struct cavan_thread *thread, u32 event);
int cavan_thread_recv_event(struct cavan_thread *thread, u32 *event);
int cavan_thread_recv_event_timeout(struct cavan_thread *thread, u32 *event, u32 msec);
int cavan_thread_wait_event(struct cavan_thread *thread, u32 msec);
int cavan_thread_init(struct cavan_thread *thread, void *data, int flags);
void cavan_thread_deinit(struct cavan_thread *thread);
int cavan_thread_start(struct cavan_thread *thread);
void cavan_thread_stop(struct cavan_thread *thread);
int cavan_thread_run(struct cavan_thread *thread, void *data, int flags);
int cavan_thread_run_self(struct cavan_thread *thread, void *data, int flags);
void cavan_thread_suspend(struct cavan_thread *thread);
void cavan_thread_resume(struct cavan_thread *thread);
int cavan_thread_sleep_until(struct cavan_thread *thread, struct timespec *time);
int cavan_thread_msleep(struct cavan_thread *thread, u32 msec);
int cavan_thread_ssleep(struct cavan_thread *thread, u32 sec);

int cavan_thread_epoll_add(struct cavan_thread *thread, int fd, u32 events);
int cavan_thread_epoll_remove(struct cavan_thread *thread, int fd);
int cavan_thread_epoll_modify(struct cavan_thread *thread, int fd, u32 events);
int cavan_thread_epoll_wait(struct cavan_thread *thread, struct epoll_event *events, int count, int timeout);
int cavan_thread_epoll_wait_event(struct cavan_thread *thread, int timeout);

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

static inline int cavan_thread_join(struct cavan_thread *thread)
{
	return cavan_pthread_join(thread->id);
}

static inline void cavan_thread_set_state(struct cavan_thread *thread, cavan_thread_state_t state)
{
	pthread_mutex_lock(&thread->lock);
	thread->state = state;
	pthread_mutex_unlock(&thread->lock);
}

static inline void cavan_thread_should_stop(struct cavan_thread *thread)
{
	cavan_thread_set_state(thread, CAVAN_THREAD_STATE_STOPPPING);
}

static inline int cavan_thread_wake_handler_send_event(struct cavan_thread *thread, void *data)
{
	return cavan_thread_send_event(thread, 0);
}

static inline int cavan_thread_wake_handler_none(struct cavan_thread *thread, void *data)
{
	return 0;
}

static inline int cavan_thread_wake_handler_resume(struct cavan_thread *thread, void *data)
{
	cavan_thread_resume(thread);

	return 0;
}

static inline int cavan_thread_wake_handler_empty(struct cavan_thread *thread, void *data)
{
	return 0;
}

static inline int cavan_thread_wakeup(struct cavan_thread *thread)
{
	return thread->wake_handker(thread, thread->private_data);
}

static inline void cavan_thread_exit(struct cavan_thread *thread)
{
	cavan_thread_stop(thread);
	cavan_thread_deinit(thread);
}

__END_DECLS;
