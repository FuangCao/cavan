#pragma once

#include <linux/mutex.h>

enum cavan_input_thread_state
{
	CAVAN_INPUT_THREAD_STATE_CREATED,
	CAVAN_INPUT_THREAD_STATE_RUNNING,
	CAVAN_INPUT_THREAD_STATE_SUSPEND,
	CAVAN_INPUT_THREAD_STATE_STOPPING,
	CAVAN_INPUT_THREAD_STATE_STOPPED
};

struct cavan_input_thread
{
	const char *name;
	void *private_data;
	enum cavan_input_thread_state state;

	int priority;
	struct mutex lock;
	struct task_struct *task;

	void (*stop)(struct cavan_input_thread *thread);
	void (*prepare)(struct cavan_input_thread *thread, bool enable);
	int (*event_handle)(struct cavan_input_thread *thread);
	void (*error_handle)(struct cavan_input_thread *thread);
	void (*wait_for_event)(struct cavan_input_thread *thread);
};

void cavan_input_thread_stop(struct cavan_input_thread *thread);
int cavan_input_thread_set_state(struct cavan_input_thread *thread, enum cavan_input_thread_state state);
int cavan_input_thread_init(struct cavan_input_thread *thread, const char *format, ...);
void cavan_input_thread_destroy(struct cavan_input_thread *thread);

static inline int cavan_input_thread_suspend(struct cavan_input_thread *thread)
{
	return cavan_input_thread_set_state(thread, CAVAN_INPUT_THREAD_STATE_SUSPEND);
}

static inline int cavan_input_thread_resume(struct cavan_input_thread *thread)
{
	return cavan_input_thread_set_state(thread, CAVAN_INPUT_THREAD_STATE_RUNNING);
}
