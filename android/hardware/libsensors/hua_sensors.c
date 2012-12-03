/*
 * File:			hua_sensors.c
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

#include <hua_sensors.h>

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

static int hua_sensor_get_attributes(struct hua_sensor_device *dev, struct sensor_t *sensor)
{
	int ret;
	unsigned int value;
	int fd = dev->ctrl_fd;

	ret = ioctl(fd, HUA_SENSOR_IOCG_MAX_RANGE, &value);
	if (ret < 0)
	{
		pr_error_info("ioctl HUA_SENSOR_IOCG_MAX_RANGE");
		return ret;
	}

	sensor->maxRange = value;

	ret = ioctl(fd, HUA_SENSOR_IOCG_RESOLUTION, &value);
	if (ret < 0)
	{
		pr_error_info("ioctl HUA_SENSOR_IOCG_RESOLUTION");
		return ret;
	}

	sensor->resolution = value;

	ret = ioctl(fd, HUA_SENSOR_IOCG_POWER_CONSUME, &value);
	if (ret < 0)
	{
		pr_error_info("ioctl HUA_SENSOR_IOCG_POWER_CONSUME");
		return ret;
	}

	sensor->power = ((float)value) / 1000;

	ret = ioctl(fd, HUA_SENSOR_IOCG_MIN_DELAY, &value);
	if (ret < 0)
	{
		pr_error_info("ioctl HUA_SENSOR_IOCG_MIN_DELAY");
		return ret;
	}

	sensor->minDelay = value;

	return 0;
}

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

	sprintf(ctrl_path, "/dev/%s", name);
	pr_bold_info("ctrl_path = %s", ctrl_path);

	sensor->ctrl_fd = open(ctrl_path, 0);
	if (sensor->ctrl_fd < 0)
	{
		pr_error_info("open `%s'", ctrl_path);
		free(sensor);
		return NULL;
	}

	sensor->data_fd = fd;
	text_ncopy(sensor->name, name, sizeof(sensor->name));

	pthread_mutex_init(&sensor->lock, NULL);

	return sensor;
}

static void hua_sensor_destory(struct hua_sensor_device *sensor)
{
	close(sensor->ctrl_fd);
	close(sensor->data_fd);
	pthread_mutex_destroy(&sensor->lock);
	free(sensor);
}

static struct hua_sensor_device *hua_sensors_list_add(struct hua_sensor_device *head, struct hua_sensor_device *sensor)
{
	if (head)
	{
		head->prev = sensor;
	}

	sensor->prev = NULL;
	sensor->next = head;

	return sensor;
}

static struct hua_sensor_device *hua_sensors_list_remove(struct hua_sensor_device *head, struct hua_sensor_device *sensor)
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

static int hua_sensors_send_wakeup_event(struct hua_sensors_poll_device *pdev, char event)
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

static int hua_sensors_recv_wakeup_event(struct hua_sensors_poll_device *pdev)
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

static int hua_sensors_rebuild_pollfd_list(struct hua_sensors_poll_device *pdev)
{
	struct hua_sensor_device *sensor;
	struct pollfd *list, *list_end;

	pthread_mutex_lock(&pdev->lock);

	if (pdev->pfd_list == NULL)
	{
		int ret;

		list = malloc(sizeof(*list) * (pdev->sensor_count + 1));
		if (list == NULL)
		{
			pr_error_info("malloc");
			pdev->poll_count = 0;
			pthread_mutex_unlock(&pdev->lock);
			return -ENOMEM;
		}

		ret = pipe(pdev->pipefd);
		if (ret < 0)
		{
			pr_error_info("pipe");
			free(list);
			pdev->poll_count = 0;
			pthread_mutex_unlock(&pdev->lock);
			return ret;
		}

		pdev->pfd_list = list;

		list->events = POLLIN;
		list->fd = pdev->pipefd[0];
		list->revents = 0;
		list++;
	}
	else
	{
		list = pdev->pfd_list + 1;
	}

	for (list_end = list + pdev->sensor_count, sensor = pdev->active_head; list < list_end && sensor; sensor = sensor->next, list++)
	{
		pr_bold_info("Add sensor %s to poll list", sensor->name);

		list->events = POLLIN;
		list->fd = sensor->data_fd;
		list->revents = 0;

		sensor->pfd = list;
	}

	pdev->poll_count = list - pdev->pfd_list;
	pr_bold_info("Active device count = %d", pdev->poll_count - 1);

	hua_sensors_send_wakeup_event(pdev, 0);

	pthread_mutex_unlock(&pdev->lock);

	return 0;
}

static int hua_sensor_active_enable(struct hua_sensors_poll_device *pdev, struct hua_sensor_device *sensor, unsigned int enable)
{
	int ret;

	pthread_mutex_lock(&sensor->lock);

	pr_bold_info("%s device %s", enable ? "Enable" : "Disable", sensor->name);

	if (sensor->active == enable)
	{
		pr_func_info("Nothing to be done");
		pthread_mutex_unlock(&sensor->lock);
		return 0;
	}

	ret = ioctl(sensor->ctrl_fd, HUA_SENSOR_IOCS_ENABLE, &enable);
	if (ret < 0)
	{
		pr_error_info("ioctl HUA_SENSOR_IOCS_ENABLE");
		pthread_mutex_unlock(&sensor->lock);
		return ret;
	}

	pthread_mutex_lock(&pdev->lock);

	if (enable)
	{
		pdev->inactive_head = hua_sensors_list_remove(pdev->inactive_head, sensor);
		pdev->active_head = hua_sensors_list_add(pdev->active_head, sensor);
	}
	else
	{
		pdev->active_head = hua_sensors_list_remove(pdev->active_head, sensor);
		pdev->inactive_head = hua_sensors_list_add(pdev->inactive_head, sensor);
	}

	sensor->active = enable;

	pthread_mutex_unlock(&pdev->lock);
	pthread_mutex_unlock(&sensor->lock);

	ret = hua_sensors_rebuild_pollfd_list(pdev);

	return ret;
}

static void hua_sensors_destory(struct hua_sensors_poll_device *pdev)
{
	struct hua_sensor_device *sensor;

	pthread_mutex_lock(&pdev->lock);

	for (sensor = pdev->active_head; sensor; sensor = sensor->next)
	{
		hua_sensor_destory(sensor);
	}

	for (sensor = pdev->inactive_head; sensor; sensor = sensor->next)
	{
		hua_sensor_destory(sensor);
	}

	pdev->active_head = NULL;
	pdev->inactive_head = NULL;
	pdev->sensor_count = 0;

	pthread_mutex_unlock(&pdev->lock);

	pthread_mutex_destroy(&pdev->lock);
}

static void hua_sensors_remove(struct hua_sensors_poll_device *pdev)
{
	pthread_mutex_lock(&pdev->lock);

	if (pdev->pfd_list)
	{
		free(pdev->pfd_list);
		close(pdev->pipefd[0]);
		close(pdev->pipefd[1]);
		pdev->poll_count = 0;
		pdev->pfd_list = NULL;
	}

	if (pdev->sensor_list)
	{
		free(pdev->sensor_list);
		pdev->sensor_list = NULL;
		pdev->sensor_map = NULL;
	}

	pthread_mutex_unlock(&pdev->lock);

	hua_sensors_destory(pdev);
}

static int hua_sensors_probe(struct hua_sensors_poll_device *pdev)
{
	struct sensor_t *list, *list_end;
	struct hua_sensor_device **map;
	struct hua_sensor_device *sensor;

	pthread_mutex_lock(&pdev->lock);

	if (pdev->sensor_list == NULL)
	{
		list = malloc((sizeof(*list) + sizeof(*map)) * pdev->sensor_count);
		if (list == NULL)
		{
			pr_error_info("malloc");
			pdev->sensor_count = 0;
			pthread_mutex_unlock(&pdev->lock);
			return 0;
		}

		pdev->sensor_list = list;
		pdev->sensor_map = (struct hua_sensor_device **)(list + pdev->sensor_count);
	}
	else
	{
		list = pdev->sensor_list;
	}

	map = pdev->sensor_map;

	for (list_end = list + pdev->sensor_count, sensor = pdev->inactive_head; list < list_end && sensor; sensor = sensor->next)
	{
		if (hua_sensor_get_attributes(sensor, list) < 0 || (sensor->probe && sensor->probe(sensor, list) < 0))
		{
			pdev->inactive_head = hua_sensors_list_remove(pdev->inactive_head, sensor);
			hua_sensor_destory(sensor);
		}
		else
		{
			pr_std_info("============================================================");

			pr_green_info("Device name = %s", sensor->name);
			pr_green_info("Name = %s, Vendor = %s", list->name, list->vendor);
			pr_green_info("maxRange = %f, Resolution = %f", list->maxRange, list->resolution);
			pr_green_info("Power = %f, minDelay = %d", list->power, list->minDelay);

			list->version = 1;
			list->handle = list - pdev->sensor_list;
			map[list->handle] = sensor;

			list++;
		}
	}

	pdev->sensor_count = list - pdev->sensor_list;

	pthread_mutex_unlock(&pdev->lock);

	return 0;
}

static int hua_sensors_match_handler(int fd, const char *pathname, const char *devname, void *data)
{
	struct hua_sensor_device *sensor;
	struct hua_sensors_poll_device *pdev = data;

	sensor = hua_sensor_create(fd, devname);
	if (sensor == NULL)
	{
		return -EFAULT;
	}

	pr_bold_info("Add sensor device %s, name = %s", pathname, devname);

	pthread_mutex_lock(&pdev->lock);

	pdev->inactive_head = hua_sensors_list_add(pdev->inactive_head, sensor);
	sensor->active = false;
	pdev->sensor_count++;

	pthread_mutex_unlock(&pdev->lock);

	return 0;
}

ssize_t hua_sensors_scan_devices(int (*match_handle)(int fd, const char *pathname, const char *devname, void *data), void *data)
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

		ret = cavan_event_get_devname(fd, devname, sizeof(devname));
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

static int hua_sensors_open(struct hua_sensors_poll_device *pdev)
{
	int ret;

	pthread_mutex_init(&pdev->lock, NULL);

	pdev->sensor_count = 0;
	pdev->sensor_list = NULL;
	pdev->sensor_map = NULL;

	pdev->poll_count = 0;
	pdev->pfd_list = NULL;
	pdev->active_head = NULL;
	pdev->inactive_head = NULL;

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

	if (pdev->sensor_count == 0)
	{
		pr_red_info("pdev->sensor_count == 0");
		hua_sensors_remove(pdev);
		return -ENOENT;
	}

	ret = hua_sensors_rebuild_pollfd_list(pdev);
	if (ret < 0)
	{
		pr_red_info("hua_sensors_rebuild_pollfd_list");
		hua_sensors_remove(pdev);
		return ret;
	}

	return 0;
}

static int hua_sensors_activate(struct sensors_poll_device_t *dev, int handle, int enabled)
{
	struct hua_sensors_poll_device *pdev = (struct hua_sensors_poll_device *)dev;
	struct hua_sensor_device *sensor = pdev->sensor_map[handle];

	return hua_sensor_active_enable(pdev, sensor, enabled);
}

static int hua_sensors_setDelay(struct sensors_poll_device_t *dev, int handle, int64_t ns)
{
	int ret;
	unsigned int delay = ns / 1000000;
	struct hua_sensors_poll_device *pdev = (struct hua_sensors_poll_device *)dev;
	struct hua_sensor_device *sensor = pdev->sensor_map[handle];

	pr_func_info("Delay = %d(ms)", delay);

	pthread_mutex_lock(&sensor->lock);

	ret = ioctl(sensor->ctrl_fd, HUA_SENSOR_IOCS_DELAY, &delay);
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
	struct input_event evbuff[count], *ep, *ep_end;
	struct hua_sensors_poll_device *pdev = (struct hua_sensors_poll_device *)dev;
	struct sensors_event_t *data_bak = data, *data_last = data + count - 1;

	pthread_mutex_lock(&pdev->lock);

	while (data_bak == data)
	{
		pthread_mutex_unlock(&pdev->lock);

		ret = poll(pdev->pfd_list, pdev->poll_count, -1);
		if (ret < 0)
		{
			pr_error_info("poll");
			return ret;
		}

		pthread_mutex_lock(&pdev->lock);

		if (pdev->pfd_list->revents)
		{
			hua_sensors_recv_wakeup_event(pdev);
			pthread_mutex_unlock(&pdev->lock);
			return 0;
		}

		for (sensor = pdev->active_head; sensor; sensor = sensor->next)
		{
			if (sensor->pfd->revents == 0)
			{
				continue;
			}

			rdlen = read(sensor->data_fd, evbuff, sizeof(evbuff));
			if (rdlen < 0)
			{
				pr_error_info("read data from %s", sensor->name);
				pthread_mutex_unlock(&pdev->lock);
				return rdlen;
			}

			for (ep = evbuff, ep_end = ep + rdlen / sizeof(evbuff[0]); ep < ep_end; ep++)
			{
				if (sensor->event_handler(sensor, ep))
				{
					memcpy(data, &sensor->event, sizeof(*data));
					data->timestamp = timeval2nano(&ep->time);

					if (++data > data_last)
					{
						pthread_mutex_unlock(&pdev->lock);
						return count;
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

	hua_sensors_remove((struct hua_sensors_poll_device *)device);

	return 0;
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
