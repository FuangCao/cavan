/*
 * File:		input.c
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

#include <cavan/input.h>
#include <cavan/mouse.h>
#include <cavan/keypad.h>
#include <cavan/gsensor.h>
#include <cavan/touchpad.h>
#include <cavan/touchscreen.h>

static struct cavan_input_device *cavan_input_device_create(uint8_t *key_bitmask, uint8_t *abs_bitmask, uint8_t *rel_bitmask)
{
	if (cavan_gsensor_device_match(abs_bitmask))
	{
		pr_green_info("G-Sensor Matched");
		return cavan_gsensor_create();
	}

	if (cavan_touchpad_device_match(key_bitmask, abs_bitmask))
	{
		pr_green_info("Touch Pad Matched");
		return cavan_touchpad_device_create();
	}

	if (cavan_multi_touch_device_match(abs_bitmask))
	{
		pr_green_info("Muti Touch Panel Matched");
		return cavan_multi_touch_device_create();
	}

	if (cavan_single_touch_device_match(abs_bitmask, key_bitmask))
	{
		pr_green_info("Single Touch Panel Matched");
		return cavan_single_touch_device_create();
	}

	if (cavan_mouse_device_match(key_bitmask, rel_bitmask))
	{
		pr_green_info("Mouse Matched");
		return cavan_mouse_create();
	}

	if (cavan_keypad_device_match(key_bitmask))
	{
		pr_green_info("Keypad Matched");
		return cavan_keypad_create();
	}

	return NULL;
}

static int cavan_input_device_probe(struct cavan_event_device *event_dev, void *data)
{
	int ret;
	int fd = event_dev->fd;
	uint8_t key_bitmask[KEY_BITMASK_SIZE];
	uint8_t abs_bitmask[ABS_BITMASK_SIZE];
	uint8_t rel_bitmask[REL_BITMASK_SIZE];
	struct cavan_input_device *dev, *head, *tail;

	ret = cavan_event_get_abs_bitmask(fd, abs_bitmask);
	if (ret < 0)
	{
		pr_error_info("cavan_event_get_abs_bitmask");
		return ret;
	}

	ret = cavan_event_get_key_bitmask(fd, key_bitmask);
	if (ret < 0)
	{
		pr_error_info("cavan_event_get_key_bitmask");
		return ret;
	}

	ret = cavan_event_get_rel_bitmask(fd, rel_bitmask);
	if (ret < 0)
	{
		pr_error_info("cavan_event_get_rel_bitmask");
		return ret;
	}

	head = tail = NULL;

	while (1)
	{
		dev = cavan_input_device_create(key_bitmask, abs_bitmask, rel_bitmask);
		if (dev == NULL)
		{
			break;
		}

		dev->event_dev = event_dev;

		if (dev->probe && dev->probe(dev, data) < 0)
		{
			free(dev);
			continue;
		}

		if (head)
		{
			tail->next = dev;
			tail = dev;
		}
		else
		{
			head = tail = dev;
		}
	}

	if (head == NULL)
	{
		pr_red_info("can't recognize device");
		return -EINVAL;
	}

	tail->next = NULL;
	event_dev->private_data = head;

	return 0;
}

static void cavan_input_device_remove(struct cavan_event_device *event_dev, void *data)
{
	struct cavan_input_device *dev = event_dev->private_data, *next;

	while (dev)
	{
		if (dev->remove)
		{
			dev->remove(dev, data);
		}

		next = dev->next;
		free(dev);
		dev = next;
	}
}

static bool cavan_input_device_event_handler(struct cavan_event_device *event_dev, struct input_event *event, void *data)
{
	struct cavan_input_device *dev = event_dev->private_data;

	switch(event->type)
	{
	case EV_SYN:
		while (dev)
		{
			dev->event_handler(dev, event, data);
			dev = dev->next;
		}
		return true;

	case EV_MSC:
		return true;

	default:
		while (dev)
		{
			if (dev->event_handler(dev, event, data))
			{
				return true;
			}

			dev = dev->next;
		}
	}

	return false;
}

static bool cavan_input_device_matcher(struct cavan_event_matcher *matcher, void *data)
{
	struct cavan_input_service *service = data;

	if (service->matcher)
	{
		return service->matcher(matcher, service->private_data);
	}

	return true;
}

static void cavan_input_key_handler_dummy(struct cavan_input_device *dev, const char *name, int code, int value, void *data)
{
	pr_bold_info("key: name = %s, code = %d, value = %d", name, code, value);
}

static void cavan_input_touch_handler_dummy(struct cavan_input_device *dev, struct cavan_touch_point *point, void *data)
{
	if (point->pressure)
	{
		pr_bold_info("touch[%d] = [%d, %d]", point->id, point->x, point->y);
	}
	else
	{
		pr_bold_info("release[%d] = [%d, %d]", point->id, point->x, point->y);
	}
}

static void cavan_input_move_handler_dummy(struct cavan_input_device *dev, struct cavan_touch_point *point, void *data)
{
	pr_bold_info("move[%d] = [%d, %d]", point->id, point->x, point->y);
}

static void cavan_input_gsensor_handler_dummy(struct cavan_input_device *dev, struct cavan_gsensor_event *event, void *data)
{
	pr_bold_info("g-sensor: [%d, %d, %d]", event->x, event->y, event->z);
}

static void cavan_input_mouse_wheel_handler_dummy(struct cavan_input_device *dev, int code, int value, void *data)
{
	pr_bold_info("wheel: code = %d, value = %d", code, value);
}

static void cavan_input_mouse_move_handler_dummy(struct cavan_input_device *dev, int x, int y, void *data)
{
	pr_bold_info("mouse_move: x = %d, y = %d", x, y);
}

static void cavan_input_mouse_touch_handler_dummy(struct cavan_input_device *dev, int code, int value, void *data)
{
	pr_bold_info("mouse_touch: code = %d, value = %d", code, value);
}

void cavan_input_service_init(struct cavan_input_service *service, bool (*matcher)(struct cavan_event_matcher *, void *))
{
	cavan_event_service_init(&service->event_service, cavan_input_device_matcher);

	service->lcd_width = -1;
	service->lcd_height = -1;

	service->matcher = matcher;
	service->key_handler = NULL;
	service->mouse_wheel_handler = NULL;
	service->mouse_move_handler = NULL;
	service->mouse_touch_handler = NULL;
	service->touch_handler = NULL;
	service->move_handler = NULL;
	service->gsensor_handler = NULL;
}

int cavan_input_service_start(struct cavan_input_service *service, void *data)
{
	int ret;
	struct cavan_event_service *event_service;

	if (service == NULL)
	{
		pr_red_info("service == NULL");
		return -EINVAL;
	}

	pthread_mutex_init(&service->lock, NULL);

	if (service->key_handler == NULL)
	{
		service->key_handler = cavan_input_key_handler_dummy;
	}

	if (service->touch_handler == NULL)
	{
		service->touch_handler = cavan_input_touch_handler_dummy;
	}

	if (service->move_handler == NULL)
	{
		service->move_handler = cavan_input_move_handler_dummy;
	}

	if (service->gsensor_handler == NULL)
	{
		service->gsensor_handler = cavan_input_gsensor_handler_dummy;
	}

	if (service->mouse_wheel_handler == NULL)
	{
		service->mouse_wheel_handler = cavan_input_mouse_wheel_handler_dummy;
	}

	if (service->mouse_move_handler == NULL)
	{
		service->mouse_move_handler = cavan_input_mouse_move_handler_dummy;
	}

	if (service->mouse_touch_handler == NULL)
	{
		service->mouse_touch_handler = cavan_input_mouse_touch_handler_dummy;
	}

	service->private_data = data;

	event_service = &service->event_service;
	event_service->matcher = cavan_input_device_matcher;
	event_service->probe = cavan_input_device_probe;
	event_service->remove = cavan_input_device_remove;
	event_service->event_handler = cavan_input_device_event_handler;

	ret = cavan_timer_service_start(&service->timer_service);
	if (ret < 0)
	{
		pr_red_info("cavan_timer_service_start");
		return ret;
	}

	ret = cavan_event_service_start(event_service, service);
	if (ret < 0)
	{
		pr_red_info("cavan_event_service_start");
		goto out_timer_service_stop;
	}

	return 0;

out_timer_service_stop:
	cavan_timer_service_stop(&service->timer_service);
	return ret;
}

int cavan_input_service_stop(struct cavan_input_service *service)
{
	int ret;

	ret = cavan_event_service_stop(&service->event_service);
	if (ret < 0)
	{
		pr_red_info("cavan_event_service_stop");
		return ret;
	}

	cavan_timer_service_stop(&service->timer_service);

	pthread_mutex_destroy(&service->lock);

	return 0;
}
