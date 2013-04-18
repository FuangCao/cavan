#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Mon Apr 16 10:03:27 CST 2012
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
	pthread_t id;
	const char *name;
	pthread_mutex_t lock;
	cavan_thread_state_t state;

	void *private_data;

	int (*wait_handler)(struct cavan_thread *thread, void *data);
	int (*wake_handker)(struct cavan_thread *thread, void *data);
	int (*handler)(struct cavan_thread *thread, void *data);
};

struct cavan_service_description
{
	const char *name;
	int daemon_count;
	int as_daemon;
	int show_verbose;
	int super_permission;
	cavan_shared_data_t data;
	pthread_t *threads;
	pthread_mutex_t mutex_lock;
	int (*handler)(struct cavan_service_description *service, int index, cavan_shared_data_t data);
};

struct cavan_daemon_description
{
	const char *pidfile;
	const char *command;
	int verbose;
	int as_daemon;
	int super_permission;
};

int cavan_thread_init(struct cavan_thread *thread, void *data);
void cavan_thread_deinit(struct cavan_thread *thread);
int cavan_thread_start(struct cavan_thread *thread);
void cavan_thread_stop(struct cavan_thread *thread);
void cavan_thread_suspend(struct cavan_thread *thread);
void cavan_thread_resume(struct cavan_thread *thread);

int cavan_service_run(struct cavan_service_description *desc);
int cavan_service_stop(struct cavan_service_description *desc);
int cavan_daemon_run(struct cavan_daemon_description *desc);
int cavan_daemon_stop(struct cavan_daemon_description *desc);
