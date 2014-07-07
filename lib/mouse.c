/*
 * File:		mouse.c
 * Author:		Fuang Cao <cavan.cfa@gmail.com>
 * Created:		2012-11-17
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

#include <cavan/mouse.h>
#include <cavan/input.h>

bool cavan_mouse_device_match(uint8_t *key_bitmask, uint8_t *rel_bitmask)
{
	if (test_bit(BTN_LEFT, key_bitmask) == 0 || \
		test_bit(BTN_RIGHT, key_bitmask) == 0 || \
		test_bit(REL_X, rel_bitmask) == 0 || \
		test_bit(REL_Y, rel_bitmask) == 0)
	{
		return false;
	}

	clean_bit(BTN_LEFT, key_bitmask);
	clean_bit(BTN_RIGHT, key_bitmask);
	clean_bit(BTN_MIDDLE, key_bitmask);
	clean_bit(REL_X, rel_bitmask);
	clean_bit(REL_Y, rel_bitmask);

	return true;
}

bool cavan_mouse_device_matcher(struct cavan_event_matcher *matcher, void *data)
{
	int ret;
	uint8_t key_bitmask[KEY_BITMASK_SIZE];
	uint8_t rel_bitmask[REL_BITMASK_SIZE];

	ret = cavan_event_get_rel_bitmask(matcher->fd, rel_bitmask);
	if (ret < 0)
	{
		pr_error_info("cavan_event_get_rel_bitmask");
		return ret;
	}

	ret = cavan_event_get_key_bitmask(matcher->fd, key_bitmask);
	if (ret < 0)
	{
		pr_error_info("cavan_event_get_key_bitmask");
		return ret;
	}

	return cavan_mouse_device_match(key_bitmask, rel_bitmask);
}

static bool cavan_mouse_event_handler(struct cavan_input_device *dev, struct input_event *event, void *data)
{
	struct cavan_mouse_device *mouse = (struct cavan_mouse_device *) dev;

	switch (event->type)
	{
	case EV_KEY:
		switch (event->code)
		{
		case BTN_LEFT:
		case BTN_RIGHT:
		case BTN_MIDDLE:
			cavan_input_service_append_key_message(data, \
					CAVAN_INPUT_MESSAGE_MOUSE_TOUCH, NULL, event->code, event->value);
			break;

		default:
			return false;
		}
		break;

	case EV_REL:
		switch (event->code)
		{
		case REL_X:
			mouse->x = event->value;
			break;

		case REL_Y:
			mouse->y = event->value;
			break;

		case REL_WHEEL:
			cavan_input_service_append_key_message(data, \
					CAVAN_INPUT_MESSAGE_WHEEL, NULL, REL_WHEEL, event->value);
			break;

		default:
			return false;
		}
		break;

	case EV_SYN:
		if (mouse->x || mouse->y)
		{
			if (cavan_input_service_append_vector_message(data, \
					CAVAN_INPUT_MESSAGE_MOUSE_MOVE, mouse->x, mouse->y, 0))
			{
				mouse->x = mouse->y = 0;
			}
		}
		break;

	default:
		return false;
	}

	return true;
}

struct cavan_input_device *cavan_mouse_create(void)
{
	struct cavan_mouse_device *mouse;
	struct cavan_input_device *dev;

	mouse = malloc(sizeof(*mouse));
	if (mouse == NULL)
	{
		pr_error_info("malloc");
		return NULL;
	}

	mouse->x = mouse->y = 0;

	dev = &mouse->input_dev;
	dev->probe = NULL;
	dev->remove = NULL;
	dev->event_handler = cavan_mouse_event_handler;

	return dev;
}
