#pragma once

#include <linux/mutex.h>

enum hua_input_thread_state
{
	HUA_INPUT_THREAD_STATE_CREATED,
	HUA_INPUT_THREAD_STATE_RUNNING,
	HUA_INPUT_THREAD_STATE_SUSPEND,
	HUA_INPUT_THREAD_STATE_STOPPING,
	HUA_INPUT_THREAD_STATE_STOPPED
};

struct hua_input_thread
{
	const char *name;
	void *private_data;
	enum hua_input_thread_state state;

	int priority;
	struct mutex lock;
	struct task_struct *task;

	void (*stop)(struct hua_input_thread *thread);
	void (*prepare)(struct hua_input_thread *thread, bool enable);
	void (*event_handle)(struct hua_input_thread *thread);
	void (*wait_for_event)(struct hua_input_thread *thread);
};

void hua_input_thread_stop(struct hua_input_thread *thread);
int hua_input_thread_set_state(struct hua_input_thread *thread, enum hua_input_thread_state state);
int hua_input_thread_init(struct hua_input_thread *thread, const char *format, ...);
void hua_input_thread_destroy(struct hua_input_thread *thread);

static inline int hua_input_thread_suspend(struct hua_input_thread *thread)
{
	return hua_input_thread_set_state(thread, HUA_INPUT_THREAD_STATE_SUSPEND);
}

static inline int hua_input_thread_resume(struct hua_input_thread *thread)
{
	return hua_input_thread_set_state(thread, HUA_INPUT_THREAD_STATE_RUNNING);
}
