/*
 * File:				sensors.c
 * Author:			Fuang Cao <cavan.cfa@gmail.com>
 *
 * Created:			2012-12-03
 * Description:		Cavan Sensor HAL
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

#include <cavan.h>
#include <cavan/time.h>
#include <cavan/text.h>
#include <cavan/event.h>
#include <android/sensors.h>
#include <cavan/calculator.h>

static struct cavan_sensor_device *cavan_sensor_device_create(int fd, const char *devname)
{
	int ret;
	int ctrl_fd;
	unsigned int type;
	char ctrl_path[1024];
	struct cavan_sensor_device *sensor;

	snprintf(ctrl_path, sizeof(ctrl_path), "/dev/%s", devname);
	pd_bold_info("ctrl_path = %s", ctrl_path);

	ctrl_fd = open(ctrl_path, 0);
	if (ctrl_fd < 0) {
		pd_error_info("open `%s'", ctrl_path);
		return NULL;
	}

	sensor = malloc(sizeof(*sensor));
	if (sensor == NULL) {
		pd_error_info("malloc");
		goto out_close_ctrl_fd;
	}

	ret = ioctl(ctrl_fd, CAVAN_INPUT_DEVICE_IOC_GET_NAME(sizeof(sensor->name)), sensor->name);
	if (ret < 0) {
		pd_error_info("ioctl CAVAN_INPUT_DEVICE_IOC_GET_NAME");
		goto out_free_sensor;
	}

	ret = ioctl(ctrl_fd, CAVAN_INPUT_CHIP_IOC_GET_VENDOR(sizeof(sensor->vensor)), sensor->vensor);
	if (ret < 0) {
		pd_error_info("ioctl CAVAN_INPUT_CHIP_IOC_GET_VENDOR");
		goto out_free_sensor;
	}

	ret = ioctl(ctrl_fd, CAVAN_INPUT_DEVICE_IOC_GET_TYPE, &type);
	if (ret < 0) {
		pd_red_info("ioctl CAVAN_INPUT_DEVICE_IOC_GET_TYPE");
		goto out_free_sensor;
	}

	cavan_sensor_event_init(&sensor->event);

	switch (type) {
	case CAVAN_INPUT_DEVICE_TYPE_ACCELEROMETER:
		sensor->event.type = SENSOR_TYPE_ACCELEROMETER;
		break;

	case CAVAN_INPUT_DEVICE_TYPE_MAGNETIC_FIELD:
		sensor->event.type = SENSOR_TYPE_MAGNETIC_FIELD;
		break;

	case CAVAN_INPUT_DEVICE_TYPE_ORIENTATION:
		sensor->event.type = SENSOR_TYPE_ORIENTATION;
		break;

	case CAVAN_INPUT_DEVICE_TYPE_GRAVITY:
		sensor->event.type = SENSOR_TYPE_GRAVITY;
		break;

	case CAVAN_INPUT_DEVICE_TYPE_GYROSCOPE:
		sensor->event.type = SENSOR_TYPE_GYROSCOPE;
		break;

	case CAVAN_INPUT_DEVICE_TYPE_ROTATION_VECTOR:
		sensor->event.type = SENSOR_TYPE_ROTATION_VECTOR;
		break;

	case CAVAN_INPUT_DEVICE_TYPE_LIGHT:
		sensor->event.type = SENSOR_TYPE_LIGHT;
		break;

	case CAVAN_INPUT_DEVICE_TYPE_PRESSURE:
		sensor->event.type = SENSOR_TYPE_PRESSURE;
		break;

	case CAVAN_INPUT_DEVICE_TYPE_TEMPERATURE:
		sensor->event.type = SENSOR_TYPE_TEMPERATURE;
		break;

	case CAVAN_INPUT_DEVICE_TYPE_PROXIMITY:
		sensor->event.type = SENSOR_TYPE_PROXIMITY;
		break;

	default:
		pd_red_info("cavan input device %s is not a sensor", sensor->name);
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

static void cavan_sensor_device_destory(struct cavan_sensor_device *sensor)
{
	pthread_mutex_destroy(&sensor->lock);

	close(sensor->ctrl_fd);
	close(sensor->data_fd);

	free(sensor);
}

static int cavan_sensor_event_load_absinfo(struct cavan_sensor_device *sensor, ...)
{
	int count;
	va_list ap;

	va_start(ap, sensor);

	for (count = 0; ; count++) {
		int ret;
		struct input_absinfo absinfo;
		int code;

		code = va_arg(ap, int);
		if (code < 0) {
			break;
		}

		ret = ioctl(sensor->data_fd, EVIOCGABS(code), &absinfo);
		if (ret < 0) {
			pd_error_info("ioctl EVIOCGABS(%d)", code);
			return ret;
		}

		pd_green_info("code = %d, value = %d, maximum = %d, minimum = %d, flat = %d, fuzz = %d",
			code, absinfo.value, absinfo.maximum, absinfo.minimum, absinfo.flat, absinfo.fuzz);

		sensor->event.data[count] = sensor->scale * absinfo.value;
	}

	va_end(ap);

	return 0;
}

static int cavan_sensor_device_probe(struct cavan_sensor_device *sensor, struct sensor_t *hal_sensor, int handle)
{
	int ret;
	int ctrl_fd = sensor->ctrl_fd;
	struct sensors_event_t *event = &sensor->event;
	unsigned int min_delay, max_range, power_consume, resolution, axis_count;

	pd_std_info("============================================================");

	ret = ioctl(ctrl_fd, CAVAN_INPUT_SENSOR_IOC_GET_MIN_DELAY, &min_delay);
	if (ret < 0) {
		pd_red_info("ioctl CAVAN_INPUT_SENSOR_IOC_GET_MIN_DELAY");
		return ret;
	}

	ret = ioctl(ctrl_fd, CAVAN_INPUT_SENSOR_IOC_GET_MAX_RANGE, &max_range);
	if (ret < 0) {
		pd_red_info("ioctl CAVAN_INPUT_SENSOR_IOC_GET_MAX_RANGE");
		return ret;
	}

	ret = ioctl(ctrl_fd, CAVAN_INPUT_SENSOR_IOC_GET_RESOLUTION, &resolution);
	if (ret < 0) {
		pd_red_info("ioctl CAVAN_INPUT_SENSOR_IOC_GET_RESOLUTION");
		return ret;
	}

	ret = ioctl(ctrl_fd, CAVAN_INPUT_SENSOR_IOC_GET_POWER_CONSUME, &power_consume);
	if (ret < 0) {
		pd_red_info("ioctl CAVAN_INPUT_SENSOR_IOC_GET_POWER_CONSUME");
		return ret;
	}

	hal_sensor->type = event->type;
	hal_sensor->version = 1;
	hal_sensor->vendor = sensor->vensor;
	hal_sensor->name = sensor->name;
	hal_sensor->handle = handle;
	hal_sensor->minDelay = min_delay;

	switch (hal_sensor->type) {
	case SENSOR_TYPE_ACCELEROMETER:
		hal_sensor->maxRange = max_range * GRAVITY_EARTH;
		break;

	case SENSOR_TYPE_GYROSCOPE:
		hal_sensor->maxRange = max_range * PI / 180;
		break;

	default:
		hal_sensor->maxRange = max_range;
	}

	hal_sensor->resolution = hal_sensor->maxRange / resolution;
	hal_sensor->power = ((float) power_consume) / 1000;

	event->sensor = handle;
	event->acceleration.status = SENSOR_STATUS_ACCURACY_HIGH;

	sensor->scale = hal_sensor->resolution;
	sensor->fake = 0;

	switch (event->type) {
	case SENSOR_TYPE_LIGHT:
	case SENSOR_TYPE_PRESSURE:
	case SENSOR_TYPE_TEMPERATURE:
	case SENSOR_TYPE_PROXIMITY:
		ret = cavan_sensor_event_load_absinfo(sensor, ABS_MISC, -1);
		break;

	case SENSOR_TYPE_ACCELEROMETER:
		ret = ioctl(ctrl_fd, CAVAN_INPUT_SENSOR_IOC_GET_AXIS_COUNT, &axis_count);
		if (ret < 0) {
			pd_red_info("ioctl CAVAN_INPUT_SENSOR_IOC_GET_MAX_RANGE");
			return ret;
		}

		if (axis_count < 3) {
			sensor->fake = GRAVITY_EARTH * GRAVITY_EARTH;
		}
	default:
		ret = cavan_sensor_event_load_absinfo(sensor, ABS_X, ABS_Y, ABS_Z, -1);
	}

	if (ret < 0) {
		pd_red_info("cavan_sensor_event_load_absinfo");
		return ret;
	}

	pd_green_info("Name = %s, Vendor = %s, Handle = %d", hal_sensor->name, hal_sensor->vendor, hal_sensor->handle);
	pd_green_info("maxRange = %f, Resolution = %f", hal_sensor->maxRange, hal_sensor->resolution);
	pd_green_info("Power = %f, minDelay = %d", hal_sensor->power, hal_sensor->minDelay);
	pd_green_info("scale = %f, fake = %f", sensor->scale, sensor->fake);

	return 0;
}

static struct cavan_sensor_device *cavan_sensor_device_add(struct cavan_sensor_device *head, struct cavan_sensor_device *sensor)
{
	if (head) {
		head->prev = sensor;
	}

	sensor->prev = NULL;
	sensor->next = head;

	return sensor;
}

static struct cavan_sensor_device *cavan_sensor_device_remove(struct cavan_sensor_device *head, struct cavan_sensor_device *sensor)
{
	struct cavan_sensor_device *prev = sensor->prev;
	struct cavan_sensor_device *next = sensor->next;

	if (next) {
		next->prev = prev;
	}

	if (prev) {
		prev->next = next;
		return head;
	}

	return next;
}

static int cavan_sensor_epoll_set_enable(struct cavan_sensor_pdev *pdev, int fd, void *data, bool enable)
{
	struct epoll_event event = {
		.events = EPOLLIN,
		.data.ptr = data,
	};

	return epoll_ctl(pdev->epoll_fd, enable ? EPOLL_CTL_ADD : EPOLL_CTL_DEL, fd, &event);
}

static int cavan_sensor_device_set_enable(struct cavan_sensor_pdev *pdev, struct cavan_sensor_device *sensor, bool enable)
{
	int ret;

	cavan_sensor_device_lock(sensor);

	ret = ioctl(sensor->ctrl_fd, CAVAN_INPUT_DEVICE_IOC_SET_ENABLE, enable);
	if (ret < 0) {
		pd_error_info("ioctl CAVAN_INPUT_DEVICE_IOC_SET_ENABLE");
		goto out_cavan_sensor_device_unlock;
	}

	if (sensor->enabled == enable) {
		pd_func_info("Nothing to be done");
		goto out_cavan_sensor_device_unlock;
	}

	cavan_sensor_pdev_lock(pdev);

	ret = cavan_sensor_epoll_set_enable(pdev, sensor->data_fd, sensor, enable);
	if (ret < 0) {
		pr_err_info("cavan_sensor_epoll_set_enable %d: %d", sensor->data_fd, ret);
		goto out_cavan_sensor_pdev_unlock;
	}

	sensor->enabled = enable;

	pd_bold_info("set sensor device %s active %s", sensor->name, enable ? "enable" : "disable");

out_cavan_sensor_pdev_unlock:
	cavan_sensor_pdev_unlock(pdev);
out_cavan_sensor_device_unlock:
	cavan_sensor_device_unlock(sensor);
	return ret;
}

static void cavan_sensors_destory_locked(struct cavan_sensor_pdev *pdev)
{
	struct cavan_sensor_device *sensor;

	for (sensor = pdev->head; sensor; sensor = sensor->next) {
		cavan_sensor_device_destory(sensor);
	}

	pdev->head = NULL;
	pdev->sensor_count = 0;
}

static void cavan_sensors_destory(struct cavan_sensor_pdev *pdev)
{
	cavan_sensor_pdev_lock(pdev);
	cavan_sensors_destory_locked(pdev);
	cavan_sensor_pdev_unlock(pdev);

	pthread_mutex_destroy(&pdev->lock);
}

static void cavan_sensors_remove(struct cavan_sensor_pdev *pdev)
{
	cavan_sensor_pdev_lock(pdev);

	close(pdev->epoll_fd);

	free(pdev->sensor_list);
	pdev->sensor_list = NULL;
	pdev->sensor_map = NULL;

	cavan_sensors_destory_locked(pdev);

	cavan_sensor_pdev_unlock(pdev);

	pthread_mutex_destroy(&pdev->lock);
}

static int cavan_sensors_probe(struct cavan_sensor_pdev *pdev)
{
	int ret;
	int count;
	struct sensor_t *list;
	struct cavan_sensor_device **map;
	struct cavan_sensor_device *sensor, *sensor_next;

	cavan_sensor_pdev_lock(pdev);

	list = malloc((sizeof(*list) + sizeof(*map)) * pdev->sensor_count);
	if (list == NULL) {
		pd_error_info("malloc");
		ret = -ENOMEM;
		goto out_mutex_unlock;
	}

	map = (struct cavan_sensor_device **) (list + pdev->sensor_count);

	for (count = 0, sensor = pdev->head; sensor; sensor = sensor_next) {
		sensor_next = sensor->next;

		ret = cavan_sensor_device_probe(sensor, list + count, count);
		if (ret < 0) {
			pd_red_info("cavan_sensor_chip_probe");
			pdev->head = cavan_sensor_device_remove(pdev->head, sensor);
			cavan_sensor_device_destory(sensor);
			continue;
		}

		map[count++] = sensor;
	}

	pdev->sensor_count = count;
	pdev->sensor_list = list;
	pdev->sensor_map = map;

	if (count == 0) {
		pd_red_info("No sensor chip found!");
		ret = -ENOENT;
		goto out_free_sensor_list;
	}

	pdev->epoll_fd = epoll_create(20);
	if (pdev->epoll_fd < 0) {
		pd_err_info("epoll_create: %d", pdev->epoll_fd);
		ret = pdev->epoll_fd;
		goto out_free_sensor_list;
	}

	ret = pipe(pdev->pipefd);
	if (ret < 0) {
		pd_err_info("pipe: %d", ret);
		goto out_close_epoll_fd;
	}

	cavan_sensor_epoll_set_enable(pdev, pdev->pipefd[0], NULL, true);

	ioctl(map[0]->ctrl_fd, CAVAN_INPUT_CORE_IOC_DISABLE_DET);

	cavan_sensor_pdev_unlock(pdev);

	return 0;

out_close_epoll_fd:
	close(pdev->epoll_fd);
out_free_sensor_list:
	free(pdev->sensor_list);
	pdev->sensor_list = NULL;
	pdev->sensor_map = NULL;
out_mutex_unlock:
	cavan_sensor_pdev_unlock(pdev);
	return ret;
}

static boolean cavan_sensors_match(struct cavan_event_matcher *matcher, void *data)
{
	char pathname[1024];
	struct stat st;

	snprintf(pathname, sizeof(pathname), "/dev/%s", matcher->devname);

	if (stat(pathname, &st) < 0) {
		return false;
	}

	if (major(st.st_rdev) != 280) {
		return false;
	}

	return true;
}

static int cavan_sensors_match_handler(struct cavan_event_matcher *matcher, void *data)
{
	struct cavan_sensor_device *sensor;
	struct cavan_sensor_pdev *pdev = data;

	sensor = cavan_sensor_device_create(matcher->fd, matcher->devname);
	if (sensor == NULL) {
		pd_red_info("cavan_sensor_device_create %s, pathname = %s", matcher->devname, matcher->pathname);
		return -EFAULT;
	}

	pd_bold_info("Add sensor device %s, name = %s", matcher->devname, matcher->pathname);

	cavan_sensor_pdev_lock(pdev);

	pdev->head = cavan_sensor_device_add(pdev->head, sensor);
	pdev->sensor_count++;

	cavan_sensor_pdev_unlock(pdev);

	return 0;
}

static int cavan_sensors_open(struct cavan_sensor_pdev *pdev)
{
	int ret;
	struct cavan_event_matcher matcher = {
		.match = cavan_sensors_match,
		.handler = cavan_sensors_match_handler
	};

	pthread_mutex_init(&pdev->lock, NULL);

	pdev->sensor_count = 0;
	pdev->sensor_list = NULL;
	pdev->sensor_map = NULL;

	pdev->head = NULL;

	ret = cavan_event_scan_devices(&matcher, pdev);
	if (ret < 0) {
		pd_red_info("cavan_event_scan_devices");
		return ret;
	}

	if (pdev->sensor_count == 0) {
		pd_red_info("No device matched");
		return 0; // -ENOENT;
	}

	ret = cavan_sensors_probe(pdev);
	if (ret < 0) {
		pd_red_info("cavan_sensors_probe");
		cavan_sensors_destory(pdev);
		return ret;
	}

	return 0;
}

static int cavan_sensors_activate(struct sensors_poll_device_t *dev, int handle, int enabled)
{
	int ret;
	struct cavan_sensor_pdev *pdev = (struct cavan_sensor_pdev *) dev;
	struct cavan_sensor_device *sensor = pdev->sensor_map[handle];

	pd_bold_info("handle = %d, sensor = %s", handle, sensor->name);

	ret = cavan_sensor_device_set_enable(pdev, sensor, enabled);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

static int cavan_sensors_setDelay(struct sensors_poll_device_t *dev, int handle, int64_t ns)
{
	int ret;
	unsigned int delay = ns / 1000000;
	struct cavan_sensor_pdev *pdev = (struct cavan_sensor_pdev *) dev;
	struct cavan_sensor_device *sensor = pdev->sensor_map[handle];

	pd_func_info("Delay = %d(ms)", delay);

	cavan_sensor_device_lock(sensor);

	ret = ioctl(sensor->ctrl_fd, CAVAN_INPUT_DEVICE_IOC_SET_DELAY, delay);
	if (ret < 0) {
		pd_error_info("ioctl CAVAN_SENSOR_IOCS_DELAY");
	}

	cavan_sensor_device_unlock(sensor);

	return ret;
}

static sensors_event_t *cavan_sensor_report(struct cavan_sensor_pdev *pdev, sensors_event_t *data, sensors_event_t *data_end)
{
	struct input_event *ep;
	struct cavan_sensor_device *sensor = pdev->sensor_pending;

	pdev->sensor_pending = NULL;

	for (ep = pdev->event_pending; ep < pdev->event_end; ep++) {
		switch (ep->type) {
		case EV_SYN:
			if (data >= data_end) {
				pdev->event_pending = ep;
				pdev->sensor_pending = sensor;
				return data;
			}

			if (sensor->fake > 0) {
				float *event_data = sensor->event.data;
				float fake = powf(event_data[0], 2) + powf(event_data[1], 2);

				if (sensor->fake > fake) {
					event_data[2] = sqrtf(sensor->fake - fake);
				} else {
					event_data[2] = 0;
				}
			}

			*data = sensor->event;
			data->timestamp = cavan_timeval2nano(&ep->time);
			// pd_std_info("%s [%f, %f, %f]", sensor->name, data->data[0], data->data[1], data->data[2]);
			data++;
			break;

		case EV_ABS:
			switch (ep->code) {
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

	return data;
}

static int cavan_sensors_poll(struct sensors_poll_device_t *dev, sensors_event_t *data, int size)
{
	int count;
	int rdlen;
	struct epoll_event events[20], *ep, *ep_end;
	struct cavan_sensor_pdev *pdev = (struct cavan_sensor_pdev *) dev;
	struct sensors_event_t *data_bak = data, *data_end = data + size;

	cavan_sensor_pdev_lock(pdev);

	if (pdev->sensor_pending) {
		data = cavan_sensor_report(pdev, data, data_end);
	}

	while (data_bak == data) {
		cavan_sensor_pdev_unlock(pdev);

		count = epoll_wait(pdev->epoll_fd, events, NELEM(events), -1);
		if (count < 0) {
			pd_error_info("epoll_wait");

			if (errno == EINTR) {
				continue;
			}

			return count;
		}

		cavan_sensor_pdev_lock(pdev);

		for (ep = events, ep_end = ep + count; ep < ep_end && data < data_end; ep++) {
			struct cavan_sensor_device *sensor = ep->data.ptr;

			if (sensor == NULL) {
				int handle;

				rdlen = read(pdev->pipefd[0], &handle, sizeof(handle));
				if (rdlen < (int) sizeof(handle)) {
					pd_err_info("read: %d", rdlen);
					break;
				}

				pd_info("flush complete: %d", handle);

				data->sensor = 0;
				data->timestamp = 0;
				data->meta_data.sensor = handle;
				data->meta_data.what = META_DATA_FLUSH_COMPLETE;
				data->type = SENSOR_TYPE_META_DATA;
				data->version = META_DATA_VERSION;
				data++;

				break;
			}

#if 0
			if (unlikely(sensor == NULL)) {
				continue;
			}
#endif

			rdlen = read(sensor->data_fd, pdev->events, sizeof(pdev->events));
			if (rdlen < 0) {
				pd_error_info("read data from chip %s", sensor->name);
				sensor->event.acceleration.status = SENSOR_STATUS_UNRELIABLE;
				cavan_sensor_epoll_set_enable(pdev, sensor->data_fd, sensor, false);
				continue;
			}

			pdev->sensor_pending = sensor;
			pdev->event_pending = pdev->events;
			pdev->event_end = pdev->events + (rdlen / sizeof(struct input_event));
			data = cavan_sensor_report(pdev, data, data_end);
		}
	}

	cavan_sensor_pdev_unlock(pdev);

	return data - data_bak;
}

#ifndef CONFIG_ANDROID_NDK
static int cavan_sensors_batch(struct sensors_poll_device_1 *dev, int handle, int flags, int64_t sampling_period_ns, int64_t max_report_latency_ns)
{
	pd_func_info("handle = %d", handle);

	return cavan_sensors_setDelay(&dev->v0, handle, sampling_period_ns);
}

static int cavan_sensors_flush(struct sensors_poll_device_1 *dev, int handle)
{
	struct cavan_sensor_pdev *pdev = (struct cavan_sensor_pdev *) dev;
	struct cavan_sensor_device *sensor = pdev->sensor_map[handle];
	int ret;

	pd_func_info("handle = %d", handle);

	cavan_sensor_device_lock(sensor);

	if (sensor->enabled) {
		ret = write(pdev->pipefd[1], &handle, sizeof(handle));
		if (ret < 0) {
			pr_err_info("write: %d", ret);
		} else {
			ret = 0;
		}
	} else {
		ret = -EINVAL;
	}

	cavan_sensor_device_unlock(sensor);

	return ret;
}
#endif

// ================================================================================

static int cavan_sensors_module_close(struct hw_device_t *device)
{
	pd_func_info("device = %p", device);

	cavan_sensors_remove((struct cavan_sensor_pdev *) device);

	return 0;
}

static struct cavan_sensor_pdev cavan_poll_device = {
	.device = {
		.common = {
			.tag = HARDWARE_DEVICE_TAG,
			.version = SENSORS_DEVICE_API_VERSION_1_3,
			.close = cavan_sensors_module_close
		},
		.activate = cavan_sensors_activate,
		.setDelay = cavan_sensors_setDelay,
		.poll = cavan_sensors_poll,
#ifndef CONFIG_ANDROID_NDK
		.batch = cavan_sensors_batch,
		.flush = cavan_sensors_flush,
#endif
	},
};

static int cavan_sensors_module_open(const struct hw_module_t *module, const char *id, struct hw_device_t **device)
{
	int ret;
	struct hw_device_t *hw_dev;

	pd_func_info("module = %p, id = %s", module, id);

	ret = cavan_sensors_open(&cavan_poll_device);
	if (ret < 0) {
		pd_red_info("cavan_sensors_open");
		return ret;
	}

	hw_dev = &cavan_poll_device.device.common;
	hw_dev->module = (struct hw_module_t *) module;
	*device = hw_dev;

	return 0;
}

static int cavan_sensors_get_list(struct sensors_module_t *module, struct sensor_t const **list)
{
	pd_func_info("module = %p, sensor_count = %d", module, (int) cavan_poll_device.sensor_count);

	*list = cavan_poll_device.sensor_list;

	return cavan_poll_device.sensor_count;
}

static struct hw_module_methods_t cavan_sensors_module_methods = {
	.open = cavan_sensors_module_open
};

struct sensors_module_t HAL_MODULE_INFO_SYM = {
	.common = {
		.tag = HARDWARE_MODULE_TAG,
		.version_major = 1,
		.version_minor = 0,
		.id = SENSORS_HARDWARE_MODULE_ID,
		.name = "Cavan Sensor HAL",
		.author = "Fuang.Cao",
		.methods = &cavan_sensors_module_methods
	},

	.get_sensors_list = cavan_sensors_get_list
};
