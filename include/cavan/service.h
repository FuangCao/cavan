#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Mon Apr 16 10:03:27 CST 2012
 */

#include <cavan.h>
#include <cavan/list.h>

typedef enum cavan_service_state
{
	CAVAN_SERVICE_STATE_RUNNING,
	CAVAN_SERVICE_STATE_STOPPED,
	CAVAN_SERVICE_STATE_STOPPING,
} cavan_service_state_t;

struct cavan_service_description
{
	const char *name;
	int used_count;
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
	const char *logfile;
	const char *command;
	int verbose;
	int as_daemon;
	int super_permission;
};

struct cavan_dynamic_service
{
	const char *name;
	int min, max;
	int count, used;
	int as_daemon;
	int verbose;
	size_t conn_size;
	int super_permission;
	void *private_data;
	const char *logfile;
	pthread_cond_t cond;
	pthread_mutex_t lock;
	cavan_service_state_t state;

	int (*open_connect)(struct cavan_dynamic_service *service, void *conn);
	void (*close_connect)(struct cavan_dynamic_service *service, void *conn);
	int (*start)(struct cavan_dynamic_service *service);
	void (*stop)(struct cavan_dynamic_service *service);
	int (*run)(struct cavan_dynamic_service *service, void *conn);
};

void cavan_service_set_busy(struct cavan_service_description *desc, int index, bool busy);
int cavan_service_start(struct cavan_service_description *desc);
int cavan_service_main_loop(struct cavan_service_description *desc);
int cavan_service_run(struct cavan_service_description *desc);
int cavan_service_stop(struct cavan_service_description *desc);
int cavan_daemon_run(struct cavan_daemon_description *desc);
int cavan_daemon_stop(struct cavan_daemon_description *desc);

int cavan_dynamic_service_init(struct cavan_dynamic_service *service);
void cavan_dynamic_service_deinit(struct cavan_dynamic_service *service);
struct cavan_dynamic_service *cavan_dynamic_service_create(size_t size);
void cavan_dynamic_service_destroy(struct cavan_dynamic_service *service);
void cavan_dynamic_service_join(struct cavan_dynamic_service *service);
int cavan_dynamic_service_start(struct cavan_dynamic_service *service, bool sync);
int cavan_dynamic_service_run(struct cavan_dynamic_service *service);
int cavan_dynamic_service_stop(struct cavan_dynamic_service *service);

static inline void *cavan_dynamic_service_get_data(struct cavan_dynamic_service *service)
{
	return service->private_data;
}

static inline void cavan_dynamic_service_set_data(struct cavan_dynamic_service *service, void *data)
{
	service->private_data = data;
}
