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

bool cavan_gsensor_device_matcher(struct cavan_event_matcher *matcher, void *data)
{
	int ret;
	uint8_t abs_bitmask[ABS_BITMASK_SIZE];

	ret = cavan_event_get_abs_bitmask(matcher->fd, abs_bitmask);
	if (ret < 0)
	{
		pr_error_info("cavan_event_get_abs_bitmask");
		return ret;
	}

	return cavan_gsensor_device_match(abs_bitmask);
}

static bool cavan_gsensor_event_handler(struct cavan_input_device *dev, struct input_event *event, void *data)
{
	cavan_input_message_t *message;
	struct cavan_input_service *service = data;
	struct cavan_gsensor_device *sensor = (struct cavan_gsensor_device *) dev;

	switch (event->type)
	{
	case EV_ABS:
		switch (event->code)
		{
		case ABS_X:
			sensor->vector.x = event->value;
			break;

		case ABS_Y:
			sensor->vector.y = event->value;
			break;

		case ABS_Z:
			sensor->vector.z = event->value;
			break;

		default:
			return false;
		}
		break;

	case EV_SYN:
		message = cavan_data_queue_get_node(&service->queue);
		if (message)
		{
			message->type = CAVAN_INPUT_MESSAGE_ACCELEROMETER;
			message->vector = sensor->vector;
			cavan_data_queue_append(&service->queue, &message->node);
		}
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
	struct cavan_input_message_vector *vector;

	sensor = malloc(sizeof(*sensor));
	if (sensor == NULL)
	{
		pr_error_info("malloc");
		return NULL;
	}

	vector = &sensor->vector;
	vector->x = vector->y = vector->z = 0;

	dev = &sensor->input_dev;
	dev->probe = NULL;
	dev->remove = NULL;
	dev->event_handler = cavan_gsensor_event_handler;

	return dev;
}
