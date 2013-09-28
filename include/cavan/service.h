#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Mon Apr 16 10:03:27 CST 2012
 */

#include <cavan.h>
#include <cavan/list.h>
#include <linux/capability.h>

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

struct cavan_service_daemon
{
	int index;
	struct double_link_node node;
};

struct cavan_service
{
	const char *name;
	int min, max;
	struct double_link link;
};

extern int capget(cap_user_header_t hdrp, cap_user_data_t datap);
extern int capset(cap_user_header_t hdrp, const cap_user_data_t datap);

void cavan_service_set_busy(struct cavan_service_description *desc, int index, bool busy);
int cavan_service_start(struct cavan_service_description *desc);
int cavan_service_main_loop(struct cavan_service_description *desc);
int cavan_service_run(struct cavan_service_description *desc);
int cavan_service_stop(struct cavan_service_description *desc);
int cavan_daemon_permission_clear(u32 permission);
int cavan_daemon_run(struct cavan_daemon_description *desc);
int cavan_daemon_stop(struct cavan_daemon_description *desc);
