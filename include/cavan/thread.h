#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Sat Apr 20 14:01:25 CST 2013
 */

#include <cavan.h>

typedef enum cavan_thread_state
{
	CAVAN_THREAD_STATE_NONE,
	CAVAN_THREAD_STATE_IDEL,
	CAVAN_THREAD_STATE_RUNNING,
	CAVAN_THREAD_STATE_SUSPEND,
	CAVAN_THREAD_STATE_STOPPPING,
	CAVAN_THREAD_STATE_STOPPED
} cavan_thread_state_t;

struct cavan_thread
{
	const char *name;

	pthread_t id;
	int pipefd[2];

	pthread_mutex_t lock;
	cavan_thread_state_t state;

	void *private_data;

	int (*wait_handler)(struct cavan_thread *thread, u32 *event, void *data);
	int (*wake_handker)(struct cavan_thread *thread, u32 event, void *data);
	int (*handler)(struct cavan_thread *thread, u32 event, void *data);
};

int cavan_thread_send_event(struct cavan_thread *thread, u32 event);
int cavan_thread_recv_event(struct cavan_thread *thread, u32 *event);
int cavan_thread_init(struct cavan_thread *thread, void *data);
void cavan_thread_deinit(struct cavan_thread *thread);
int cavan_thread_start(struct cavan_thread *thread);
void cavan_thread_stop(struct cavan_thread *thread);
void cavan_thread_suspend(struct cavan_thread *thread);
void cavan_thread_resume(struct cavan_thread *thread);

static inline int cavan_thread_join(struct cavan_thread *thread)
{
	return pthread_join(thread->id, NULL);
}
