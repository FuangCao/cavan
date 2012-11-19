/*
 * File:		gsensor.c
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

#include <cavan/gsensor.h>
#include <cavan/input.h>

bool cavan_gsensor_device_match(uint8_t *abs_bitmask)
{
	if (test_bit(ABS_X, abs_bitmask) == 0 || \
		test_bit(ABS_Y, abs_bitmask) == 0 || \
		test_bit(ABS_Z, abs_bitmask) == 0)
	{
		return false;
	}

	clean_bit(ABS_X, abs_bitmask);
	clean_bit(ABS_Y, abs_bitmask);
	clean_bit(ABS_Z, abs_bitmask);

	return true;
}

bool cavan_gsensor_device_matcher(int fd, const char *name, void *data)
{
	int ret;
	uint8_t abs_bitmask[ABS_BITMASK_SIZE];

	pr_pos_info();

	ret = cavan_event_get_abs_bitmask(fd, abs_bitmask);
	if (ret < 0)
	{
		pr_error_info("cavan_event_get_abs_bitmask");
		return ret;
	}

	return cavan_gsensor_device_match(abs_bitmask);
}

static bool cavan_gsensor_event_handler(struct cavan_input_device *dev, struct input_event *event, void *data)
{
	struct cavan_gsensor_device *sensor = (struct cavan_gsensor_device *)dev;
	struct cavan_input_service *service = data;

	switch (event->type)
	{
	case EV_ABS:
		switch (event->code)
		{
		case ABS_X:
			sensor->event.x = event->value;
			break;

		case ABS_Y:
			sensor->event.y = event->value;
			break;

		case ABS_Z:
			sensor->event.z = event->value;
			break;

		default:
			return false;
		}
		break;

	case EV_SYN:
		service->gsensor_handler(dev, &sensor->event, service->private_data);
		break;

	default:
		return false;
	}

	return true;
}

struct cavan_input_device *cavan_gsensor_create(void)
{
	struct cavan_gsensor_device *sensor;
	struct cavan_input_device *dev;
	struct cavan_gsensor_event *event;

	pr_pos_info();

	sensor = malloc(sizeof(*sensor));
	if (sensor == NULL)
	{
		pr_error_info("malloc");
		return NULL;
	}

	event = &sensor->event;
	event->x = event->y = event->z = 0;

	dev = &sensor->input_dev;
	dev->probe = NULL;
	dev->remove = NULL;
	dev->event_handler = cavan_gsensor_event_handler;

	return dev;
}
