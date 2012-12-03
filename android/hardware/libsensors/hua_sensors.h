/*
 * File:			hua_sensors.h
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

#pragma once

#ifndef LOG_TAG
#define LOG_TAG "Sensors"
#endif

#include <sys/types.h>
#include <dirent.h>
#include <poll.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <linux/input.h>
#include <utils/Log.h>
#include <utils/Atomic.h>
#include <hardware/sensors.h>

#define HUA_SENSOR_IOCG_MAX_RANGE		_IOR('H', 0x01, unsigned int)
#define HUA_SENSOR_IOCG_RESOLUTION		_IOR('H', 0x02, unsigned int)
#define HUA_SENSOR_IOCG_POWER_CONSUME	_IOR('H', 0x03, unsigned int)
#define HUA_SENSOR_IOCG_MIN_DELAY		_IOR('H', 0x04, unsigned int)

#define HUA_SENSOR_IOCS_ENABLE			_IOW('H', 0x05, unsigned int)
#define HUA_SENSOR_IOCS_DELAY			_IOW('H', 0x06, unsigned int)

#define pr_std_info(fmt, args ...) \
	LOGD(fmt "\n", ##args)

#define pr_pos_info() \
	pr_std_pos("%s => %s[%d]")

#define pr_func_info(fmt, args ...) \
	pr_std_info("%s[%d]: " fmt, __FUNCTION__, __LINE__, ##args)

#define pr_red_info(fmt, args ...) \
	pr_std_info("\033[31m" fmt "\033[0m", ##args)

#define pr_green_info(fmt, args ...) \
	pr_std_info("\033[32m" fmt "\033[0m", ##args)

#define pr_bold_info(fmt, args ...) \
	pr_std_info("\033[1m" fmt "\033[0m", ##args)

#define pr_error_info(fmt, args ...) \
	if (errno) { \
		pr_red_info("%s[%d] (" fmt "): %s", __FUNCTION__, __LINE__, ##args, strerror(errno)); \
	} else { \
		pr_red_info("%s[%d]:" fmt, __FUNCTION__, __LINE__, ##args); \
	}

typedef enum
{
	false = 0,
	true
} bool;

struct hua_sensor_device
{
	int data_fd;
	int ctrl_fd;
	char name[128];

	unsigned int active;
	pthread_mutex_t lock;

	struct pollfd *pfd;
	struct sensors_event_t event;

	struct hua_sensor_device *prev;
	struct hua_sensor_device *next;

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

	pthread_mutex_t lock;

	int sensor_count;
	struct sensor_t *sensor_list;
	struct hua_sensor_device **sensor_map;

	int pipefd[2];
	size_t poll_count;
	struct pollfd *pfd_list;

	struct hua_sensor_device *active_head;
	struct hua_sensor_device *inactive_head;
};

int text_lhcmp(const char *text1, const char *text2);
char *text_copy(char *dest, const char *src);
char *text_ncopy(char *dest, const char *src, size_t size);
ssize_t hua_sensors_scan_devices(int (*match_handle)(int fd, const char *pathname, const char *devname, void *data), void *data);

struct hua_sensor_device *hua_gsensor_create(void);

static inline int64_t timeval2nano(struct timeval *time)
{
	return time->tv_sec * 1000000000LL + time->tv_usec * 1000;
}

static inline void hua_sensor_event_init(struct sensors_event_t *event)
{
	memset(event, 0, sizeof(*event));
	event->version = sizeof(*event);
}

static inline int cavan_event_get_devname(int fd, char *devname, size_t size)
{
	return ioctl(fd, EVIOCGNAME(size), devname);
}
