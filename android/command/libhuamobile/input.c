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
#include <huamobile/mouse.h>
#include <huamobile/keypad.h>
#include <huamobile/gsensor.h>
#include <huamobile/touchscreen.h>

static struct huamobile_input_device *huamobile_input_device_create(uint8_t *key_bitmask, uint8_t *abs_bitmask, uint8_t *rel_bitmask)
{
	if (huamobile_gsensor_device_match(abs_bitmask))
	{
		return huamobile_gsensor_create();
	}

	if (huamobile_multi_touch_device_match(abs_bitmask))
	{
		return huamobile_multi_touch_device_create();
	}

	if (huamobile_single_touch_device_match(abs_bitmask, key_bitmask))
	{
		return huamobile_single_touch_device_create();
	}

	if (huamobile_mouse_device_match(key_bitmask, rel_bitmask))
	{
		return huamobile_mouse_create();
	}

	if (huamobile_keypad_device_match(key_bitmask, sizeof(key_bitmask)))
	{
		return huamobile_keypad_create();
	}

	return NULL;
}

static int huamobile_input_device_probe(struct huamobile_event_device *event_dev, void *data)
{
	int ret;
	int fd = event_dev->fd;
	uint8_t key_bitmask[KEY_BITMASK_SIZE];
	uint8_t abs_bitmask[ABS_BITMASK_SIZE];
	uint8_t rel_bitmask[REL_BITMASK_SIZE];
	struct huamobile_input_device *dev, *head = NULL, *tail;

	pr_pos_info();

	ret = huamobile_event_get_abs_bitmask(fd, abs_bitmask, sizeof(abs_bitmask));
	if (ret < 0)
	{
		pr_error_info("huamobile_event_get_abs_bitmask");
		return ret;
	}

	ret = huamobile_event_get_key_bitmask(fd, key_bitmask, sizeof(key_bitmask));
	if (ret < 0)
	{
		pr_error_info("huamobile_event_get_key_bitmask");
		return ret;
	}

	ret = huamobile_event_get_rel_bitmask(fd, rel_bitmask, sizeof(rel_bitmask));
	if (ret < 0)
	{
		pr_error_info("huamobile_event_get_rel_bitmask");
		return ret;
	}

	while (1)
	{
		dev = huamobile_input_device_create(key_bitmask, abs_bitmask, rel_bitmask);
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

static void huamobile_input_device_remove(struct huamobile_event_device *event_dev, void *data)
{
	struct huamobile_input_device *dev = event_dev->private_data, *next;

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

static bool huamobile_input_device_event_handler(struct huamobile_event_device *event_dev, struct input_event *event, void *data)
{
	struct huamobile_input_device *dev = event_dev->private_data;

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

static void huamobile_input_key_handler_dummy(struct huamobile_input_device *dev, const char *name, int code, int value, void *data)
{
	pr_bold_info("key: name = %s, code = %d, value = %d", name, code, value);
}

static void huamobile_input_touch_handler_dummy(struct huamobile_input_device *dev, struct huamobile_touch_point *point, void *data)
{
	pr_bold_info("touch[%d] = [%d, %d]", point->id, point->x, point->y);
}

static void huamobile_input_move_handler_dummy(struct huamobile_input_device *dev, struct huamobile_touch_point *point, void *data)
{
	pr_bold_info("move[%d] = [%d, %d]", point->id, point->x, point->y);
}

static void huamobile_input_release_handler_dummy(struct huamobile_input_device *dev, struct huamobile_touch_point *point, void *data)
{
	pr_bold_info("release[%d] = [%d, %d]", point->id, point->x, point->y);
}

static void huamobile_input_gsensor_handler_dummy(struct huamobile_input_device *dev, struct huamobile_gsensor_event *event, void *data)
{
	pr_bold_info("g-sensor: [%d, %d, %d]", event->x, event->y, event->z);
}

static void huamobile_input_right_touch_handler_dummy(struct huamobile_input_device *dev, struct huamobile_touch_point *point, void *data)
{
	pr_bold_info("right_touch[%d] = [%d, %d]", point->id, point->x, point->y);
}

static void huamobile_input_right_release_handler_dummy(struct huamobile_input_device *dev, struct huamobile_touch_point *point, void *data)
{
	pr_bold_info("right_release[%d] = [%d, %d]", point->id, point->x, point->y);
}

static void huamobile_input_wheel_handler_dummy(struct huamobile_input_device *dev, int value, void *data)
{
	pr_bold_info("wheel: value = %d", value);
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

	if (service->wheel_handler == NULL)
	{
		service->wheel_handler = huamobile_input_wheel_handler_dummy;
	}

	if (service->right_touch_handler == NULL)
	{
		service->right_touch_handler = huamobile_input_right_touch_handler_dummy;
	}

	if (service->right_release_handler == NULL)
	{
		service->right_release_handler = huamobile_input_right_release_handler_dummy;
	}

	if (service->mouse_speed <= 0)
	{
		service->mouse_speed = 1;
	}

	service->private_data = data;

	event_service = &service->event_service;
	event_service->probe = huamobile_input_device_probe;
	event_service->remove = huamobile_input_device_remove;
	event_service->event_handler = huamobile_input_device_event_handler;

	return huamobile_event_service_start(event_service, service);
}
