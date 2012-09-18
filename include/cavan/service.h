#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Mon Apr 16 10:03:27 CST 2012
 */

#include <cavan.h>

union cavan_service_data
{
	void *type_void;

	char type_char;
	short type_short;
	int type_int;
	long type_long;

	unsigned char type_uchar;
	unsigned short type_ushort;
	unsigned int type_uint;
	unsigned long type_ulong;
};

struct cavan_service_description
{
	const char *name;
	int daemon_count;
	int as_daemon;
	int show_verbose;
	int super_permission;
	union cavan_service_data data;
	pthread_t *threads;
	pthread_mutex_t mutex_lock;
	int (*handler)(int index, union cavan_service_data data);
};

struct cavan_daemon_description
{
	char cmdfile[512];
	char pidfile[512];
	int as_daemon;
	int super_permission;
	char *argv[32];
};

int cavan_service_run(struct cavan_service_description *desc);
int cavan_service_stop(struct cavan_service_description *desc);
int cavan_daemon_run(struct cavan_daemon_description *desc);
int cavan_daemon_stop(struct cavan_daemon_description *desc);

