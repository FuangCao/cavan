/*
 * File:         input.h
 * Based on:
 * Author:       Fuang Cao <cavan.cfa@gmail.com>
 *
 * Created:	  2012-11-14
 * Description:  HUAMOBILE LIBRARY
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

#ifndef SYN_MT_REPORT
#define SYN_MT_REPORT		2
#define ABS_MT_POSITION_X	0x35	/* Center X ellipse position */
#define ABS_MT_POSITION_Y	0x36	/* Center Y ellipse position */
#define ABS_MT_TOUCH_MAJOR	0x30	/* Major axis of touching ellipse */
#define ABS_MT_TRACKING_ID	0x39	/* Unique ID of initiated contact */
#endif

#ifndef ABS_CNT
#define ABS_CNT				(ABS_MAX + 1)
#endif

#ifndef KEY_CNT
#define KEY_CNT				(KEY_MAX + 1)
#endif

enum huamobile_input_command
{
	HUA_INPUT_COMMAND_STOP
};

enum huamobile_input_service_state
{
	HUA_INPUT_THREAD_STATE_RUNNING,
	HUA_INPUT_THREAD_STATE_STOPPING,
	HUA_INPUT_THREAD_STATE_STOPPED
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

struct huamobile_touch_point
{
	int id;
	int x;
	int y;
	int pressure;
};

struct huamobile_input_device
{
	int fd;
	struct pollfd *pfd;
	char name[512];

	struct huamobile_virtual_key *vk_head;
	struct huamobile_keylayout_node *kl_head;

	void *private_data;
	struct huamobile_input_device *next;
};

struct huamobile_input_service
{
	pthread_t thread;
	int pipefd[2];
	pthread_mutex_t lock;
	enum huamobile_input_service_state state;

	void *private_data;
	struct huamobile_input_device *dev_head;

	int (*matcher)(struct huamobile_input_device *dev, void *data);
	int (*probe)(struct huamobile_input_device *dev, void *data);
	void (*remove)(struct huamobile_input_device *dev, void *data);
	int (*event_handler)(struct huamobile_input_device *dev, struct input_event *event, void *data);
};

struct huamobile_ts_device
{
	int pressed;
	int released;

	double xscale, yscale;
	double xoffset, yoffset;

	struct huamobile_touch_point points[10];
	int point_count;

	struct huamobile_input_device *input_dev;
	struct huamobile_ts_device *next;
};

struct huamobile_ts_service
{
	int lcd_width, lcd_height;
	pthread_mutex_t lock;

	void *private_data;
	struct huamobile_input_service input_service;
	struct huamobile_ts_device *dev_head;

	int (*matcher)(struct huamobile_input_device *dev, void *data);
	int (*probe)(struct huamobile_ts_device *dev, void *data);
	int (*remove)(struct huamobile_ts_device *dev, void *data);
	int (*key_handler)(struct huamobile_ts_device *dev, const char *name, int code, int value, void *data);
	int (*point_handler)(struct huamobile_ts_device *dev, struct huamobile_touch_point *point, void *data);
};

int huamobile_input_start_poll_thread(struct huamobile_input_service *service);
int huamobile_input_stop_poll_thread(struct huamobile_input_service *service);
int huamobile_input_service_start(struct huamobile_input_service *service, void *data);
int huamobile_input_service_stop(struct huamobile_input_service *service);

int huamobile_ts_service_start(struct huamobile_ts_service *service, void *data);

int huamobile_touch_screen_matcher_multi(struct huamobile_ts_device *ts, struct huamobile_input_device *dev, void *data);
int huamobile_touch_screen_matcher_single(struct huamobile_ts_device *ts, struct huamobile_input_device *dev, void *data);
int huamobile_touch_screen_matcher(struct huamobile_ts_device *ts, struct huamobile_input_device *dev, void *data);
int huamobile_gsensor_matcher(struct huamobile_input_device *dev, void *data);
int huamobile_input_name_matcher(const char *devname, ...);

static inline int huamobile_input_send_command(struct huamobile_input_service *service, enum huamobile_input_command cmd)
{
	return write(service->pipefd[1], &cmd, sizeof(cmd));
}

static inline int huamobile_input_service_join(struct huamobile_input_service *service)
{
	return pthread_join(service->thread, NULL);
}

static inline int huamobile_ts_service_stop(struct huamobile_ts_service *service)
{
	return huamobile_input_service_stop(&service->input_service);
}
