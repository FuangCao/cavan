/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LOG_TAG
#define LOG_TAG "Sensors"
#endif

#include <cavan.h>
#include <cavan/event.h>
#include <linux/input.h>
#include <utils/Log.h>
#include <utils/Atomic.h>
#include <hardware/sensors.h>

#define HUA_MAX_SENSOR_COUNT	8

#define HUA_SENSOR_IOCG_MAX_RANGE		_IOR('H', 0x01, unsigned int)
#define HUA_SENSOR_IOCG_RESOLUTION		_IOR('H', 0x02, unsigned int)
#define HUA_SENSOR_IOCG_POWER_CONSUME	_IOR('H', 0x03, unsigned int)
#define HUA_SENSOR_IOCG_MIN_DELAY		_IOR('H', 0x04, unsigned int)

#define HUA_SENSOR_IOCS_ENABLE			_IOW('H', 0x05, unsigned int)
#define HUA_SENSOR_IOCS_DELAY			_IOW('H', 0x06, unsigned int)

struct hua_sensor_device
{
	int input_fd;
	int ctrl_fd;
	char name[128];
	struct sensors_event_t event;
	struct sensor_t *sensor;
	struct pollfd *pfd;

	int (*probe)(struct hua_sensor_device *dev, struct sensor_t *sensor);
	void (*remove)(struct hua_sensor_device *dev);
	bool (*event_handler)(struct hua_sensor_device *dev, struct input_event *event);
};

struct hua_gsensor_device
{
	struct hua_sensor_device device;

	float scale;
};

struct hua_sensors_poll_device
{
	struct sensors_poll_device_t device;

	size_t sensor_count;
	struct sensor_t *sensor_list;
	int pipefd[2];
	struct pollfd *pfds;
	struct hua_sensor_device *sensors[HUA_MAX_SENSOR_COUNT];
};

// ================================================================================

void hua_sensor_event_init(struct sensors_event_t *event)
{
	memset(event, 0, sizeof(*event));
	event->version = sizeof(*event);
}

// ================================================================================

