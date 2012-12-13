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

static struct hua_sensor_chip *hua_sensor_chip_create(int fd, const char *devname)
{
	int ret;
	int index;
	int ctrl_fd;
	char ctrl_path[1024];
	struct hua_sensor_chip *chip;
	struct hua_sensor_device *sensor, *sensor_end;

	if (text_lhcmp("sensor-", devname))
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

	chip = malloc(sizeof(*chip));
	if (chip == NULL)
	{
		pr_error_info("malloc");
		goto out_close_ctrl_fd;
	}

	chip->data_fd = fd;
	chip->ctrl_fd = ctrl_fd;

	ret = ioctl(ctrl_fd, HUA_SENSOR_IOC_GET_CHIP_NAME(sizeof(chip->name)), chip->name);
	if (ret < 0)
	{
		pr_error_info("ioctl HUA_SENSOR_IOC_GET_CHIP_NAME");
		goto out_free_chip;
	}

	ret = ioctl(ctrl_fd, HUA_SENSOR_IOC_GET_SENSOR_COUNT, &chip->sensor_count);
	if (ret < 0)
	{
		pr_error_info("ioctl HUA_SENSOR_IOC_GET_SENSOR_COUNT");
		goto out_free_chip;
	}

	pr_bold_info("chip %s sensor count = %d", chip->name, chip->sensor_count);

	if (chip->sensor_count == 0)
	{
		pr_red_info("chip->sensor_count == 0");
		goto out_free_chip;
	}

	ret = ioctl(ctrl_fd, HUA_SENSOR_IOC_GET_CHIP_VENDOR(sizeof(chip->vensor)), chip->vensor);
	if (ret < 0)
	{
		pr_error_info("ioctl HUA_SENSOR_IOC_GET_CHIP_VENDOR");
		goto out_free_chip;
	}

	ret = ioctl(ctrl_fd, HUA_SENSOR_IOC_GET_MIN_DELAY, &chip->min_delay);
	if (ret < 0)
	{
		pr_error_info("ioctl HUA_SENSOR_IOC_GET_MIN_DELAY");
		goto out_free_chip;
	}

	sensor = malloc(sizeof(*sensor) * chip->sensor_count);
	if (sensor == NULL)
	{
		pr_error_info("malloc");
		goto out_free_chip;
	}

	chip->sensor_list = sensor;

	for (index = 0, sensor_end = sensor + chip->sensor_count; sensor < sensor_end; index++, sensor++)
	{
		ret = hua_sensor_device_init(sensor, chip, index);
		if (ret < 0)
		{
			pr_red_info("hua_sensor_device_init");
			goto out_free_sensors;
		}
	}

	return chip;

out_free_sensors:
	free(chip->sensor_list);
out_free_chip:
	free(chip);
out_close_ctrl_fd:
	close(ctrl_fd);
	return NULL;
}

static void hua_sensor_chip_destory(struct hua_sensor_chip *chip)
{
	close(chip->ctrl_fd);
	close(chip->data_fd);
	pthread_mutex_destroy(&chip->lock);
	free(chip->sensor_list);
	free(chip);
}

static int hua_sensor_chip_probe(struct hua_sensor_chip *chip, struct sensor_t asensor[], struct hua_sensor_device *sensor_map[], int handle)
{
	int ret;
	struct hua_sensor_device *sensor, *sensor_end;

	for (sensor = chip->sensor_list, sensor_end = sensor + chip->sensor_count; sensor < sensor_end; sensor++, handle++)
	{
		ret = hua_sensor_device_probe(sensor, asensor + handle);
		if (ret < 0)
		{
			pr_red_info("hua_sensor_device_probe");
			return ret;
		}

		asensor[handle].version = 1;
		asensor[handle].handle = handle;
		sensor_map[handle] = sensor;

		pr_std_info("============================================================");

		pr_green_info("Name = %s, Vendor = %s, Handle = %d", asensor->name, asensor->vendor, handle);
		pr_green_info("maxRange = %f, Resolution = %f", asensor->maxRange, asensor->resolution);
		pr_green_info("Power = %f, minDelay = %d", asensor->power, asensor->minDelay);
	}

	return 0;
}

