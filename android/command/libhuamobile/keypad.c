/*
 * File:		keypad.c
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

#include <huamobile/keypad.h>
#include <huamobile/input.h>

bool huamobile_keypad_device_match(const uint8_t *key_bitmask, size_t size)
{
	const uint8_t *key_end;

	for (key_end = key_bitmask + size; key_bitmask < key_end; key_bitmask++)
	{
		if (*key_bitmask)
		{
			return true;
		}
	}

	return false;
}

bool huamobile_keypad_device_matcher(int fd, const char *name, void *data)
{
	int ret;
	uint8_t key_bitmask[ABS_BITMASK_SIZE];

	pr_pos_info();

	ret = huamobile_event_get_key_bitmask(fd, key_bitmask, sizeof(key_bitmask));
	if (ret < 0)
	{
		pr_error_info("huamobile_event_get_key_bitmask");
		return ret;
	}

	return huamobile_keypad_device_match(key_bitmask, sizeof(key_bitmask));
}

static void huamobile_keypad_event_handler(struct huamobile_event_device *dev, struct input_event *event, void *data)
{
	struct huamobile_input_service *service = data;
	const char *keyname;

	switch (event->type)
	{
	case EV_KEY:
		keyname = huamobile_event_find_key_name(dev, event->code);
		service->key_handler(dev, keyname, event->code, event->value, service->private_data);
		break;

	case EV_MSC:
	case EV_SYN:
		break;

	default:
		pr_red_info("unknown keypad event %d", event->type);
	}
}

static int huamobile_keypad_probe(struct huamobile_event_device *dev, void *data)
{
	struct huamobile_input_service *service = data;

	pr_pos_info();

	if (service->probe)
	{
		return service->probe(dev, service->private_data);
	}

	return 0;
}

static void huamobile_keypad_remove(struct huamobile_event_device *dev, void *data)
{
	struct huamobile_input_service *service = data;

	pr_pos_info();

	if (service->remove)
	{
		service->remove(dev, service->private_data);
	}
}

static void huamobile_keypad_destroy(struct huamobile_event_device *dev, void *data)
{
	pr_pos_info();

	free(dev);
}

struct huamobile_event_device *huamobile_keypad_create(void *data)
{
	struct huamobile_keypad_device *keypad;
	struct huamobile_event_device *dev;

	pr_pos_info();

	keypad = malloc(sizeof(*keypad));
	if (keypad == NULL)
	{
		pr_error_info("malloc");
		return NULL;
	}

	dev = &keypad->dev;
	dev->type = HUA_EVENT_DEVICE_KEYPAD;
	dev->probe = huamobile_keypad_probe;
	dev->remove = huamobile_keypad_remove;
	dev->destroy = huamobile_keypad_destroy;
	dev->event_handler = huamobile_keypad_event_handler;

	return dev;
}