static int hua_gsensor_probe(struct hua_sensor_device *dev, struct sensor_t *sensor)
{
	int ret;
	int fd;
	struct hua_gsensor_device *gsensor = (struct hua_gsensor_device *)dev;
	unsigned int range, resolution, power, delay;

	fd = dev->ctrl_fd;
	if (fd < 0)
	{
		pr_red_info("fd < 0");
		return -EINVAL;
	}

	ret = ioctl(fd, HUA_SENSOR_IOCG_MAX_RANGE, &range);
	if (ret < 0)
	{
		pr_error_info("ioctl HUA_SENSOR_IOCG_MAX_RANGE");
		return ret;
	}

	ret = ioctl(fd, HUA_SENSOR_IOCG_RESOLUTION, &resolution);
	if (ret < 0)
	{
		pr_error_info("ioctl HUA_SENSOR_IOCG_RESOLUTION");
		return ret;
	}

	ret = ioctl(fd, HUA_SENSOR_IOCG_POWER_CONSUME, &power);
	if (ret < 0)
	{
		pr_error_info("ioctl HUA_SENSOR_IOCG_POWER_CONSUME");
		power = 0;
	}

	ret = ioctl(fd, HUA_SENSOR_IOCG_MIN_DELAY, &delay);
	if (ret < 0)
	{
		pr_error_info("ioctl HUA_SENSOR_IOCG_MIN_DELAY");
		delay = 0;
	}

	pr_green_info("range = %d, resolution = %d", range, resolution);
	pr_green_info("power = %d, delay = %d", power, delay);

	sensor->name = "Huamobile 3-AXIS Accelerometer";
	sensor->vendor = "Huamobile";
	sensor->type = SENSOR_TYPE_ACCELEROMETER;
	sensor->maxRange = range * GRAVITY_EARTH;
	sensor->resolution = sensor->maxRange / resolution;
	sensor->power = ((float)power) / 1000;
	sensor->minDelay = delay;

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

static struct hua_sensor_device *hua_gsensor_create(void)
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

// ================================================================================

static struct hua_sensor_device *hua_sensor_create(int fd, const char *name)
{
	struct hua_sensor_device *sensor;
	char ctrl_path[1024];

	if (strcmp(name, "accelerometer") == 0)
	{
		sensor = hua_gsensor_create();
	}
	else
	{
		return NULL;
	}

	sensor->input_fd = fd;
	text_ncopy(sensor->name, name, sizeof(sensor->name));

	sprintf(ctrl_path, "/dev/%s", name);
	pr_func_info("ctrl_path = %s", ctrl_path);

	sensor->ctrl_fd = open(ctrl_path, 0);
	if (sensor->ctrl_fd < 0)
	{
		pr_error_info("open `%s'", ctrl_path);
	}

	return sensor;
}

static void hua_sensor_close(struct hua_sensor_device *sensor)
{
	if (sensor->ctrl_fd > 0)
	{
		close(sensor->ctrl_fd);
	}

	if (sensor->input_fd > 0)
	{
		close(sensor->input_fd);
	}
}

static void hua_sensors_free(struct hua_sensor_device *sensors[], size_t size)
{
	unsigned int i;

	for (i = 0; i < size; i++)
	{
		if (sensors[i] == NULL)
		{
			continue;
		}

		hua_sensor_close(sensors[i]);
		free(sensors[i]);
		sensors[i] = NULL;
	}
}

static void hua_sensors_remove(struct hua_sensor_device *sensors[], size_t size)
{
	unsigned int i;

	for (i = 0; i < size; i++)
	{
		if (sensors[i] && sensors[i]->remove)
		{
			sensors[i]->remove(sensors[i]);
		}
	}
}

static int hua_sensors_close(struct hua_sensors_poll_device *poll_device)
{
	hua_sensors_remove(poll_device->sensors, NELEM(poll_device->sensors));
	hua_sensors_free(poll_device->sensors, NELEM(poll_device->sensors));

	if (poll_device->sensor_list)
	{
		free(poll_device->sensor_list);
		poll_device->sensor_list = NULL;
	}

	poll_device->sensor_count = 0;

	return 0;
}

static int hua_sensors_get_free_index(struct hua_sensor_device *sensors[], size_t size)
{
	unsigned int i;

	for (i = 0; i < size; i++)
	{
		if (sensors[i] == NULL)
		{
			return i;
		}
	}

	return -1;
}

static int hua_sensors_match_handler(struct cavan_event_matcher *matcher, void *data)
{
	int index;
	struct hua_sensor_device *sensor;
	struct hua_sensors_poll_device *poll_device = data;

	index = hua_sensors_get_free_index(poll_device->sensors, NELEM(poll_device->sensors));
	if (index < 0)
	{
		pr_red_info("hua_sensors_get_free_index");
		return -ENOMEDIUM;
	}

	pr_green_info("index = %d", index);

	sensor = hua_sensor_create(matcher->fd, matcher->devname);
	if (sensor == NULL)
	{
		pr_red_info("Can't recognize device `%s', pathname = %s", matcher->devname, matcher->pathname);
		return -EFAULT;
	}

	pr_green_info("Device[%d] `%s' create complete, pathname = %s", index, matcher->devname, matcher->pathname);

	poll_device->sensors[index] = sensor;

	return 0;
}

static int hua_sensors_open(struct hua_sensors_poll_device *poll_device)
{
	unsigned int i;
	int ret;
	size_t count;
	struct sensor_t *sensor;
	struct pollfd *pfd;
	struct hua_sensor_device **sensors = poll_device->sensors;
	struct cavan_event_matcher matcher =
	{
 		.match = NULL,
 		.handler = hua_sensors_match_handler
	};

	count = cavan_event_scan_devices(&matcher, poll_device);
	if (count == 0)
	{
		pr_red_info("No device matched");
		ret = -ENOENT;
		goto out_clean_sensor_count;
	}

	sensor = malloc((sizeof(*sensor) * count) + (sizeof(*pfd) * (count + 1)));
	if (sensor == NULL)
	{
		pr_error_info("malloc");
		ret = -ENOMEM;
		goto out_hua_sensors_free;
	}

	pfd = (struct pollfd *)(sensor + count);
	poll_device->sensor_list = sensor;
	poll_device->pfds = pfd;

	ret = pipe(poll_device->pipefd);
	if (ret < 0)
	{
		pr_error_info("pipe");
		goto out_free_sensor_list;
	}

	pfd->events = POLLIN;
	pfd->fd = poll_device->pipefd[0];
	pfd->revents = 0;

	for (i = 0; i < NELEM(poll_device->sensors); i++)
	{
		if (sensors[i] == NULL)
		{
			pr_bold_info("sensors[%d] == NULL", i);
			continue;
		}

		if (sensors[i]->probe && sensors[i]->probe(sensors[i], sensor) < 0)
		{
			pr_red_info("Probe failed");

			free(sensors[i]);
			sensors[i] = NULL;
			continue;
		}

		sensor->version = 1;
		sensor->handle = i;
		sensors[i]->sensor = sensor;

		pfd++;
		pfd->events = POLLIN;
		pfd->fd = sensors[i]->input_fd;
		pfd->revents = 0;
		sensors[i]->pfd = pfd;

		sensor++;
	}

	count = sensor - poll_device->sensor_list;
	if (count == 0)
	{
		pr_red_info("count == 0");
		ret = -ENOENT;
		goto out_close_pipe;
	}

	poll_device->sensor_count = count;

	return 0;

out_close_pipe:
	close(poll_device->pipefd[0]);
	close(poll_device->pipefd[1]);
out_free_sensor_list:
	free(poll_device->sensor_list);
out_hua_sensors_free:
	hua_sensors_free(sensors, NELEM(poll_device->sensors));
out_clean_sensor_count:
	poll_device->sensor_count = 0;
	poll_device->sensor_list = NULL;
	poll_device->pfds = NULL;
	return ret;
}

static int hua_sensors_activate(struct sensors_poll_device_t *dev, int handle, int enabled)
{
	pr_func_info("dev = %p, handle = %d, enable = %d", dev, handle, enabled);

	return 0;
}

static int hua_sensors_setDelay(struct sensors_poll_device_t *dev, int handle, int64_t ns)
{
	pr_func_info("dev = %p, handle = %d, ns = %Ld", dev, handle, ns);

	return 0;
}

static inline int64_t timeval2nano(struct timeval *time)
{
	return time->tv_sec * 1000000000LL + time->tv_usec * 1000;
}

static size_t hua_sensors_repo_event(struct hua_sensor_device *sensors[], size_t size, sensors_event_t *data, int count)
{
	int i;
	ssize_t rdlen;
	struct input_event events[8], *ep, *ep_end;
	struct hua_sensor_device *sensor;
	sensors_event_t *pd, *pd_last;

	for (i = size - 1, pd = data, pd_last = pd + count - 1; i >= 0; i--)
	{
		sensor = sensors[i];

		if (sensor == NULL || sensor->pfd->revents == 0)
		{
			continue;
		}

		rdlen = read(sensor->input_fd, events, sizeof(events));
		if (rdlen < 0)
		{
			pr_error_info("read");
			return rdlen;
		}

		for (ep = events, ep_end = ep + (rdlen / sizeof(*ep)); ep < ep_end; ep++)
		{
			if (sensor->event_handler(sensor, ep) == false)
			{
				continue;
			}

			memcpy(pd, &sensor->event, sizeof(*pd));
			pd->timestamp = timeval2nano(&ep->time);

			if (++pd > pd_last)
			{
				break;
			}
		}
	}

	return pd - data;
}

static int hua_sensors_poll(struct sensors_poll_device_t *dev, sensors_event_t *data, int count)
{
	int ret;
	struct hua_sensors_poll_device *poll_dev = (struct hua_sensors_poll_device *)dev;;
	struct pollfd *pfds = poll_dev->pfds;
	size_t event_count;

	while (1)
	{
		ret = poll(pfds, poll_dev->sensor_count + 1, -1);
		if (ret < 0)
		{
			pr_error_info("poll");
			return ret;
		}

		if (pfds->revents)
		{
			char value;
			ssize_t rdlen;

			rdlen = read(pfds->fd, &value, 1);
			if (rdlen < 0)
			{
				pr_error_info("read wake event");
				return rdlen;
			}

			pr_green_info("Sensors Wakeup Event %d", value);
			break;
		}

		event_count = hua_sensors_repo_event(poll_dev->sensors, poll_dev->sensor_count, data, count);
		if (event_count)
		{
			return event_count;
		}
	}

	return 0;
}

// ================================================================================

static int hua_sensors_module_close(struct hw_device_t *device)
{
	pr_func_info("device = %p", device);

	return hua_sensors_close((struct hua_sensors_poll_device *)device);
}

static struct hua_sensors_poll_device hua_poll_device =
{
	.device =
	{
		.common =
		{
			.tag = HARDWARE_DEVICE_TAG,
			.version = 0,
			.close = hua_sensors_module_close
		},
		.activate = hua_sensors_activate,
		.setDelay = hua_sensors_setDelay,
		.poll = hua_sensors_poll
	},
};

static int hua_sensors_module_open(const struct hw_module_t *module, const char *id, struct hw_device_t **device)
{
	int ret;
	struct hw_device_t *hw_dev;

	pr_func_info("module = %p, id = %s", module, id);

	ret = hua_sensors_open(&hua_poll_device);
	if (ret < 0)
	{
		pr_red_info("hua_sensors_open");
		return ret;
	}

	hw_dev = &hua_poll_device.device.common;
	hw_dev->module = (struct hw_module_t *)module;
	*device = hw_dev;

	return 0;
}

static int hua_sensors_get_list(struct sensors_module_t *module, struct sensor_t const **list) 
{
	pr_func_info("module = %p, sensor_count = %d", module, hua_poll_device.sensor_count);

	*list = hua_poll_device.sensor_list;

	return hua_poll_device.sensor_count;
}

static struct hw_module_methods_t hua_sensors_module_methods =
{
	.open = hua_sensors_module_open
};

struct sensors_module_t HAL_MODULE_INFO_SYM =
{
	.common =
	{
		.tag = HARDWARE_MODULE_TAG,
		.version_major = 1,
		.version_minor = 0,
		.id = SENSORS_HARDWARE_MODULE_ID,
		.name = "Huamobile Sensor HAL",
		.author = "Fuang.Cao",
		.methods = &hua_sensors_module_methods
	},

	.get_sensors_list = hua_sensors_get_list
};
