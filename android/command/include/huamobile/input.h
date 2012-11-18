/*
 * File:		input.h
 * Based on:
 * Author:		Fuang Cao <cavan.cfa@gmail.com>
 *
 * Created:		2012-11-14
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

#include <huamobile/event.h>
#include <huamobile.h>

struct huamobile_gsensor_event
{
	int x, y, z;
};

struct huamobile_touch_point
{
	int id;
	int x;
	int y;
	int pressure;
	int released;
};

struct huamobile_input_device
{
	struct huamobile_event_device *event_dev;
	struct huamobile_input_device *next;

	int (*probe)(struct huamobile_input_device *dev, void *data);
	void (*remove)(struct huamobile_input_device *dev, void *data);
	bool (*event_handler)(struct huamobile_input_device *dev, struct input_event *event, void *data);
};

struct huamobile_input_service
{
	struct huamobile_event_service event_service;

	void *private_data;
	int lcd_width, lcd_height;
	double mouse_speed;
	pthread_mutex_t lock;

	void (*key_handler)(struct huamobile_input_device *dev, const char *name, int code, int value, void *data);
	void (*wheel_handler)(struct huamobile_input_device *dev, int value, void *data);
	void (*touch_handler)(struct huamobile_input_device *dev, struct huamobile_touch_point *point, void *data);
	void (*right_touch_handler)(struct huamobile_input_device *dev, struct huamobile_touch_point *point, void *data);
	void (*move_handler)(struct huamobile_input_device *dev, struct huamobile_touch_point *point, void *data);
	void (*release_handler)(struct huamobile_input_device *dev, struct huamobile_touch_point *point, void *data);
	void (*right_release_handler)(struct huamobile_input_device *dev, struct huamobile_touch_point *point, void *data);
	void (*gsensor_handler)(struct huamobile_input_device *dev, struct huamobile_gsensor_event *event, void *data);
};

int huamobile_input_service_start(struct huamobile_input_service *service, void *data);

static inline int huamobile_input_service_stop(struct huamobile_input_service *service)
{
	return huamobile_event_service_stop(&service->event_service);
}
