/*
 * File:		gsensor.c
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

#include <huamobile/gsensor.h>
#include <huamobile/input.h>

bool huamobile_gsensor_device_matcher(int fd, const char *name, void *data)
{
	int ret;
	uint8_t abs_bitmask[ABS_BITMASK_SIZE];

	pr_pos_info();

	ret = huamobile_event_get_abs_bitmask(fd, abs_bitmask, sizeof(abs_bitmask));
	if (ret < 0)
	{
		pr_error_info("huamobile_event_get_abs_bitmask");
		return ret;
	}

	return huamobile_gsensor_device_match(abs_bitmask);
}

static void huamobile_gsensor_event_handler(struct huamobile_event_device *dev, struct input_event *event, void *data)
{
	struct huamobile_gsensor_device *sensor = (struct huamobile_gsensor_device *)dev;
	struct huamobile_input_service *service = data;

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
		}
		break;

	case EV_SYN:
		service->gsensor_handler(dev, &sensor->event, service->private_data);
		break;
	}
}

static int huamobile_gsensor_probe(struct huamobile_event_device *dev, void *data)
{
	struct huamobile_input_service *service = data;

	pr_pos_info();

	if (service->probe)
	{
		return service->probe(dev, service->private_data);
	}

	return 0;
}

static void huamobile_gsensor_remove(struct huamobile_event_device *dev, void *data)
{
	struct huamobile_input_service *service = data;

	pr_pos_info();

	if (service->remove)
	{
		service->remove(dev, service->private_data);
	}
}

static void huamobile_gsensor_destroy(struct huamobile_event_device *dev, void *data)
{
	pr_pos_info();

	free(dev);
}

struct huamobile_event_device *huamobile_gsensor_create(void *data)
{
	struct huamobile_gsensor_device *sensor;
	struct huamobile_event_device *dev;
	struct huamobile_gsensor_event *event;

	pr_pos_info();

	sensor = malloc(sizeof(*sensor));
	if (sensor == NULL)
	{
		pr_error_info("malloc");
		return NULL;
	}

	event = &sensor->event;
	event->x = event->y = event->z = 0;

	dev = &sensor->dev;
	dev->type = HUA_EVENT_DEVICE_GSENSOR;
	dev->probe = huamobile_gsensor_probe;
	dev->remove = huamobile_gsensor_remove;
	dev->destroy = huamobile_gsensor_destroy;
	dev->event_handler = huamobile_gsensor_event_handler;

	return dev;
}
