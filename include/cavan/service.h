#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Mon Apr 16 10:03:27 CST 2012
 */

#include <cavan.h>
#include <cavan/list.h>
#include <cavan/thread.h>

typedef enum cavan_service_state {
	CAVAN_SERVICE_STATE_RUNNING,
	CAVAN_SERVICE_STATE_STOPPED,
	CAVAN_SERVICE_STATE_STOPPING,
} cavan_service_state_t;

struct cavan_epoll_service;

struct cavan_service_description {
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

struct cavan_daemon_description {
	const char *pidfile;
	const char *logfile;
	const char *command;
	int verbose;
	int as_daemon;
	int super_permission;
};

struct cavan_dynamic_service {
	const char *name;
	int epollfd;
	u32 index;
	u32 min, max;
	u32 count, used;
	bool as_daemon;
	bool verbose;
	size_t conn_size;
	int super_permission;
	void *private_data;
	const char *logfile;
	pthread_cond_t cond;
	pthread_mutex_t lock;
	const char *user, *group;
	cavan_service_state_t state;
	cavan_thread_t epoll_thread;
	bool stopping;

	void (*init_connect)(struct cavan_dynamic_service *service, void *conn_data);
	int (*open_connect)(struct cavan_dynamic_service *service, void *conn_data);
	bool (*close_connect)(struct cavan_dynamic_service *service, void *conn_data);
	int (*start)(struct cavan_dynamic_service *service);
	void (*stop)(struct cavan_dynamic_service *service);
	int (*keepalive)(struct cavan_dynamic_service *service);
	void (*epoll)(struct cavan_dynamic_service *service, void *conn_data);
	int (*run)(struct cavan_dynamic_service *service, void *conn_data);
};

struct cavan_epoll_pack {
	u16 offset;
	struct cavan_epoll_pack *next;

	union {
		u16 length;
		char data[0];
	};

	char body[0];
};

struct cavan_epoll_client {
	int fd;
	u32 events;
	u32 pending;
	struct cavan_epoll_service *service;
	struct cavan_epoll_pack *wr_head;
	struct cavan_epoll_pack *wr_tail;
	struct cavan_epoll_pack *rd_pack;
	struct cavan_epoll_pack header;
	struct cavan_epoll_client *poll_next;

	bool (*do_poll)(struct cavan_epoll_client *client);
	int (*do_read)(struct cavan_epoll_client *client, void *buff, u16 size);
	int (*do_write)(struct cavan_epoll_client *client, const void *buff, u16 size);
	int (*on_received)(struct cavan_epoll_client *client, const void *buff, u16 size);
};

struct cavan_epoll_service {
	const char *name;
	int epoll_fd;
	u32 index;
	u16 min, max;
	u16 count, used;
	bool as_daemon;
	bool verbose;
	size_t conn_size;
	int super_permission;
	void *private_data;
	const char *logfile;
	pthread_cond_t cond;
	pthread_mutex_t lock;
	const char *user, *group;
	struct cavan_epoll_client *poll_head;
	struct cavan_epoll_client *poll_tail;
};

// ================================================================================

extern int cavan_dynamic_service_count;
extern struct cavan_dynamic_service *cavan_dynamic_services[10];

// ================================================================================

void cavan_service_set_busy(struct cavan_service_description *desc, int index, bool busy);
int cavan_service_start(struct cavan_service_description *desc);
int cavan_service_main_loop(struct cavan_service_description *desc);
int cavan_service_run(struct cavan_service_description *desc);
int cavan_service_stop(struct cavan_service_description *desc);
int cavan_daemon_run(struct cavan_daemon_description *desc);
int cavan_daemon_stop(struct cavan_daemon_description *desc);

// ================================================================================

int cavan_dynamic_service_init(struct cavan_dynamic_service *service);
void cavan_dynamic_service_deinit(struct cavan_dynamic_service *service);
struct cavan_dynamic_service *cavan_dynamic_service_create(size_t size);
void cavan_dynamic_service_destroy(struct cavan_dynamic_service *service);
void cavan_dynamic_service_join(struct cavan_dynamic_service *service);
int cavan_dynamic_service_start(struct cavan_dynamic_service *service, bool sync);
int cavan_dynamic_service_run(struct cavan_dynamic_service *service);
int cavan_dynamic_service_stop(struct cavan_dynamic_service *service);
bool cavan_dynamic_service_register(struct cavan_dynamic_service *service);
bool cavan_dynamic_service_unregister(struct cavan_dynamic_service *service);
int cavan_dynamic_service_epoll_add(struct cavan_dynamic_service *service, int fd, void *conn);
void cavan_dynamic_service_epoll_remove(struct cavan_dynamic_service *service, int fd);
struct cavan_dynamic_service *cavan_dynamic_service_find(const char *name);
boolean cavan_dynamic_service_stop_by_name(const char *name);
void cavan_dynamic_service_scan(void *data, void (*handler)(struct cavan_dynamic_service *service, void *data));

// ================================================================================

void cavan_epoll_client_init(struct cavan_epoll_client *client, int fd);
int cavan_epoll_service_init(struct cavan_epoll_service *service);
void cavan_epoll_service_deinit(struct cavan_epoll_service *service);
int cavan_epoll_service_add(struct cavan_epoll_service *service, struct cavan_epoll_client *client);
void cavan_epoll_service_remove(struct cavan_epoll_service *service, struct cavan_epoll_client *client);
void cavan_epoll_service_run(struct cavan_epoll_service *service);

// ================================================================================

static inline void cavan_dynamic_service_lock(struct cavan_dynamic_service *service)
{
	pthread_mutex_lock(&service->lock);
}

static inline void cavan_dynamic_service_unlock(struct cavan_dynamic_service *service)
{
	pthread_mutex_unlock(&service->lock);
}

static inline void *cavan_dynamic_service_get_data(struct cavan_dynamic_service *service)
{
	return service->private_data;
}

static inline void cavan_dynamic_service_set_data(struct cavan_dynamic_service *service, void *data)
{
	service->private_data = data;
}

// ================================================================================

static inline void cavan_epoll_service_lock(struct cavan_epoll_service *service)
{
	pthread_mutex_lock(&service->lock);
}

static inline void cavan_epoll_service_unlock(struct cavan_epoll_service *service)
{
	pthread_mutex_unlock(&service->lock);
}

static inline void cavan_epoll_service_wait(struct cavan_epoll_service *service)
{
	pthread_cond_wait(&service->cond, &service->lock);
}

static inline void cavan_epoll_service_wakeup(struct cavan_epoll_service *service)
{
	pthread_cond_signal(&service->cond);
}

static inline void *cavan_epoll_service_get_data(struct cavan_epoll_service *service)
{
	return service->private_data;
}

static inline void cavan_epoll_service_set_data(struct cavan_epoll_service *service, void *data)
{
	service->private_data = data;
}
