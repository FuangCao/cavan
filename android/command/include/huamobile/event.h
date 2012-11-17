/*
 * File:		event.h
 * Based on:
 * Author:		Fuang Cao <cavan.cfa@gmail.com>
 *
 * Created:		2012-11-17
 * Description:	HUAMOBILE LIBRARY
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#pragma once

#include <linux/input.h>
#include <huamobile.h>

#define test_bit(bit, array) \
	((array)[(bit) >> 3] & (1 << ((bit) & 0x07)))

#define sizeof_bit_array(bits) \
	(((bits) + 7) >> 3)

#ifndef ABS_CNT
#define ABS_CNT				(ABS_MAX + 1)
#endif

#ifndef KEY_CNT
#define KEY_CNT				(KEY_MAX + 1)
#endif

#define ABS_BITMASK_SIZE	sizeof_bit_array(ABS_CNT)
#define KEY_BITMASK_SIZE	sizeof_bit_array(KEY_CNT)

enum huamobile_event_command
{
	HUA_INPUT_COMMAND_STOP
};

enum huamobile_event_service_state
{
	HUA_INPUT_THREAD_STATE_RUNNING,
	HUA_INPUT_THREAD_STATE_STOPPING,
	HUA_INPUT_THREAD_STATE_STOPPED
};

enum huamobile_event_device_type
{
	HUA_EVENT_DEVICE_UNKNOWN,
	HUA_EVENT_DEVICE_KEYPAD,
	HUA_EVENT_DEVICE_MULTI_TOUCH,
	HUA_EVENT_DEVICE_SINGLE_TOUCH,
	HUA_EVENT_DEVICE_GSENSOR
};

struct huamobile_keylayout_node
{
	char name[32];
	int code;

	struct huamobile_keylayout_node *next;
};

struct huamobile_virtual_key
{
	int left;
	int right;
	int top;
	int bottom;
	int code;
	int value;

	const char *name;
	struct huamobile_virtual_key *next;
};

struct huamobile_event_device
{
	int fd;
	char name[512];
	struct pollfd *pfd;
	enum huamobile_event_device_type type;

	struct huamobile_virtual_key *vk_head;
	struct huamobile_keylayout_node *kl_head;
	struct huamobile_event_device *next;

	int (*probe)(struct huamobile_event_device *dev, void *data);
	void (*remove)(struct huamobile_event_device *dev, void *data);
	void (*destroy)(struct huamobile_event_device *dev, void *data);
	void (*event_handler)(struct huamobile_event_device *dev, struct input_event *event, void *data);
};

struct huamobile_event_service
{
	pthread_t thread;
	int pipefd[2];
	pthread_mutex_t lock;
	enum huamobile_event_service_state state;

	void *private_data;
	struct huamobile_event_device *dev_head;

	enum huamobile_event_device_type (*matcher)(int fd, const char *name, void *data);
	struct huamobile_event_device *(*create_device)(enum huamobile_event_device_type type, void *data);
};

struct huamobile_virtual_key *huamobile_event_find_virtual_key(struct huamobile_event_device *dev, int x, int y);
const char *huamobile_event_find_key_name(struct huamobile_event_device *dev, int code);

int huamobile_event_start_poll_thread(struct huamobile_event_service *service);
int huamobile_event_stop_poll_thread(struct huamobile_event_service *service);
int huamobile_event_service_start(struct huamobile_event_service *service, void *data);
int huamobile_event_service_stop(struct huamobile_event_service *service);

bool huamobile_event_name_matcher(const char *devname, ...);
int huamobile_event_get_absinfo(int fd, int axis, int *min, int *max);

static inline int huamobile_event_send_command(struct huamobile_event_service *service, enum huamobile_event_command cmd)
{
	return write(service->pipefd[1], &cmd, sizeof(cmd));
}

static inline int huamobile_event_service_join(struct huamobile_event_service *service)
{
	return pthread_join(service->thread, NULL);
}

static inline int huamobile_event_get_bitmask(int fd, int type, void *bitmask, size_t size)
{
	memset(bitmask, 0, sizeof(bitmask));

	return ioctl(fd, EVIOCGBIT(type, size), bitmask);
}

static inline int huamobile_event_get_devname(int fd, char *devname, size_t size)
{
	return ioctl(fd, EVIOCGNAME(size), devname);
}

static inline int huamobile_event_get_abs_bitmask(int fd, void *bitmask, size_t size)
{
	return huamobile_event_get_bitmask(fd, EV_ABS, bitmask, size);
}

static inline int huamobile_event_get_key_bitmask(int fd, void *bitmask, size_t size)
{
	return huamobile_event_get_bitmask(fd, EV_KEY, bitmask, size);
}
