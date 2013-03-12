/*
 * File:			hua_sensor_core.c
 * Author:			Fuang Cao <cavan.cfa@gmail.com>
 *
 * Created:			2012-12-03
 * Description:		Huamobile Sensor HAL
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

char *text_copy(char *dest, const char *src)
{
	while ((*dest = *src))
	{
		dest++;
		src++;
	}

	return dest;
}

char *text_ncopy(char *dest, const char *src, size_t size)
{
	const char *end_src;

	for (end_src = src + size - 1; src < end_src && *src; src++, dest++)
	{
		*dest = *src;
	}

	*dest = 0;

	return dest;
}

int text_lhcmp(const char *text1, const char *text2)
{
	while (*text1)
	{
		if (*text1 != *text2)
		{
			return *text1 - *text2;
		}

		text1++;
		text2++;
	}

	return 0;
}

static struct hua_sensor_device *hua_sensor_device_create(int fd, const char *devname)
{
	int ret;
	int ctrl_fd;
	unsigned int type;
	char ctrl_path[1024];
	struct hua_sensor_device *sensor;

	if (text_lhcmp("HUA-", devname))
	{
		return NULL;
	}

	sprintf(ctrl_path, "/dev/%s", devname);
	pr_bold_info("ctrl_path = %s", ctrl_path);

	ctrl_fd = open(ctrl_path, 0);
	if (ctrl_fd < 0)
	{
		pr_error_info("open `%s'", ctrl_path);
		return NULL;
	}

	sensor = malloc(sizeof(*sensor));
	if (sensor == NULL)
	{
		pr_error_info("malloc");
		goto out_close_ctrl_fd;
	}

	ret = ioctl(ctrl_fd, HUA_INPUT_DEVICE_IOC_GET_NAME(sizeof(sensor->name)), sensor->name);
	if (ret < 0)
	{
		pr_error_info("ioctl HUA_INPUT_DEVICE_IOC_GET_NAME");
		goto out_free_sensor;
	}

	ret = ioctl(ctrl_fd, HUA_INPUT_CHIP_IOC_GET_VENDOR(sizeof(sensor->vensor)), sensor->vensor);
	if (ret < 0)
	{
		pr_error_info("ioctl HUA_INPUT_CHIP_IOC_GET_VENDOR");
		goto out_free_sensor;
	}

	ret = ioctl(ctrl_fd, HUA_INPUT_DEVICE_IOC_GET_TYPE, &type);
	if (ret < 0)
	{
		pr_red_info("ioctl HUA_INPUT_DEVICE_IOC_GET_TYPE");
		goto out_free_sensor;
	}

	hua_sensor_event_init(&sensor->event);

	switch (type)
	{
	case HUA_INPUT_DEVICE_TYPE_ACCELEROMETER:
		sensor->event.type = SENSOR_TYPE_ACCELEROMETER;
		break;

	case HUA_INPUT_DEVICE_TYPE_MAGNETIC_FIELD:
		sensor->event.type = SENSOR_TYPE_MAGNETIC_FIELD;
		break;

	case HUA_INPUT_DEVICE_TYPE_ORIENTATION:
		sensor->event.type = SENSOR_TYPE_ORIENTATION;
		break;

	case HUA_INPUT_DEVICE_TYPE_GRAVITY:
		sensor->event.type = SENSOR_TYPE_GRAVITY;
		break;

	case HUA_INPUT_DEVICE_TYPE_GYROSCOPE:
		sensor->event.type = SENSOR_TYPE_GYROSCOPE;
		break;

	case HUA_INPUT_DEVICE_TYPE_ROTATION_VECTOR:
		sensor->event.type = SENSOR_TYPE_ROTATION_VECTOR;
		break;

	case HUA_INPUT_DEVICE_TYPE_LIGHT:
		sensor->event.type = SENSOR_TYPE_LIGHT;
		break;

	case HUA_INPUT_DEVICE_TYPE_PRESSURE:
		sensor->event.type = SENSOR_TYPE_PRESSURE;
		break;

	case HUA_INPUT_DEVICE_TYPE_TEMPERATURE:
		sensor->event.type = SENSOR_TYPE_TEMPERATURE;
		break;

	case HUA_INPUT_DEVICE_TYPE_PROXIMITY:
		sensor->event.type = SENSOR_TYPE_PROXIMITY;
		break;

	default:
		pr_red_info("huamobile input device %s is not a sensor", sensor->name);
		ret = -EINVAL;
		goto out_free_sensor;
	}

	sensor->data_fd = fd;
	sensor->ctrl_fd = ctrl_fd;
	sensor->enabled = false;
	pthread_mutex_init(&sensor->lock, NULL);

	return sensor;

out_free_sensor:
	free(sensor);
out_close_ctrl_fd:
	close(ctrl_fd);
	return NULL;
}

static void hua_sensor_device_destory(struct hua_sensor_device *sensor)
{
	pthread_mutex_destroy(&sensor->lock);

	close(sensor->ctrl_fd);
	close(sensor->data_fd);

	free(sensor);
}

static int hua_sensor_device_probe(struct hua_sensor_device *sensor, struct sensor_t *hal_sensor, int handle)
{
	int ret;
	unsigned int min_delay, max_range, power_consume, resolution;
	int ctrl_fd = sensor->ctrl_fd;
	struct sensors_event_t *event = &sensor->event;

	pr_std_info("============================================================");

	ret = ioctl(ctrl_fd, HUA_INPUT_SENSOR_IOC_GET_MIN_DELAY, &min_delay);
	if (ret < 0)
	{
		pr_red_info("ioctl HUA_INPUT_SENSOR_IOC_GET_MIN_DELAY");
		return ret;
	}

	ret = ioctl(ctrl_fd, HUA_INPUT_SENSOR_IOC_GET_MAX_RANGE, &max_range);
	if (ret < 0)
	{
		pr_red_info("ioctl HUA_INPUT_SENSOR_IOC_GET_MAX_RANGE");
		return ret;
	}

	ret = ioctl(ctrl_fd, HUA_INPUT_SENSOR_IOC_GET_RESOLUTION, &resolution);
	if (ret < 0)
	{
		pr_red_info("ioctl HUA_INPUT_SENSOR_IOC_GET_RESOLUTION");
		return ret;
	}

	ret = ioctl(ctrl_fd, HUA_INPUT_SENSOR_IOC_GET_POWER_CONSUME, &power_consume);
	if (ret < 0)
	{
		pr_red_info("ioctl HUA_INPUT_SENSOR_IOC_GET_POWER_CONSUME");
		return ret;
	}

	hal_sensor->type = event->type;
	hal_sensor->version = 1;
	hal_sensor->vendor = sensor->vensor;
	hal_sensor->name = sensor->name;
	hal_sensor->handle = handle;
	hal_sensor->minDelay = min_delay;

	if (hal_sensor->type == SENSOR_TYPE_ACCELEROMETER)
	{
		hal_sensor->maxRange = max_range * GRAVITY_EARTH;
	}
	else
	{
		hal_sensor->maxRange = max_range;
	}

	hal_sensor->resolution = hal_sensor->maxRange / resolution;
	hal_sensor->power = ((float)power_consume) / 1000;

	event->sensor = handle;
	event->acceleration.status = SENSOR_STATUS_ACCURACY_HIGH;

	sensor->scale = hal_sensor->resolution;

	pr_green_info("Name = %s, Vendor = %s, Handle = %d", hal_sensor->name, hal_sensor->vendor, hal_sensor->handle);
	pr_green_info("maxRange = %f, Resolution = %f", hal_sensor->maxRange, hal_sensor->resolution);
	pr_green_info("Power = %f, minDelay = %d", hal_sensor->power, hal_sensor->minDelay);

	return 0;
}

static struct hua_sensor_device *hua_sensor_device_add(struct hua_sensor_device *head, struct hua_sensor_device *sensor)
{
	if (head)
	{
		head->prev = sensor;
	}

	sensor->prev = NULL;
	sensor->next = head;

	return sensor;
}

static struct hua_sensor_device *hua_sensor_device_remove(struct hua_sensor_device *head, struct hua_sensor_device *sensor)
{
	struct hua_sensor_device *prev = sensor->prev;
	struct hua_sensor_device *next = sensor->next;

	if (next)
	{
		next->prev = prev;
	}

	if (prev)
	{
		prev->next = next;
		return head;
	}

	return next;
}

static int hua_sensors_send_wakeup_event(struct hua_sensor_poll_device *pdev, char event)
{
	ssize_t wrlen;

	wrlen = write(pdev->pipefd[1], &event, 1);
	if (wrlen < 0)
	{
		pr_error_info("write");
		return wrlen;
	}

	pr_bold_info("Send wakeup event %d", event);

	return 0;
}

static int hua_sensors_recv_wakeup_event(struct hua_sensor_poll_device *pdev)
{
	char event;
	ssize_t rdlen;

	rdlen = read(pdev->pipefd[0], &event, 1);
	if (rdlen < 0)
	{
		pr_error_info("read");
		return rdlen;
	}

	pr_bold_info("Receive wakeup event %d", event);

	return event;
}

static int hua_sensor_device_set_enable(struct hua_sensor_poll_device *pdev, struct hua_sensor_device *sensor, bool enable)
{
	int ret;

	pthread_mutex_lock(&sensor->lock);

	ret = ioctl(sensor->ctrl_fd, HUA_INPUT_DEVICE_IOC_SET_ENABLE, enable);
	if (ret < 0)
	{
		pr_error_info("ioctl HUA_INPUT_DEVICE_IOC_SET_ENABLE");
		pthread_mutex_unlock(&sensor->lock);
		return ret;
	}

	if (sensor->enabled == enable)
	{
		pr_func_info("Nothing to be done");
		pthread_mutex_unlock(&sensor->lock);
		return 0;
	}

	pthread_mutex_lock(&pdev->lock);

	if (enable)
	{
		pdev->inactive_head = hua_sensor_device_remove(pdev->inactive_head, sensor);
		pdev->active_head = hua_sensor_device_add(pdev->active_head, sensor);
		sensor->pfd->events = POLLIN;
	}
	else
	{
		pdev->active_head = hua_sensor_device_remove(pdev->active_head, sensor);
		pdev->inactive_head = hua_sensor_device_add(pdev->inactive_head, sensor);
		sensor->pfd->events = 0;
	}

	sensor->enabled = enable;
	hua_sensors_send_wakeup_event(pdev, 0);

	pthread_mutex_unlock(&pdev->lock);

	pr_bold_info("set sensor device %s active %s", sensor->name, enable ? "enable" : "disable");

	pthread_mutex_unlock(&sensor->lock);

	return 0;
}

static void hua_sensors_destory(struct hua_sensor_poll_device *pdev)
{
	struct hua_sensor_device *sensor;

	pthread_mutex_lock(&pdev->lock);

	for (sensor = pdev->active_head; sensor; sensor = sensor->next)
	{
		hua_sensor_device_destory(sensor);
	}

	for (sensor = pdev->inactive_head; sensor; sensor = sensor->next)
	{
		hua_sensor_device_destory(sensor);
	}

	pdev->active_head = NULL;
	pdev->inactive_head = NULL;
	pdev->sensor_count = 0;

	pthread_mutex_unlock(&pdev->lock);

	pthread_mutex_destroy(&pdev->lock);
}

static void hua_sensors_remove(struct hua_sensor_poll_device *pdev)
{
	pthread_mutex_lock(&pdev->lock);

	free(pdev->sensor_list);
	pdev->sensor_list = NULL;
	pdev->sensor_map = NULL;
	pdev->pollfd_list = NULL;

	close(pdev->pipefd[0]);
	close(pdev->pipefd[1]);

	pthread_mutex_unlock(&pdev->lock);

	hua_sensors_destory(pdev);
}

static int hua_sensors_probe(struct hua_sensor_poll_device *pdev)
{
	int ret;
	int count;
	struct sensor_t *list;
	struct hua_sensor_device **map;
	struct pollfd *pfd;
	struct hua_sensor_device *sensor, *sensor_next;

	pthread_mutex_lock(&pdev->lock);

	ret = pipe(pdev->pipefd);
	if (ret < 0)
	{
		pr_error_info("pipe");
		goto out_mutex_unlock;
	}

	list = malloc((sizeof(*list) + sizeof(*map)) * pdev->sensor_count + sizeof(*pfd) * (pdev->sensor_count + 1));
	if (list == NULL)
	{
		pr_error_info("malloc");
		ret = -ENOMEM;
		goto out_close_pipe;
	}

	map = (struct hua_sensor_device **)(list + pdev->sensor_count);
	pfd = (struct pollfd *)(map + pdev->sensor_count);

	pdev->sensor_list = list;
	pdev->sensor_map = map;
	pdev->pollfd_list = pfd;

	pfd->events = POLLIN;
	pfd->fd = pdev->pipefd[0];

	for (count = 0, sensor = pdev->inactive_head; sensor; sensor = sensor_next)
	{
		sensor_next = sensor->next;

		ret = hua_sensor_device_probe(sensor, list + count, count);
		if (ret < 0)
		{
			pr_red_info("hua_sensor_chip_probe");
			pdev->inactive_head = hua_sensor_device_remove(pdev->inactive_head, sensor);
			hua_sensor_device_destory(sensor);
			continue;
		}

		map[count] = sensor;

		pfd++;
		pfd->events = 0;
		pfd->fd = sensor->data_fd;
		sensor->pfd = pfd;

		count++;
	}

	pdev->sensor_count = count;
	pdev->pollfd_count = count + 1;

	if (count)
	{
		ret = 0;
		goto out_mutex_unlock;
	}

	pr_red_info("No sensor chip found!");
	ret = -ENOENT;

	free(pdev->sensor_list);
out_close_pipe:
	close(pdev->pipefd[0]);
	close(pdev->pipefd[1]);
out_mutex_unlock:
	pthread_mutex_unlock(&pdev->lock);
	return ret;
}

static int hua_sensors_match_handler(int fd, const char *pathname, const char *devname, void *data)
{
	struct hua_sensor_device *sensor;
	struct hua_sensor_poll_device *pdev = data;

	sensor = hua_sensor_device_create(fd, devname);
	if (sensor == NULL)
	{
		pr_red_info("hua_sensor_device_create %s, pathname = %s", devname, pathname);
		return -EFAULT;
	}

	pr_bold_info("Add sensor device %s, name = %s", pathname, devname);

	pthread_mutex_lock(&pdev->lock);

	pdev->inactive_head = hua_sensor_device_add(pdev->inactive_head, sensor);
	pdev->sensor_count++;

	pthread_mutex_unlock(&pdev->lock);

	return 0;
}

static ssize_t hua_sensors_scan_devices(int (*match_handle)(int fd, const char *pathname, const char *devname, void *data), void *data)
{
	int fd;
	int ret;
	DIR *dp;
	size_t count;
	struct dirent *entry;
	char *filename;
	char pathname[1024];
	char devname[512];

	if (match_handle == NULL)
	{
		pr_red_info("match_handle == NULL");
		return -EINVAL;
	}

	filename = text_copy(pathname, "/dev/input/");
	dp = opendir(pathname);
	if (dp == NULL)
	{
		pr_error_info("open directory `%s'", pathname);
		return -ENOENT;
	}

	count = 0;

	while ((entry = readdir(dp)))
	{
		if (text_lhcmp("event", entry->d_name))
		{
			continue;
		}

		pr_std_info("============================================================");

		text_copy(filename, entry->d_name);
		fd = open(pathname, O_RDONLY);
		if (fd < 0)
		{
			pr_error_info("open file `%s'", pathname);
			continue;
		}

		ret = hua_input_get_devname(fd, devname, sizeof(devname));
		if (ret < 0)
		{
			pr_error_info("cavan_event_get_devname");
			close(fd);
			continue;
		}

		ret = match_handle(fd, pathname, devname, data);
		if (ret < 0)
		{
			pr_red_info("Handler device %s, name = %s", pathname, devname);
			close(fd);
			continue;
		}

		count++;
	}

	closedir(dp);

	return count;
}

static int hua_sensors_open(struct hua_sensor_poll_device *pdev)
{
	int ret;

	pthread_mutex_init(&pdev->lock, NULL);

	pdev->sensor_count = 0;
	pdev->sensor_list = NULL;
	pdev->sensor_map = NULL;

	pdev->active_head = pdev->inactive_head = NULL;
	pdev->pipefd[0] = pdev->pipefd[1] = -1;

	ret = hua_sensors_scan_devices(hua_sensors_match_handler, pdev);
	if (ret < 0)
	{
		pr_red_info("cavan_event_scan_devices");
		return ret;
	}

	if (pdev->sensor_count == 0)
	{
		pr_red_info("No device matched");
		return -ENOENT;
	}

	ret = hua_sensors_probe(pdev);
	if (ret < 0)
	{
		pr_red_info("hua_sensors_probe");
		hua_sensors_destory(pdev);
		return ret;
	}

	return 0;
}

static int hua_sensors_activate(struct sensors_poll_device_t *dev, int handle, int enabled)
{
	struct hua_sensor_poll_device *pdev = (struct hua_sensor_poll_device *)dev;
	struct hua_sensor_device *sensor = pdev->sensor_map[handle];

	pr_bold_info("handle = %d, sensor = %s", handle, sensor->name);

	return hua_sensor_device_set_enable(pdev, sensor, enabled);
}

static int hua_sensors_setDelay(struct sensors_poll_device_t *dev, int handle, int64_t ns)
{
	int ret;
	unsigned int delay = ns / 1000000;
	struct hua_sensor_poll_device *pdev = (struct hua_sensor_poll_device *)dev;
	struct hua_sensor_device *sensor = pdev->sensor_map[handle];

	pr_func_info("Delay = %d(ms)", delay);

	pthread_mutex_lock(&sensor->lock);

	ret = ioctl(sensor->ctrl_fd, HUA_INPUT_DEVICE_IOC_SET_DELAY, delay);
	if (ret < 0)
	{
		pr_error_info("ioctl HUA_SENSOR_IOCS_DELAY");
	}

	pthread_mutex_unlock(&sensor->lock);

	return ret;
}

static int hua_sensors_poll(struct sensors_poll_device_t *dev, sensors_event_t *data, int count)
{
	int ret;
	ssize_t rdlen;
	struct hua_sensor_device *sensor;
	int64_t timestamp;
	struct input_event evbuff[count], *ep, *ep_end;
	struct hua_sensor_poll_device *pdev = (struct hua_sensor_poll_device *)dev;
	struct sensors_event_t *data_bak = data, *data_end = data + count;

	pthread_mutex_lock(&pdev->lock);

	while (data_bak == data)
	{
		pthread_mutex_unlock(&pdev->lock);

		ret = poll(pdev->pollfd_list, pdev->pollfd_count, -1);
		if (ret < 0)
		{
			pr_error_info("poll");
			return ret;
		}

		pthread_mutex_lock(&pdev->lock);

		if (pdev->pollfd_list->revents)
		{
			hua_sensors_recv_wakeup_event(pdev);
			pthread_mutex_unlock(&pdev->lock);
			return 0;
		}

		timestamp = systemTime(SYSTEM_TIME_MONOTONIC);

		for (sensor = pdev->active_head; sensor; sensor = sensor->next)
		{
			if (sensor->pfd->revents == 0)
			{
				continue;
			}

			rdlen = read(sensor->data_fd, evbuff, sizeof(evbuff));
			if (rdlen < 0)
			{
				pr_error_info("read data from chip %s", sensor->name);
				sensor->event.acceleration.status = SENSOR_STATUS_UNRELIABLE;
				continue;
			}

			for (ep = evbuff, ep_end = ep + rdlen / sizeof(evbuff[0]); ep < ep_end && data < data_end; ep++)
			{
				switch (ep->type)
				{
				case EV_SYN:
					*data = sensor->event;
					data->timestamp = timestamp;
					// pr_std_info("%s [%f, %f, %f]", sensor->name, data->data[0], data->data[1], data->data[2]);
					data++;
					break;

				case EV_ABS:
					switch (ep->code)
					{
					case ABS_MISC:
					case ABS_X:
						sensor->event.data[0] = ep->value * sensor->scale;
						break;

					case ABS_Y:
						sensor->event.data[1] = ep->value * sensor->scale;
						break;

					case ABS_Z:
						sensor->event.data[2] = ep->value * sensor->scale;
						break;
					}
				}
			}
		}
	}

	pthread_mutex_unlock(&pdev->lock);

	return data - data_bak;
}

// ================================================================================

static int hua_sensors_module_close(struct hw_device_t *device)
{
	pr_func_info("device = %p", device);

	hua_sensors_remove((struct hua_sensor_poll_device *)device);

	return 0;
}

static struct hua_sensor_poll_device hua_poll_device =
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
