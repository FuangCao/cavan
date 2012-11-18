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

bool huamobile_keypad_device_match(uint8_t *key_bitmask, size_t size)
{
	const uint8_t *key_end;
	bool result = false;

	for (key_end = key_bitmask + size; key_bitmask < key_end; key_bitmask++)
	{
		if (*key_bitmask)
		{
			result = true;
			*key_bitmask = 0;
		}
	}

	return result;
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

static bool huamobile_keypad_event_handler(struct huamobile_input_device *dev, struct input_event *event, void *data)
{
	struct huamobile_input_service *service = data;
	const char *keyname;

	switch (event->type)
	{
	case EV_KEY:
		keyname = huamobile_event_find_key_name(dev->event_dev, event->code);
		service->key_handler(dev, keyname, event->code, event->value, service->private_data);
		break;

	case EV_SYN:
		break;

	default:
		return false;
	}

	return true;
}

struct huamobile_input_device *huamobile_keypad_create(void)
{
	struct huamobile_keypad_device *keypad;
	struct huamobile_input_device *dev;

	pr_pos_info();

	keypad = malloc(sizeof(*keypad));
	if (keypad == NULL)
	{
		pr_error_info("malloc");
		return NULL;
	}

	dev = &keypad->dev;
	dev->probe = NULL;
	dev->remove = NULL;
	dev->event_handler = huamobile_keypad_event_handler;

	return dev;
}
