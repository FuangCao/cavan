#pragma once

// Fuang.Cao <cavan.cfa@gmail.com> Wed May 25 10:12:17 CST 2011

#include <cavan.h>
#include <cavan/list.h>
#include <cavan/thread.h>
#include <linux/input.h>

#define test_bit(bit, array) \
	((array)[(bit) >> 3] & (1 << ((bit) & 0x07)))

#define set_bit(bit, array) \
	((array)[(bit) >> 3] |= 1 << ((bit) & 0x07))

#define clean_bit(bit, array) \
	((array)[(bit) >> 3] &= ~(1 << ((bit) & 0x07)))

#define sizeof_bit_array(bits) \
	(((bits) + 7) >> 3)

#ifndef ABS_CNT
#define ABS_CNT				(ABS_MAX + 1)
#endif

#ifndef KEY_CNT
#define KEY_CNT				(KEY_MAX + 1)
#endif

#ifndef REL_CNT
#define REL_CNT				(REL_MAX + 1)
#endif

#define ABS_BITMASK_SIZE	sizeof_bit_array(ABS_CNT)
#define KEY_BITMASK_SIZE	sizeof_bit_array(KEY_CNT)
#define REL_BITMASK_SIZE	sizeof_bit_array(REL_CNT)

struct cavan_keylayout_node
{
	char name[32];
	int code;

	struct single_link_node node;
};

struct cavan_virtual_key
{
	int left;
	int right;
	int top;
	int bottom;
	int code;
	int value;

	const char *name;

	struct single_link_node node;
};

struct cavan_event_matcher
{
	int fd;
	char devname[128];
	char pathname[24];
	bool (*match)(struct cavan_event_matcher *matcher, void *data);
	int (*handler)(struct cavan_event_matcher *matcher, void *data);
};

struct cavan_event_device
{
	int fd;
	char name[128];
	char pathname[24];
	struct pollfd *pfd;
	void *private_data;

	struct single_link vk_link;
	struct single_link kl_link;
	struct double_link_node node;
};

struct cavan_event_service
{
	struct cavan_thread thread;
	pthread_mutex_t lock;

	void *private_data;
	size_t pfd_count;
	struct pollfd *pfds;

	struct double_link link;

	bool (*matcher)(struct cavan_event_matcher *matcher, void *data);
	int (*probe)(struct cavan_event_device *dev, void *data);
	void (*remove)(struct cavan_event_device *dev, void *data);
	bool (*event_handler)(struct cavan_event_device *dev, struct input_event *event, void *data);
};

struct cavan_virtual_key *cavan_event_find_virtual_key(struct cavan_event_device *dev, int x, int y);
const char *cavan_event_find_key_name(struct cavan_event_device *dev, int code);

ssize_t cavan_event_scan_devices(struct cavan_event_matcher *matcher, void *data);
void cavan_event_service_init(struct cavan_event_service *service, bool (*matcher)(struct cavan_event_matcher *, void *));
int cavan_event_service_start(struct cavan_event_service *service, void *data);
int cavan_event_service_stop(struct cavan_event_service *service);

bool cavan_event_simple_matcher(struct cavan_event_matcher *matcher, void *data);
bool cavan_event_name_matcher(const char *devname, ...);
int cavan_event_get_absinfo(int fd, int axis, int *min, int *max);
char *cavan_event_tostring(struct input_event *event, char *text);

static inline char *cavan_event_tostring_simple(struct input_event *event, char *text)
{
	sprintf(text, "type = %d, code = %d, value = %d", event->type, event->code, event->value);

	return text;
}

static inline int cavan_event_send_event(struct cavan_event_service *service, u32 event)
{
	return cavan_thread_send_event(&service->thread, event);
}

static inline int cavan_event_service_join(struct cavan_event_service *service)
{
	return cavan_thread_join(&service->thread);
}

static inline int cavan_event_get_bitmask(int fd, int type, void *bitmask, size_t size)
{
	mem_set(bitmask, 0, size);

	return ioctl(fd, EVIOCGBIT(type, size), bitmask);
}

static inline int cavan_event_get_devname(int fd, char *devname, size_t size)
{
	return ioctl(fd, EVIOCGNAME(size), devname);
}

static inline int cavan_event_get_abs_bitmask(int fd, void *bitmask)
{
	return cavan_event_get_bitmask(fd, EV_ABS, bitmask, ABS_BITMASK_SIZE);
}

static inline int cavan_event_get_key_bitmask(int fd, void *bitmask)
{
	return cavan_event_get_bitmask(fd, EV_KEY, bitmask, KEY_BITMASK_SIZE);
}

static inline int cavan_event_get_rel_bitmask(int fd, void *bitmask)
{
	return cavan_event_get_bitmask(fd, EV_REL, bitmask, REL_BITMASK_SIZE);
}

static inline int cavan_event_start_poll_thread(struct cavan_event_service *service)
{
	return cavan_thread_start(&service->thread);
}

static inline void cavan_event_stop_poll_thread(struct cavan_event_service *service)
{
	cavan_thread_stop(&service->thread);
}
