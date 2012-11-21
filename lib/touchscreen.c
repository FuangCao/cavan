/*
 * File:		touchscreen.c
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

#include <cavan/touchscreen.h>
#include <cavan/input.h>

bool cavan_multi_touch_device_match(uint8_t *abs_bitmask)
{
	if (test_bit(ABS_MT_POSITION_X, abs_bitmask) == 0 || test_bit(ABS_MT_POSITION_Y, abs_bitmask) == 0)
	{
		return false;
	}

	clean_bit(ABS_MT_POSITION_X, abs_bitmask);
	clean_bit(ABS_MT_POSITION_Y, abs_bitmask);

	return true;
}

bool cavan_multi_touch_device_matcher(struct cavan_event_matcher *matcher, void *data)
{
	int ret;
	uint8_t abs_bitmask[ABS_BITMASK_SIZE];

	ret = cavan_event_get_abs_bitmask(matcher->fd, abs_bitmask);
	if (ret < 0)
	{
		pr_error_info("cavan_event_get_abs_bitmask");
		return false;
	}

	return cavan_multi_touch_device_match(abs_bitmask);
}

bool cavan_single_touch_device_match(uint8_t *abs_bitmask, uint8_t *key_bitmask)
{
	if (test_bit(BTN_TOUCH, key_bitmask) == 0 || test_bit(ABS_X, abs_bitmask) == 0 || test_bit(ABS_Y, abs_bitmask) == 0)
	{
		return false;
	}

	clean_bit(BTN_TOUCH, key_bitmask);
	clean_bit(ABS_X, abs_bitmask);
	clean_bit(ABS_Y, abs_bitmask);

	return true;
}

bool cavan_single_touch_device_matcher(struct cavan_event_matcher *matcher, void *data)
{
	int ret;
	uint8_t abs_bitmask[ABS_BITMASK_SIZE];
	uint8_t key_bitmask[KEY_BITMASK_SIZE];

	ret = cavan_event_get_abs_bitmask(matcher->fd, abs_bitmask);
	if (ret < 0)
	{
		pr_error_info("cavan_event_get_abs_bitmask");
		return false;
	}

	ret = cavan_event_get_key_bitmask(matcher->fd, key_bitmask);
	if (ret < 0)
	{
		pr_error_info("cavan_event_get_key_bitmask");
		return false;
	}

	return cavan_single_touch_device_match(abs_bitmask, key_bitmask);
}

bool cavan_touch_device_matcher(struct cavan_event_matcher *matcher, void *data)
{
	int ret;
	uint8_t abs_bitmask[ABS_BITMASK_SIZE];
	uint8_t key_bitmask[KEY_BITMASK_SIZE];

	ret = cavan_event_get_abs_bitmask(matcher->fd, abs_bitmask);
	if (ret < 0)
	{
		pr_error_info("cavan_event_get_abs_bitmask");
		return false;
	}

	if (cavan_multi_touch_device_match(abs_bitmask))
	{
		pr_green_info("Deivce %s mutil touch screen", matcher->devname);
		return true;
	}

	ret = cavan_event_get_key_bitmask(matcher->fd, key_bitmask);
	if (ret < 0)
	{
		pr_error_info("cavan_event_get_key_bitmask");
		return false;
	}

	return cavan_single_touch_device_match(abs_bitmask, key_bitmask);
}

static inline void cavan_touch_point_mapping(struct cavan_touch_device *dev, struct cavan_touch_point *point)
{
	point->x = point->x * dev->xscale - dev->xoffset;
	point->y = point->y * dev->yscale - dev->yoffset;
}

static int cavan_touch_device_probe(struct cavan_touch_device *dev, void *data)
{
	int ret;
	int min, max, diff;
	int fd = dev->input_dev.event_dev->fd;
	struct cavan_input_service *service = data;

	pr_bold_info("LCD: width = %d, height = %d", service->lcd_width, service->lcd_height);

	if (service->lcd_width > 0)
	{
		ret = cavan_event_get_absinfo(fd, dev->xaxis, &min, &max);
		if (ret < 0)
		{
			pr_red_info("cavan_event_get_absinfo");
			return ret;
		}

		pr_bold_info("x-min = %d, x-max = %d", min, max);
		diff = max - min;
		dev->xscale = ((double)service->lcd_width) / diff;
		dev->xoffset = ((double)service->lcd_width) * min / diff;
	}
	else
	{
		dev->xscale = 1;
		dev->xoffset = 0;
	}

	if (service->lcd_height > 0)
	{
		ret = cavan_event_get_absinfo(fd, dev->yaxis, &min, &max);
		if (ret < 0)
		{
			pr_red_info("cavan_event_get_absinfo");
			return ret;
		}

		pr_bold_info("y-min = %d, y-max = %d", min, max);
		diff = max - min;
		dev->yscale = ((double)service->lcd_height) / diff;
		dev->yoffset = ((double)service->lcd_height) * min / diff;
	}
	else
	{
		dev->yscale = 1;
		dev->yoffset = 0;
	}

	pr_bold_info("xscale = %lf, xoffset = %lf", dev->xscale, dev->xoffset);
	pr_bold_info("yscale = %lf, yoffset = %lf", dev->yscale, dev->yoffset);

	return 0;
}

static bool cavan_multi_touch_event_handler(struct cavan_input_device *dev, struct input_event *event, void *data)
{
	struct cavan_multi_touch_device *ts = (struct cavan_multi_touch_device *)dev;
	struct cavan_input_service *service = data;
	struct cavan_touch_point *p, *p_end;

	switch (event->type)
	{
	case EV_ABS:
		switch (event->code)
		{
		case ABS_MT_WIDTH_MAJOR:
			break;

		case ABS_MT_POSITION_X:
			ts->points[ts->point_count].x = event->value;
			break;

		case ABS_MT_POSITION_Y:
			ts->points[ts->point_count].y = event->value;
			break;

		case ABS_MT_TOUCH_MAJOR:
			ts->points[ts->point_count].pressure = event->value;
			break;

		case ABS_MT_TRACKING_ID:
			ts->points[ts->point_count].id = event->value;
			break;

		default:
			return false;
		}
		break;

	case EV_SYN:
		switch (event->code)
		{
		case SYN_MT_REPORT:
			p = ts->points + ts->point_count;
			if (p->pressure >= 0)
			{
				ts->point_count++;
			}
			break;

		case SYN_REPORT:
			if (ts->point_count)
			{
				struct cavan_virtual_key *key;

				for (p = ts->points, p_end = p + ts->point_count; p < p_end; p++)
				{
					key = cavan_event_find_virtual_key(dev->event_dev, p->x, p->y);
					if (key)
					{
						int value = p->pressure > 0;

						if (key->value != value)
						{
							service->key_handler(dev, key->name, key->code, value, service->private_data);
							key->value = value;
						}

						ts->point_count--;
					}
					else if (p->pressure > 0)
					{
						cavan_touch_point_mapping(&ts->touch_dev, p);

						if (p->released)
						{
							service->touch_handler(dev, p, service->private_data);
							p->released = 0;
						}

						service->move_handler(dev, p, service->private_data);
					}
					else
					{
						ts->point_count--;
					}

					p->pressure = -1;
				}
			}
			else
			{
				struct cavan_virtual_key *key;

				for (key = dev->event_dev->vk_head; key; key = key->next)
				{
					if (key->value)
					{
						service->key_handler(dev, key->name, key->code, 0, service->private_data);
						key->value = 0;
					}
				}
			}

			if (ts->point_count < ts->point_count_old)
			{
				for (p = ts->points + ts->point_count, p_end = p + ts->point_count_old; p < p_end; p++)
				{
					if (p->released == 0)
					{
						service->touch_handler(dev, p, service->private_data);
						p->released = 1;
					}
				}
			}

			ts->point_count_old = ts->point_count;
			ts->point_count = 0;
			break;

		default:
			return false;
		}
		break;

	default:
		return false;
	}

	return true;
}

static int cavan_multi_touch_device_probe(struct cavan_input_device *dev, void *data)
{
	struct cavan_multi_touch_device *ts = (struct cavan_multi_touch_device *)dev;

	return cavan_touch_device_probe(&ts->touch_dev, data);
}

struct cavan_input_device *cavan_multi_touch_device_create(void)
{
	struct cavan_multi_touch_device *ts;
	struct cavan_touch_device *touch_dev;
	struct cavan_input_device *dev;
	struct cavan_touch_point *p, *p_end;

	ts = malloc(sizeof(*ts));
	if (ts == NULL)
	{
		pr_error_info("malloc");
		return NULL;
	}

	ts->point_count = 0;
	ts->point_count_old = 0;

	for (p = ts->points, p_end = p + NELEM(ts->points); p < p_end; p++)
	{
		p->id = p - ts->points;
		p->pressure = -1;
		p->released = 1;
	}

	touch_dev = &ts->touch_dev;
	touch_dev->xaxis = ABS_MT_POSITION_X;
	touch_dev->yaxis = ABS_MT_POSITION_Y;

	dev = &touch_dev->input_dev;
	dev->probe = cavan_multi_touch_device_probe;
	dev->remove = NULL;
	dev->event_handler = cavan_multi_touch_event_handler;

	return dev;
}

// ================================================================================

static bool cavan_single_touch_event_handler(struct cavan_input_device *dev, struct input_event *event, void *data)
{
	struct cavan_single_touch_device *ts = (struct cavan_single_touch_device *)dev;
	struct cavan_input_service *service = data;
	struct cavan_touch_point *p = &ts->point;

	switch (event->type)
	{
	case EV_KEY:
		switch (event->code)
		{
		case BTN_TOUCH:
			ts->pressed = event->value;
			break;

		default:
			return false;
		}
		break;

	case EV_ABS:
		switch (event->code)
		{
		case ABS_X:
			p->x = event->value;
			break;

		case ABS_Y:
			p->y = event->value;
			break;

		case ABS_PRESSURE:
			p->pressure = event->value;
			break;

		default:
			return false;
		}
		break;

	case EV_SYN:
		if (ts->pressed)
		{
			struct cavan_virtual_key *key;
			key = cavan_event_find_virtual_key(dev->event_dev, p->x, p->y);
			if (key)
			{
				int value = p->pressure > 0;

				if (key->value != value)
				{
					service->key_handler(dev, key->name, key->code, value, service->private_data);
					key->value = value;
				}

			}
			else
			{
				cavan_touch_point_mapping(&ts->touch_dev, p);

				if (p->released)
				{
					service->touch_handler(dev, p, service->private_data);
					p->released = 0;
				}

				service->move_handler(dev, p, service->private_data);
			}
		}
		else
		{
			struct cavan_virtual_key *key;

			for (key = dev->event_dev->vk_head; key; key = key->next)
			{
				if (key->value != 0)
				{
					service->key_handler(dev, key->name, key->code, 0, service->private_data);
					key->value = 0;
				}
			}

			if (p->released == 0)
			{
				service->touch_handler(dev, p, service->private_data);
				p->released = 1;
			}
		}
		break;

	default:
		return false;
	}

	return true;
}

static int cavan_single_touch_device_probe(struct cavan_input_device *dev, void *data)
{
	struct cavan_single_touch_device *ts = (struct cavan_single_touch_device *)dev;

	return cavan_touch_device_probe(&ts->touch_dev, data);
}

struct cavan_input_device *cavan_single_touch_device_create(void)
{
	struct cavan_single_touch_device *ts;
	struct cavan_touch_device *touch_dev;
	struct cavan_input_device *dev;

	ts = malloc(sizeof(*ts));
	if (ts == NULL)
	{
		pr_error_info("malloc");
		return NULL;
	}

	ts->pressed = 0;
	ts->point.released = 1;

	touch_dev = &ts->touch_dev;
	touch_dev->xaxis = ABS_X;
	touch_dev->yaxis = ABS_Y;

	dev = &touch_dev->input_dev;
	dev->probe = cavan_single_touch_device_probe;
	dev->remove = NULL;
	dev->event_handler = cavan_single_touch_event_handler;

	return dev;
}
