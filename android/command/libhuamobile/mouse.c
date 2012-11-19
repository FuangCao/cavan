/*
 * File:		mouse.c
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

#include <huamobile/mouse.h>
#include <huamobile/input.h>

bool huamobile_mouse_device_match(uint8_t *key_bitmask, uint8_t *rel_bitmask)
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
	clean_bit(REL_X, rel_bitmask);
	clean_bit(REL_Y, rel_bitmask);

	return true;
}

bool huamobile_mouse_device_matcher(int fd, const char *name, void *data)
{
	int ret;
	uint8_t key_bitmask[KEY_BITMASK_SIZE];
	uint8_t rel_bitmask[REL_BITMASK_SIZE];

	pr_pos_info();

	ret = huamobile_event_get_rel_bitmask(fd, rel_bitmask);
	if (ret < 0)
	{
		pr_error_info("huamobile_event_get_rel_bitmask");
		return ret;
	}

	ret = huamobile_event_get_key_bitmask(fd, key_bitmask);
	if (ret < 0)
	{
		pr_error_info("huamobile_event_get_key_bitmask");
		return ret;
	}

	return huamobile_mouse_device_match(key_bitmask, rel_bitmask);
}

static bool huamobile_mouse_event_handler(struct huamobile_input_device *dev, struct input_event *event, void *data)
{
	struct huamobile_input_service *service = data;
	struct huamobile_mouse_device *mouse = (struct huamobile_mouse_device *)dev;
	struct huamobile_touch_point *p = &mouse->point;

	switch (event->type)
	{
	case EV_KEY:
		switch (event->code)
		{
		case BTN_LEFT:
			if (event->value)
			{
				service->touch_handler(dev, p, service->private_data);
			}
			else
			{
				service->release_handler(dev, p, service->private_data);
			}
			break;

		case BTN_RIGHT:
			if (event->value)
			{
				service->right_touch_handler(dev, p, service->private_data);
			}
			else
			{
				service->right_release_handler(dev, p, service->private_data);
			}
			break;

		default:
			return false;
		}
		break;

	case EV_REL:
		switch (event->code)
		{
		case REL_X:
			p->x += event->value * service->mouse_speed;
			if (p->x < 0)
			{
				p->x = 0;
			}
			else if (p->x > mouse->xmax)
			{
				p->x = mouse->xmax;
			}
			break;

		case REL_Y:
			p->y += event->value * service->mouse_speed;
			if (p->y < 0)
			{
				p->y = 0;
			}
			else if (p->y > mouse->ymax)
			{
				p->y = mouse->ymax;
			}
			break;

		case REL_WHEEL:
			service->wheel_handler(dev, event->value, service->private_data);
			break;

		default:
			return false;
		}
		break;

	case EV_SYN:
		if (p->x != mouse->xold || p->y != mouse->yold)
		{
			service->move_handler(dev, p, service->private_data);

			mouse->xold = p->x;
			mouse->yold = p->y;
		}
		break;

	default:
		return false;
	}

	return true;
}

static int huamobile_mouse_probe(struct huamobile_input_device *dev, void *data)
{
	struct huamobile_input_service *service = data;
	struct huamobile_mouse_device *mouse = (struct huamobile_mouse_device *)dev;

	pr_pos_info();

	if (service->lcd_width <= 0 || service->lcd_height <= 0)
	{
		pr_red_info("service->lcd_width <= 0 || service->lcd_height <= 0");
		return -EINVAL;
	}

	mouse->xmax = service->lcd_width - 1;
	mouse->ymax = service->lcd_height - 1;

	return 0;
}

struct huamobile_input_device *huamobile_mouse_create(void)
{
	struct huamobile_mouse_device *mouse;
	struct huamobile_input_device *dev;
	struct huamobile_touch_point *point;

	pr_pos_info();

	mouse = malloc(sizeof(*mouse));
	if (mouse == NULL)
	{
		pr_error_info("malloc");
		return NULL;
	}

	mouse->xold = 0;
	mouse->yold = 0;

	point = &mouse->point;
	point->id = 0;
	point->x = 0;
	point->y = 0;
	point->pressure = 0;
	point->released = 1;

	dev = &mouse->input_dev;
	dev->probe = huamobile_mouse_probe;
	dev->remove = NULL;
	dev->event_handler = huamobile_mouse_event_handler;

	return dev;
}
