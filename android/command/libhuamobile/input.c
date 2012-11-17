/*
 * File:		input.c
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

#include <huamobile/input.h>

static enum huamobile_event_device_type huamobile_input_get_device_type(int fd, const char *name)
{
	int ret;
	uint8_t abs_bitmask[ABS_BITMASK_SIZE];
	uint8_t key_bitmask[KEY_BITMASK_SIZE];

	pr_pos_info();

	ret = huamobile_event_get_abs_bitmask(fd, abs_bitmask, sizeof(abs_bitmask));
	if (ret < 0)
	{
		pr_error_info("huamobile_event_get_abs_bitmask");
		return HUA_EVENT_DEVICE_UNKNOWN;
	}

	if (huamobile_multi_touch_device_match(abs_bitmask))
	{
		pr_green_info("Deivce %s is mutil touch screen", name);
		return HUA_EVENT_DEVICE_MULTI_TOUCH;
	}

	ret = huamobile_event_get_key_bitmask(fd, key_bitmask, sizeof(key_bitmask));
	if (ret < 0)
	{
		pr_error_info("huamobile_event_get_key_bitmask");
		return HUA_EVENT_DEVICE_UNKNOWN;
	}

	if (huamobile_single_touch_device_match(abs_bitmask, key_bitmask))
	{
		pr_green_info("Deivce %s is single touch screen", name);
		return HUA_EVENT_DEVICE_SINGLE_TOUCH;
	}

	if (huamobile_gsensor_device_match(abs_bitmask))
	{
		pr_green_info("Device %s is g-sensor", name);
		return HUA_EVENT_DEVICE_GSENSOR;
	}

	if (huamobile_keypad_device_match(key_bitmask, sizeof(key_bitmask)))
	{
		pr_green_info("Device %s is keypad", name);
		return HUA_EVENT_DEVICE_KEYPAD;
	}

	pr_red_info("Device %s type unknown", name);

	return HUA_EVENT_DEVICE_UNKNOWN;
}

static enum huamobile_event_device_type huamobile_input_matcher(int fd, const char *name, void *data)
{
	struct huamobile_input_service *service = data;

	if (service->matcher && service->matcher(fd, name, service->private_data) == false)
	{
		pr_red_info("Match device %s failed", name);
		return HUA_EVENT_DEVICE_UNKNOWN;
	}

	return huamobile_input_get_device_type(fd, name);
}

static struct huamobile_event_device *huamobile_input_create_device(enum huamobile_event_device_type type, void *data)
{
	pr_pos_info();

	switch (type)
	{
	case HUA_EVENT_DEVICE_GSENSOR:
		return huamobile_gsensor_create(data);

	case HUA_EVENT_DEVICE_KEYPAD:
		return huamobile_keypad_create(data);

	case HUA_EVENT_DEVICE_MULTI_TOUCH:
		return huamobile_multi_touch_device_create(data);

	case HUA_EVENT_DEVICE_SINGLE_TOUCH:
		return huamobile_single_touch_device_create(data);

	case HUA_EVENT_DEVICE_UNKNOWN:
		break;

	default:
		pr_red_info("unknown device type %d", type);
	}

	return NULL;
}

static void huamobile_input_key_handler_dummy(struct huamobile_event_device *dev, const char *name, int code, int value, void *data)
{
	pr_bold_info("key: name = %s, code = %d, value = %d", name, code, value);
}

static void huamobile_input_touch_handler_dummy(struct huamobile_event_device *dev, struct huamobile_touch_point *point, void *data)
{
	pr_bold_info("touch[%d] = [%d, %d]", point->id, point->x, point->y);
}

static void huamobile_input_move_handler_dummy(struct huamobile_event_device *dev, struct huamobile_touch_point *point, void *data)
{
	pr_bold_info("move[%d] = [%d, %d]", point->id, point->x, point->y);
}

static void huamobile_input_release_handler_dummy(struct huamobile_event_device *dev, struct huamobile_touch_point *point, void *data)
{
	pr_bold_info("release[%d] = [%d, %d]", point->id, point->x, point->y);
}

static void huamobile_input_gsensor_handler_dummy(struct huamobile_event_device *dev, struct huamobile_gsensor_event *event, void *data)
{
	pr_bold_info("g-sensor: [%d, %d, %d]", event->x, event->y, event->z);
}

int huamobile_input_service_start(struct huamobile_input_service *service, void *data)
{
	struct huamobile_event_service *event_service;

	if (service == NULL)
	{
		pr_red_info("service == NULL");
		return -EINVAL;
	}

	pthread_mutex_init(&service->lock, NULL);

	if (service->key_handler == NULL)
	{
		service->key_handler = huamobile_input_key_handler_dummy;
	}

	if (service->touch_handler == NULL)
	{
		service->touch_handler = huamobile_input_touch_handler_dummy;
	}

	if (service->move_handler == NULL)
	{
		service->move_handler = huamobile_input_move_handler_dummy;
	}

	if (service->release_handler == NULL)
	{
		service->release_handler = huamobile_input_release_handler_dummy;
	}

	if (service->gsensor_handler == NULL)
	{
		service->gsensor_handler = huamobile_input_gsensor_handler_dummy;
	}

	service->private_data = data;

	event_service = &service->event_service;
	event_service->matcher = huamobile_input_matcher;
	event_service->create_device = huamobile_input_create_device;

	return huamobile_event_service_start(event_service, service);
}