static struct hua_sensor_chip *hua_sensor_add_chip(struct hua_sensor_chip *head, struct hua_sensor_chip *chip)
{
	if (head)
	{
		head->prev = chip;
	}

	chip->prev = NULL;
	chip->next = head;

	return chip;
}

static struct hua_sensor_chip *hua_sensor_remove_chip(struct hua_sensor_chip *head, struct hua_sensor_chip *chip)
{
	struct hua_sensor_chip *prev = chip->prev;
	struct hua_sensor_chip *next = chip->next;

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

static void hua_sensor_set_enable_base(struct hua_sensor_poll_device *pdev, struct hua_sensor_chip *chip, bool enable)
{
	pr_bold_info("set sensor chip %s active %s", chip->name, enable ? "enable" : "disable");

	if (enable)
	{
		pdev->inactive_head = hua_sensor_remove_chip(pdev->inactive_head, chip);
		pdev->active_head = hua_sensor_add_chip(pdev->active_head, chip);
		chip->pfd->events = POLLIN;
	}
	else
	{
		pdev->active_head = hua_sensor_remove_chip(pdev->active_head, chip);
		pdev->inactive_head = hua_sensor_add_chip(pdev->inactive_head, chip);
		chip->pfd->events = 0;
	}

	hua_sensors_send_wakeup_event(pdev, 0);
}

static int hua_sensor_chip_set_enable_lock(struct hua_sensor_poll_device *pdev, struct hua_sensor_chip *chip, bool enable)
{
	pthread_mutex_lock(&pdev->lock);
	pthread_mutex_lock(&chip->lock);

	if (enable)
	{
		if (chip->use_count == 0)
		{
			hua_sensor_set_enable_base(pdev, chip, true);
		}

		chip->use_count++;
	}
	else
	{
		if (chip->use_count == 1)
		{
			hua_sensor_set_enable_base(pdev, chip, false);
		}

		chip->use_count--;
	}

	pthread_mutex_unlock(&chip->lock);
	pthread_mutex_unlock(&pdev->lock);

	pr_bold_info("chip %s use count = %d", chip->name, chip->use_count);

	return 0;
}

static int hua_sensor_device_set_enable(struct hua_sensor_poll_device *pdev, struct hua_sensor_device *sensor, bool enable)
{
	int ret;

	pthread_mutex_lock(&sensor->lock);

	pr_bold_info("%s device %s", enable ? "Enable" : "Disable", sensor->name);

	ret = ioctl(sensor->chip->ctrl_fd, HUA_SENSOR_IOC_SET_ENABLE(sensor->index), enable);
	if (ret < 0)
	{
		pr_error_info("ioctl HUA_SENSOR_IOCS_ENABLE");
		pthread_mutex_unlock(&sensor->lock);
		return ret;
	}

	if (sensor->enabled == enable)
	{
		pr_func_info("Nothing to be done");
		pthread_mutex_unlock(&sensor->lock);
		return 0;
	}

	hua_sensor_chip_set_enable_lock(pdev, sensor->chip, enable);
	sensor->enabled = enable;

	pthread_mutex_unlock(&sensor->lock);

	return ret;
}

static void hua_sensors_destory(struct hua_sensor_poll_device *pdev)
{
	struct hua_sensor_chip *chip;

	pthread_mutex_lock(&pdev->lock);

	for (chip = pdev->active_head; chip; chip = chip->next)
	{
		hua_sensor_chip_destory(chip);
	}

	for (chip = pdev->inactive_head; chip; chip = chip->next)
	{
		hua_sensor_chip_destory(chip);
	}

	pdev->active_head = NULL;
	pdev->inactive_head = NULL;
	pdev->sensor_count = 0;
	pdev->pollfd_count = 0;

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
	int handle;
	struct sensor_t *list;
	struct hua_sensor_device **map;
	struct pollfd *pfd;
	struct hua_sensor_chip *chip, *chip_next;

	pthread_mutex_lock(&pdev->lock);

	ret = pipe(pdev->pipefd);
	if (ret < 0)
	{
		pr_error_info("pipe");
		goto out_mutex_unlock;
	}

	pdev->pollfd_count++;

	list = malloc((sizeof(*list) + sizeof(*map)) * pdev->sensor_count + sizeof(*pfd) * pdev->pollfd_count);
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

	for (handle = 0, chip = pdev->inactive_head; chip; chip = chip->next)
	{
		chip_next = chip->next;

		ret = hua_sensor_chip_probe(chip, list, map, handle);
		if (ret < 0)
		{
			pr_red_info("hua_sensor_chip_probe");
			pdev->inactive_head = hua_sensor_remove_chip(pdev->inactive_head, chip);
			hua_sensor_chip_destory(chip);
			continue;
		}

		handle += chip->sensor_count;

		pfd++;
		pfd->events = 0;
		pfd->fd = chip->data_fd;
		chip->pfd = pfd;
	}

	pdev->sensor_count = handle;
	if (pdev->sensor_count)
	{
		pthread_mutex_unlock(&pdev->lock);
		return 0;
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
	struct hua_sensor_chip *chip;
	struct hua_sensor_poll_device *pdev = data;

	chip = hua_sensor_chip_create(fd, devname);
	if (chip == NULL)
	{
		pr_red_info("hua_sensor_chip_create %s, pathname = %s", devname, pathname);
		return -EFAULT;
	}

	pr_bold_info("Add sensor device %s, name = %s", pathname, devname);

	pthread_mutex_lock(&pdev->lock);

	pdev->inactive_head = hua_sensor_add_chip(pdev->inactive_head, chip);
	pdev->sensor_count += chip->sensor_count;
	pdev->pollfd_count++;

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

static int hua_sensors_open(struct hua_sensor_poll_device *pdev)
{
	int ret;
	int core_fd;
	const char *core_ctrl_path;

	pthread_mutex_init(&pdev->lock, NULL);

	pdev->pollfd_count = 0;
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

	core_ctrl_path = HUA_SENSOR_CORE_DEVPATH;
	core_fd = open(core_ctrl_path, 0);
	if (core_fd < 0)
	{
		pr_error_info("open core ctrl device %s", core_ctrl_path);
		hua_sensors_remove(pdev);
		return core_fd;
	}

	ret = ioctl(core_fd, HUA_SENSOR_IOC_SET_DETECT, false);
	if (ret < 0)
	{
		pr_error_info("ioctl HUA_SENSOR_IOC_SET_DETECT");
		hua_sensors_remove(pdev);
	}

	close(core_fd);

	return ret;
}

static int hua_sensors_activate(struct sensors_poll_device_t *dev, int handle, int enabled)
{
	struct hua_sensor_poll_device *pdev = (struct hua_sensor_poll_device *)dev;
	struct hua_sensor_device *sensor = pdev->sensor_map[handle];

	pr_bold_info("handle = %d, index = %d", handle, sensor->index);

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

	ret = ioctl(sensor->chip->ctrl_fd, HUA_SENSOR_IOC_SET_DELAY(sensor->index), delay);
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
	struct hua_sensor_chip *chip;
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

		for (chip = pdev->active_head; chip; chip = chip->next)
		{
			if (chip->pfd->revents == 0)
			{
				continue;
			}

			rdlen = read(chip->data_fd, evbuff, sizeof(evbuff));
			if (rdlen < 0)
			{
				pr_error_info("read data from chip %s", chip->name);
				pthread_mutex_unlock(&pdev->lock);
				return rdlen;
			}

			for (ep = evbuff, ep_end = ep + rdlen / sizeof(evbuff[0]); ep < ep_end; ep++)
			{
				if (ep->type == EV_SYN)
				{
					data = hua_sensor_chip_sync_event(chip, data, data_end - data);
					if (data >= data_end)
					{
						pthread_mutex_unlock(&pdev->lock);
						return count;
					}
				}
				else
				{
					hua_sensor_chip_report_event(chip, ep);
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
