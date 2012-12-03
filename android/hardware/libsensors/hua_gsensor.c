/*
 * File:			hua_gsensor.c
 * Author:			Fuang Cao <cavan.cfa@gmail.com>
 *
 * Created:			2012-12-03
 * Description:		Huamobile G-Sensor HAL
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

#include <hua_sensors.h>

static int hua_gsensor_probe(struct hua_sensor_device *dev, struct sensor_t *sensor)
{
	struct hua_gsensor_device *gsensor = (struct hua_gsensor_device *)dev;

	sensor->name = "Huamobile 3-AXIS Accelerometer";
	sensor->vendor = "Huamobile";
	sensor->type = SENSOR_TYPE_ACCELEROMETER;
	sensor->maxRange *= GRAVITY_EARTH;
	sensor->resolution = sensor->maxRange / sensor->resolution;

	gsensor->scale = sensor->resolution;

	return 0;
}

static bool hua_gsensor_event_handler(struct hua_sensor_device *dev, struct input_event *event)
{
	struct hua_gsensor_device *gsensor = (struct hua_gsensor_device *)dev;
	sensors_vec_t *acceleration = &dev->event.acceleration;

	switch (event->type)
	{
	case EV_ABS:
		switch (event->code)
		{
		case ABS_X:
			acceleration->x = event->value * gsensor->scale;
			break;

		case ABS_Y:
			acceleration->y = event->value * gsensor->scale;
			break;

		case ABS_Z:
			acceleration->z = event->value * gsensor->scale;
			break;
		}
		break;

	case EV_SYN:
		return true;
	}

	return false;
}

struct hua_sensor_device *hua_gsensor_create(void)
{
	struct hua_gsensor_device *gsensor;
	struct hua_sensor_device *sensor;
	struct sensors_event_t *event;
	sensors_vec_t *acceleration;

	pr_green_info("Create G-Sensor Device");

	gsensor = malloc(sizeof(*gsensor));
	if (gsensor == NULL)
	{
		pr_error_info("gsensor == NULL");
		return NULL;
	}

	sensor = &gsensor->device;
	sensor->probe = hua_gsensor_probe;
	sensor->remove = NULL;
	sensor->event_handler = hua_gsensor_event_handler;

	event = &sensor->event;
	hua_sensor_event_init(event);
	event->sensor = 0;
	event->type = SENSOR_TYPE_ACCELEROMETER;

	acceleration = &event->acceleration;
	acceleration->status = SENSOR_STATUS_ACCURACY_HIGH;

	return sensor;
}
