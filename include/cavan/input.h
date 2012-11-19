/*
 * File:		input.h
 * Author:		Fuang Cao <cavan.cfa@gmail.com>
 * Created:		2012-11-14
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
#include <cavan/event.h>
#include <cavan.h>

struct cavan_gsensor_event
{
	int x, y, z;
};

struct cavan_touch_point
{
	int id;
	int x;
	int y;
	int pressure;
	int released;
};

struct cavan_input_device
{
	struct cavan_event_device *event_dev;
	struct cavan_input_device *next;

	int (*probe)(struct cavan_input_device *dev, void *data);
	void (*remove)(struct cavan_input_device *dev, void *data);
	bool (*event_handler)(struct cavan_input_device *dev, struct input_event *event, void *data);
};

struct cavan_input_service
{
	struct cavan_event_service event_service;

	void *private_data;
	int lcd_width, lcd_height;
	double mouse_speed;
	pthread_mutex_t lock;

	void (*key_handler)(struct cavan_input_device *dev, const char *name, int code, int value, void *data);
	void (*wheel_handler)(struct cavan_input_device *dev, int value, void *data);
	void (*touch_handler)(struct cavan_input_device *dev, struct cavan_touch_point *point, void *data);
	void (*right_touch_handler)(struct cavan_input_device *dev, struct cavan_touch_point *point, void *data);
	void (*move_handler)(struct cavan_input_device *dev, struct cavan_touch_point *point, void *data);
	void (*release_handler)(struct cavan_input_device *dev, struct cavan_touch_point *point, void *data);
	void (*right_release_handler)(struct cavan_input_device *dev, struct cavan_touch_point *point, void *data);
	void (*gsensor_handler)(struct cavan_input_device *dev, struct cavan_gsensor_event *event, void *data);
};

void cavan_input_service_init(struct cavan_input_service *service, bool (*matcher)(struct cavan_event_matcher *, void *));
int cavan_input_service_start(struct cavan_input_service *service, void *data);
int cavan_input_service_stop(struct cavan_input_service *service);
