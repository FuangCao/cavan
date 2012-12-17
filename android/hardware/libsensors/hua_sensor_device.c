/*
 * File:			hua_sensor_device.c
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

#include <hua_sensor.h>

#define HUA_SENSOR_DEVICE_DEBUG		1

struct sensors_event_t *hua_sensor_device_sync_event(struct hua_sensor_device *head, struct sensors_event_t *data, size_t data_size)
{
	struct sensors_event_t *data_end;
	int64_t timestamp = systemTime(SYSTEM_TIME_MONOTONIC);

	for (data_end = data + data_size; head; head = head->next)
	{
		if (head->updated)
		{
#if HUA_SENSOR_DEVICE_DEBUG
			float *value = head->event.data;

			pr_func_info("%s: [%f, %f, %f]", head->name, value[0], value[1], value[2]);
#endif

			if (data < data_end)
			{
				head->updated = false;
				*data = head->event;
				data->timestamp = timestamp;
			}
			else
			{
				break;
			}

			data++;
		}
	}

	return data;
}

static bool hua_sensor_device_report_event_single(struct hua_sensor_device *sensor, struct input_event *event)
{
	int code = event->code;

	if (code == sensor->xcode)
	{
		sensor->event.data[0] = event->value * sensor->scale;
	}
	else if (code == sensor->ycode)
	{
		sensor->event.data[1] = event->value * sensor->scale;
	}
	else if (code == sensor->zcode)
	{
		sensor->event.data[2] = event->value * sensor->scale;
	}
	else
	{
		return false;
	}

	return true;
}

bool hua_sensor_device_report_event(struct hua_sensor_device *head, struct input_event *event)
{
	while (head)
	{
		if (hua_sensor_device_report_event_single(head, event))
		{
			head->updated = true;
			return true;
		}

		head = head->next;
	}

	return false;
}

int hua_sensor_device_init(struct hua_sensor_device *sensor, struct hua_sensor_chip *chip, int index)
{
	int ret;
	int ctrl_fd = chip->ctrl_fd;

	ret = ioctl(ctrl_fd, HUA_SENSOR_IOC_GET_SENSOR_TYPE(index), &sensor->type);
	if (ret < 0)
	{
		pr_error_info("ioctl HUA_SENSOR_IOC_GET_SENSOR_TYPE");
		return ret;
	}

	ret = ioctl(ctrl_fd, HUA_SENSOR_IOC_GET_SENSOR_NAME(index, sizeof(sensor->name)), sensor->name);
	if (ret < 0)
	{
		pr_error_info("ioctl HUA_SENSOR_IOC_GET_SENSOR_NAME");
		return ret;
	}

	ret = ioctl(ctrl_fd, HUA_SENSOR_IOC_GET_XCODE(index), &sensor->xcode);
	if (ret < 0)
	{
		pr_error_info("ioctl HUA_SENSOR_IOC_GET_MAX_RANGE");
		return ret;
	}

	ret = ioctl(ctrl_fd, HUA_SENSOR_IOC_GET_YCODE(index), &sensor->ycode);
	if (ret < 0)
	{
		pr_error_info("ioctl HUA_SENSOR_IOC_GET_MAX_RANGE");
		return ret;
	}

	ret = ioctl(ctrl_fd, HUA_SENSOR_IOC_GET_ZCODE(index), &sensor->zcode);
	if (ret < 0)
	{
		pr_error_info("ioctl HUA_SENSOR_IOC_GET_MAX_RANGE");
		return ret;
	}

	sensor->index = index;
	sensor->chip = chip;
	sensor->enabled = false;
	sensor->updated = false;

	return 0;
}

int hua_sensor_device_probe(struct hua_sensor_device *sensor, struct sensor_t *hal_sensor)
{
	int ret;
	unsigned int max_range;
	unsigned int power_consume;
	unsigned int resolution;
	int index = sensor->index;
	int ctrl_fd = sensor->chip->ctrl_fd;
	struct sensors_event_t *event = &sensor->event;

	ret = ioctl(ctrl_fd, HUA_SENSOR_IOC_GET_MAX_RANGE(index), &max_range);
	if (ret < 0)
	{
		pr_error_info("ioctl HUA_SENSOR_IOC_GET_MAX_RANGE");
		return ret;
	}

	ret = ioctl(ctrl_fd, HUA_SENSOR_IOC_GET_RESOLUTION(index), &resolution);
	if (ret < 0)
	{
		pr_error_info("ioctl HUA_SENSOR_IOC_GET_RESOLUTION");
		return ret;
	}

	ret = ioctl(ctrl_fd, HUA_SENSOR_IOC_GET_POWER_CONSUME(index), &power_consume);
	if (ret < 0)
	{
		pr_error_info("ioctl HUA_SENSOR_IOC_GET_POWER_CONSUME");
		return ret;
	}

	hua_sensor_event_init(event);
	hal_sensor->maxRange = max_range;

	switch (sensor->type)
	{
	case HUA_SENSOR_TYPE_ACCELEROMETER:
		hal_sensor->type = SENSOR_TYPE_ACCELEROMETER;
		hal_sensor->maxRange = max_range * GRAVITY_EARTH;
		break;

	case HUA_SENSOR_TYPE_MAGNETIC_FIELD:
		hal_sensor->type = SENSOR_TYPE_MAGNETIC_FIELD;
		break;

	case HUA_SENSOR_TYPE_ORIENTATION:
		hal_sensor->type = SENSOR_TYPE_ORIENTATION;
		break;

	case HUA_SENSOR_TYPE_GRAVITY:
		hal_sensor->type = SENSOR_TYPE_GRAVITY;
		break;

	case HUA_SENSOR_TYPE_GYROSCOPE:
		hal_sensor->type = SENSOR_TYPE_GYROSCOPE;
		break;

	case HUA_SENSOR_TYPE_ROTATION_VECTOR:
		hal_sensor->type = SENSOR_TYPE_ROTATION_VECTOR;
		break;

	case HUA_SENSOR_TYPE_LIGHT:
		hal_sensor->type = SENSOR_TYPE_LIGHT;
		break;

	case HUA_SENSOR_TYPE_PRESSURE:
		hal_sensor->type = SENSOR_TYPE_PRESSURE;
		break;

	case HUA_SENSOR_TYPE_TEMPERATURE:
		hal_sensor->type = SENSOR_TYPE_TEMPERATURE;
		break;

	case HUA_SENSOR_TYPE_PROXIMITY:
		hal_sensor->type = SENSOR_TYPE_PROXIMITY;
		break;

	default:
		pr_red_info("Invalid sensor type %d", sensor->type);
		return -EINVAL;
	}

	event->type = hal_sensor->type;
	event->acceleration.status = SENSOR_STATUS_ACCURACY_HIGH;

	hal_sensor->resolution = hal_sensor->maxRange / resolution;
	hal_sensor->power = ((float)power_consume) / 1000;

	return 0;
}
