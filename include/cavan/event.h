#pragma once

// Fuang.Cao <cavan.cfa@gmail.com> Wed May 25 10:12:17 CST 2011

#include <cavan/input.h>
#include <poll.h>

#define INPUT_EVENT_PATH	"/dev/input/event"

struct event_desc
{
	int fd;
	char dev_path[1024];
	char dev_name[1024];
};

int event_init_by_path(struct event_desc *desc, const char *pathname);
int event_init_by_name(struct event_desc *desc, const char *devname);
void event_uninit(struct event_desc *desc);
char *event_to_text(struct input_event *event, char *text);
ssize_t read_events(struct event_desc *desc, struct input_event *buff, size_t count);
ssize_t read_events_timeout(struct event_desc *desc, struct input_event *buff, size_t count, long timeout);
int read_event_by_type(struct event_desc *desc, u16 type, struct input_event *event);

ssize_t open_event_devices(struct pollfd *event_fds, size_t size, int flags);
void close_event_devices(struct pollfd *event_fds, size_t count);
ssize_t poll_event_devices(struct pollfd *event_fds, size_t count, void *buff, size_t length);

static inline int device_get_devname(int fd, char *devname, size_t size)
{
	return  ioctl(fd, EVIOCGNAME(size), devname);
}

static inline char *event_to_text_simple(struct input_event *event, char *text)
{
	sprintf(text, "type = %d, code = %d, value = %d", event->type, event->code, event->value);

	return text;
}

