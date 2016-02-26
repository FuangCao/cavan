/*
 * File:		sensor_poll.cpp
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2014-11-25 11:55:09
 *
 * Copyright (c) 2014 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <pthread.h>
#include <cavan/command.h>
#include <hardware/sensors.h>
#include <android/sensor.h>
#include <gui/Sensor.h>
#include <gui/SensorManager.h>
#include <gui/SensorEventQueue.h>
#include <utils/Looper.h>

using namespace android;

struct cavan_sensor_module {
	struct sensors_module_t *module;
	struct sensors_poll_device_t *device;

	u32 mask;
	int count;
	const struct sensor_t *list[32];
};

static void show_usage(const char *command)
{
	pr_std_info("Usage: %s [option]", command);
	pr_std_info("--help, -H, -h\t\t\tshow this help");
	pr_std_info("--all, -A, -a\t\t\tenable all sensor");
	pr_std_info("--delay, -D, -d\t\t\tpoll delay ms");
	pr_std_info("--hal\t\t\t\tpoll sensor hal, default poll sensor service");
	pr_std_info("--acc, --gsensor, -G, -g\tenable accelerometer sensor");
	pr_std_info("--mag, -M, -m\t\t\tenable magnetic sensor");
	pr_std_info("--orient, -O, -o\t\tenable orientation sensor");
	pr_std_info("--gyro\t\t\t\tenable gyroscope sensor");
	pr_std_info("--light, -L, -l\t\t\tenable light sensor");
	pr_std_info("--pressure\t\t\tenable pressure sensor");
	pr_std_info("--temp, -T, -t\t\t\tenable temperature sensor");
	pr_std_info("--prox, -P, -p\t\t\tenable proximity sensor");
	pr_std_info("--gravity\t\t\tenable gravity sensor");
	pr_std_info("--la, --lg\t\t\tenable liner accelerometer sensor");
	pr_std_info("--rv, -R, -r\t\t\tenable rotation vector sensor");
}

static int cavan_sensor_module_open(struct cavan_sensor_module *module)
{
	int i;
	int ret;
	int count;
	const struct sensor_t *list;
	const struct sensor_t *sensor;

	ret = hw_get_module(SENSORS_HARDWARE_MODULE_ID, (const struct hw_module_t **) &module->module);
	if (ret < 0) {
		pr_red_info("hw_get_module: %d", ret);
		return ret;
	}

	ret = sensors_open(&module->module->common, &module->device);
	if (ret < 0) {
		pr_red_info("sensors_open: %d", ret);
		return ret;
	}

	count = module->module->get_sensors_list(module->module, &list);
	if (count < 0) {
		pr_red_info("get_sensors_list");
		ret = count;
		goto out_sensors_close;
	}

	module->mask = 0;

	for (i = 0; i < NELEM(module->list); i++) {
		module->list[i] = NULL;
	}

	pr_std_info("============================================================");

	for (sensor = list + count - 1; sensor >= list; sensor--) {
		module->mask |= 1 << sensor->type;
		module->list[sensor->type] = sensor;

		pr_bold_info("version = %d, type = %d, handle = %d", sensor->version, sensor->type, sensor->handle);
		pr_bold_info("maxRange = %f", sensor->maxRange);
		pr_bold_info("resolution = %f", sensor->resolution);
		pr_bold_info("power = %f", sensor->power);
		pr_bold_info("minDelay = %d", sensor->minDelay);

		pr_std_info("============================================================");
	}

	module->count = count;

	println("count = %d, mask = 0x%08x", module->count, module->mask);

	return 0;

out_sensors_close:
	sensors_close(module->device);
	return ret;
}

static void cavan_sensor_module_close(struct cavan_sensor_module *module)
{
	sensors_close(module->device);
}

static inline const struct sensor_t *cavan_sensor_get(struct cavan_sensor_module *module, int type)
{
	return module->list[type];
}

static int cavan_sensor_set_enable(struct cavan_sensor_module *module, int type, bool enable)
{
	int ret;
	const struct sensor_t *sensor = cavan_sensor_get(module, type);

	if (sensor == NULL) {
		pr_red_info("sensor %d not found", type);
		return -ENOENT;
	}

	ret = module->device->activate(module->device, sensor->handle, enable);
	if (ret < 0) {
		return ret;
	}

	if (enable) {
		pr_green_info("Enable sensor: %s", sensor->name);
	} else {
		pr_brown_info("Disable sensor: %s", sensor->name);
	}

	return 0;
}

static int cavan_sensor_set_delay(struct cavan_sensor_module *module, int type, int64_t delay_ns)
{
	const struct sensor_t *sensor = cavan_sensor_get(module, type);

	if (sensor == NULL) {
		pr_red_info("sensor %d not found", type);
		return -ENOENT;
	}

	return module->device->setDelay(module->device, sensor->handle, delay_ns);
}

static int cavan_sensor_set_enable_mask(struct cavan_sensor_module *module, u32 mask, int64_t delay_ms)
{
	int i;
	int ret;

	for (i = 0; i < NELEM(module->list); i++) {
		if ((module->mask & (1 << i)) == 0) {
			continue;
		}

		if (mask & (1 << i)) {
			ret = cavan_sensor_set_enable(module, i, true);
			if (ret < 0) {
				pr_red_info("cavan_sensor_set_enable");
				return ret;
			}

			ret = cavan_sensor_set_delay(module, i, delay_ms * 1000 * 1000);
			if (ret < 0) {
				pr_red_info("cavan_sensor_set_delay");
				return ret;
			}
		} else {
			ret = cavan_sensor_set_enable(module, i, false);
			if (ret < 0) {
				pr_red_info("cavan_sensor_set_enable");
				return ret;
			}
		}
	}

	return 0;
}

static void *sensor_poll_thread(void *data)
{
	sensors_event_t events[8], *p, *pe_end;
	struct cavan_sensor_module *module = (struct cavan_sensor_module *) data;
	struct sensors_poll_device_t *sensor_device = module->device;

	while (1) {
		int count = sensor_device->poll(sensor_device, events, NELEM(events));

		if (count < 0) {
			break;
		}

		for (p = events, pe_end = p + count; p < pe_end; p++) {
			switch (p->type) {
			case SENSOR_TYPE_ACCELEROMETER:
				pr_std_info("Accelerometer: [%f, %f, %f]", p->data[0], p->data[1], p->data[2]);
				break;

			case SENSOR_TYPE_MAGNETIC_FIELD:
				pr_std_info("Magnetic_Field: [%f, %f, %f]", p->data[0], p->data[1], p->data[2]);
				break;

			case SENSOR_TYPE_ORIENTATION:
				pr_std_info("Orientation: [%f, %f, %f]", p->data[0], p->data[1], p->data[2]);
				break;

			case SENSOR_TYPE_GYROSCOPE:
				pr_std_info("Gyroscope: [%f, %f, %f]", p->data[0], p->data[1], p->data[2]);
				break;

			case SENSOR_TYPE_LIGHT:
				pr_std_info("Light: [%f]", p->data[0]);
				break;

			case SENSOR_TYPE_PRESSURE:
				pr_std_info("Pressure: [%f]", p->data[0]);
				break;

			case SENSOR_TYPE_TEMPERATURE:
				pr_std_info("Temperature: [%f]", p->data[0]);
				break;

			case SENSOR_TYPE_PROXIMITY:
				pr_std_info("Proximity: [%f]", p->data[0]);
				break;

			case SENSOR_TYPE_GRAVITY:
				pr_std_info("Gravity: [%f, %f, %f]", p->data[0], p->data[1], p->data[2]);
				break;

			case SENSOR_TYPE_LINEAR_ACCELERATION:
				pr_std_info("Linear_Acceleration: [%f, %f, %f]", p->data[0], p->data[1], p->data[2]);
				break;

			case SENSOR_TYPE_ROTATION_VECTOR:
				pr_std_info("Rotation_Vecto: [%f, %f, %f]", p->data[0], p->data[1], p->data[2]);
				break;
			}
		}
	}

	return NULL;
}

static int sensor_poll_main_loop(u32 mask, u32 delay)
{
	int i;
	int ret;
	pthread_t thread;
	struct cavan_sensor_module module;

	pr_bold_info("Start poll sensor hal");

	ret = cavan_sensor_module_open(&module);
	if (ret < 0) {
		pr_red_info("cavan_sensor_module_open");
		return ret;
	}

	ret = pthread_create(&thread, NULL, sensor_poll_thread, &module);
	if (ret < 0) {
		pr_err_info("pthread_create: %d", ret);
		goto out_cavan_sensor_module_close;
	}

	if (mask) {
		cavan_sensor_set_enable_mask(&module, mask, delay);
	} else {
		cavan_sensor_set_enable_mask(&module, 0, delay);

		while (1) {
			for (i = 0; i < NELEM(module.list); i++) {
				if ((module.mask & (1 << i)) == 0) {
					continue;
				}

				cavan_sensor_set_enable(&module, i, true);
				msleep(5000);
				cavan_sensor_set_enable(&module, i, false);
				msleep(5000);
			}
		}
	}

	pthread_join(thread, NULL);

out_cavan_sensor_module_close:
	cavan_sensor_module_close(&module);
	return ret;
}

static int sensor_event_receiver(int fd, int events, void *data)
{
	sp<SensorEventQueue> queue((SensorEventQueue *) data);

	while (1) {
		ASensorEvent buff[8];
		ASensorEvent *p, *p_end;
		ssize_t count = queue->read(buff, NELEM(buff));

		if (count <= 0) {
			if (count < 0) {
				pr_red_info("queue->read: %d", count);
			}

			break;
		}

		for (p = buff, p_end = p + count; p < p_end; p++) {
			switch (p->type) {
			case SENSOR_TYPE_ACCELEROMETER:
				pr_std_info("Accelerometer: [%f, %f, %f]", p->data[0], p->data[1], p->data[2]);
				break;

			case SENSOR_TYPE_MAGNETIC_FIELD:
				pr_std_info("Magnetic_Field: [%f, %f, %f]", p->data[0], p->data[1], p->data[2]);
				break;

			case SENSOR_TYPE_ORIENTATION:
				pr_std_info("Orientation: [%f, %f, %f]", p->data[0], p->data[1], p->data[2]);
				break;

			case SENSOR_TYPE_GYROSCOPE:
				pr_std_info("Gyroscope: [%f, %f, %f]", p->data[0], p->data[1], p->data[2]);
				break;

			case SENSOR_TYPE_LIGHT:
				pr_std_info("Light: [%f]", p->data[0]);
				break;

			case SENSOR_TYPE_PRESSURE:
				pr_std_info("Pressure: [%f]", p->data[0]);
				break;

			case SENSOR_TYPE_TEMPERATURE:
				pr_std_info("Temperature: [%f]", p->data[0]);
				break;

			case SENSOR_TYPE_PROXIMITY:
				pr_std_info("Proximity: [%f]", p->data[0]);
				break;

			case SENSOR_TYPE_GRAVITY:
				pr_std_info("Gravity: [%f, %f, %f]", p->data[0], p->data[1], p->data[2]);
				break;

			case SENSOR_TYPE_LINEAR_ACCELERATION:
				pr_std_info("Linear_Acceleration: [%f, %f, %f]", p->data[0], p->data[1], p->data[2]);
				break;

			case SENSOR_TYPE_ROTATION_VECTOR:
				pr_std_info("Rotation_Vecto: [%f, %f, %f]", p->data[0], p->data[1], p->data[2]);
				break;
			}
		}
	}

	return 1;
}

static int sensor_service_poll_main_loop(u32 mask, u32 delay)
{
	ssize_t count;
	Sensor const* const* list;
	SensorManager &manager(SensorManager::getInstance());

	pr_bold_info("Start poll sensor service");

	count = manager.getSensorList(&list);
	if (count < 0) {
		pr_red_info("getSensorList");
		return count;
	}

	println("sensor count = %d", int(count));

	sp<SensorEventQueue> queue = manager.createEventQueue();

	for (int i = 0; i < 32; i++) {
		Sensor const* sensor = manager.getDefaultSensor(i);
		if (sensor == NULL) {
			continue;
		}

		if (mask & (1 << i)) {
			pr_green_info("Enable sensor: %s", sensor->getName().string());
			queue->enableSensor(sensor);
			queue->setEventRate(sensor, ms2ns(delay));
		} else {
			pr_brown_info("Disable sensor: %s", sensor->getName().string());
			queue->disableSensor(sensor);
		}
	}

	sp<Looper> loop = new Looper(false);
	loop->addFd(queue->getFd(), 0, ALOOPER_EVENT_INPUT, sensor_event_receiver, queue.get());

	while (1) {
		int result = loop->pollOnce(-1);
		switch (result) {
		case ALOOPER_POLL_WAKE:
			println("ALOOPER_POLL_WAKE");
			break;
		case ALOOPER_POLL_CALLBACK:
			// println("ALOOPER_POLL_CALLBACK");
			break;
		case ALOOPER_POLL_TIMEOUT:
			pr_red_info("ALOOPER_POLL_TIMEOUT");
			break;
		case ALOOPER_POLL_ERROR:
			pr_red_info("ALOOPER_POLL_TIMEOUT");
			break;
		default:
			pr_red_info("poll returned %d", result);
			break;
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int c;
	u32 mask;
	u32 delay;
	int option_index;
	bool test_hal = false;
	struct option long_option[] = {
		{
			.name = "help",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_HELP,
		}, {
			.name = "all",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_ALL,
		}, {
			.name = "delay",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_DELAY,
		}, {
			.name = "hal",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_HAL,
		}, {
			.name = "acc",
			.has_arg = no_argument,
			.flag = NULL,
			.val = SENSOR_TYPE_ACCELEROMETER,
		}, {
			.name = "gsensor",
			.has_arg = no_argument,
			.flag = NULL,
			.val = SENSOR_TYPE_ACCELEROMETER,
		}, {
			.name = "mag",
			.has_arg = no_argument,
			.flag = NULL,
			.val = SENSOR_TYPE_MAGNETIC_FIELD,
		}, {
			.name = "orient",
			.has_arg = no_argument,
			.flag = NULL,
			.val = SENSOR_TYPE_ORIENTATION,
		}, {
			.name = "gyro",
			.has_arg = no_argument,
			.flag = NULL,
			.val = SENSOR_TYPE_GYROSCOPE,
		}, {
			.name = "light",
			.has_arg = no_argument,
			.flag = NULL,
			.val = SENSOR_TYPE_LIGHT,
		}, {
			.name = "pressure",
			.has_arg = no_argument,
			.flag = NULL,
			.val = SENSOR_TYPE_PRESSURE,
		}, {
			.name = "temp",
			.has_arg = no_argument,
			.flag = NULL,
			.val = SENSOR_TYPE_TEMPERATURE,
		}, {
			.name = "prox",
			.has_arg = no_argument,
			.flag = NULL,
			.val = SENSOR_TYPE_PROXIMITY,
		}, {
			.name = "gravity",
			.has_arg = no_argument,
			.flag = NULL,
			.val = SENSOR_TYPE_GRAVITY,
		}, {
			.name = "la",
			.has_arg = no_argument,
			.flag = NULL,
			.val = SENSOR_TYPE_LINEAR_ACCELERATION,
		}, {
			.name = "lg",
			.has_arg = no_argument,
			.flag = NULL,
			.val = SENSOR_TYPE_LINEAR_ACCELERATION,
		}, {
			.name = "rv",
			.has_arg = no_argument,
			.flag = NULL,
			.val = SENSOR_TYPE_ROTATION_VECTOR,
		}, {
			0, 0, 0, 0
		},
	};

	mask = 0;
	delay = 200;

	while ((c = getopt_long(argc, argv, "HhAaD:d:GgMmLlPpTtRr", long_option, &option_index)) != EOF) {
		switch (c) {
		case 'h':
		case 'H':
		case CAVAN_COMMAND_OPTION_HELP:
			show_usage(argv[0]);
			return 0;

		case 'a':
		case 'A':
		case CAVAN_COMMAND_OPTION_ALL:
			mask = -1;
			break;

		case 'd':
		case 'D':
		case CAVAN_COMMAND_OPTION_DELAY:
			delay = text2value_unsigned(optarg, NULL, 10);
			break;

		case CAVAN_COMMAND_OPTION_HAL:
			test_hal = true;
			break;

		case 'g':
		case 'G':
			mask |= 1 << SENSOR_TYPE_ACCELEROMETER;
			break;

		case 'm':
		case 'M':
			mask |= 1 << SENSOR_TYPE_MAGNETIC_FIELD;
			break;

		case 'l':
		case 'L':
			mask |= 1 << SENSOR_TYPE_LIGHT;
			break;

		case 'p':
		case 'P':
			mask |= 1 << SENSOR_TYPE_PROXIMITY;
			break;

		case 't':
		case 'T':
			mask |= 1 << SENSOR_TYPE_TEMPERATURE;
			break;

		case 'r':
		case 'R':
			mask |= 1 << SENSOR_TYPE_ROTATION_VECTOR;
			break;

		case SENSOR_TYPE_ACCELEROMETER:
		case SENSOR_TYPE_MAGNETIC_FIELD:
		case SENSOR_TYPE_ORIENTATION:
		case SENSOR_TYPE_GYROSCOPE:
		case SENSOR_TYPE_LIGHT:
		case SENSOR_TYPE_PRESSURE:
		case SENSOR_TYPE_TEMPERATURE:
		case SENSOR_TYPE_PROXIMITY:
		case SENSOR_TYPE_GRAVITY:
		case SENSOR_TYPE_LINEAR_ACCELERATION:
		case SENSOR_TYPE_ROTATION_VECTOR:
			mask |= 1 << c;
			break;

		default:
			pr_red_info("Invalid option %c", c);
			show_usage(argv[0]);
			return -EINVAL;
		}
	}

	if (test_hal) {
		return sensor_poll_main_loop(mask, delay);
	} else {
		return sensor_service_poll_main_loop(mask, delay);
	}
}
