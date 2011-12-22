#pragma once

// Fuang.Cao <cavan.cfa@gmail.com> 2011-12-22 12:20:56

#include <sys/inotify.h>

#define INOTIFY_MAX_WATCH_COUNT		32

struct cavan_inotify_watch
{
	int wd;
	void *data;
	char pathname[1024];
	int (*handle)(const char *, struct inotify_event *, void *);
};

struct cavan_inotify_descriptor
{
	int fd;
	int watch_count;
	struct cavan_inotify_watch watchs[INOTIFY_MAX_WATCH_COUNT];
};

int cavan_inotify_init(struct cavan_inotify_descriptor *desc);
void cavan_inotify_uninit(struct cavan_inotify_descriptor *desc);
int cavan_inotify_register_watch(struct cavan_inotify_descriptor *desc, const char *pathname, int (*handle)(const char *pathname, struct inotify_event *event, void *data), uint32_t mask, void *data);
int cavan_inotify_unregister_watch(struct cavan_inotify_descriptor *desc, const char *pathname);
int cavan_inotify_event_loop(struct cavan_inotify_descriptor *desc);

