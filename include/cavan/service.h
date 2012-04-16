#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Mon Apr 16 10:03:27 CST 2012
 */

#include <cavan.h>

struct cavan_service_description
{
	const char *name;
	int daemon_count;
	int as_daemon;
	int show_verbose;
	void *data;
	pthread_t *threads;
	pthread_mutex_t mutex_lock;
	int (*handler)(int index, void *data);
};

int cavan_service_run(struct cavan_service_description *desc);
int cavan_service_stop(struct cavan_service_description *desc);

