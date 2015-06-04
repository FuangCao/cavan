/*
 * File:			cavan_sensor.h
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

#pragma once

#ifndef LOG_TAG
#define LOG_TAG "Sensors"
#endif

#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <linux/input.h>
#include <utils/Log.h>
#include <utils/Atomic.h>
#include <utils/Timers.h>
#include <hardware/sensors.h>

#define CAVAN_INPUT_IOC_LENGTH_TO_MASK(len) \
	((1 << (len)) - 1)

#define CAVAN_INPUT_IOC_GET_VALUE(cmd, shift, mask) \
	(((cmd) >> (shift)) & (mask))

#define CAVAN_INPUT_IOC_TYPE_LEN		8
#define CAVAN_INPUT_IOC_NR_LEN		8
#define CAVAN_INPUT_IOC_SIZE_LEN		16

#define CAVAN_INPUT_IOC_TYPE_MASK		CAVAN_INPUT_IOC_LENGTH_TO_MASK(CAVAN_INPUT_IOC_TYPE_LEN)
#define CAVAN_INPUT_IOC_NR_MASK		CAVAN_INPUT_IOC_LENGTH_TO_MASK(CAVAN_INPUT_IOC_NR_LEN)
#define CAVAN_INPUT_IOC_SIZE_MASK		CAVAN_INPUT_IOC_LENGTH_TO_MASK(CAVAN_INPUT_IOC_SIZE_LEN)

#define CAVAN_INPUT_IOC_TYPE_SHIFT	0
#define CAVAN_INPUT_IOC_NR_SHIFT		(CAVAN_INPUT_IOC_TYPE_SHIFT + CAVAN_INPUT_IOC_TYPE_LEN)
#define CAVAN_INPUT_IOC_SIZE_SHIFT	(CAVAN_INPUT_IOC_NR_SHIFT + CAVAN_INPUT_IOC_NR_LEN)

#define CAVAN_INPUT_IOC_GET_TYPE(cmd) \
	CAVAN_INPUT_IOC_GET_VALUE(cmd, CAVAN_INPUT_IOC_TYPE_SHIFT, CAVAN_INPUT_IOC_TYPE_MASK)

#define CAVAN_INPUT_IOC_GET_NR(cmd) \
	CAVAN_INPUT_IOC_GET_VALUE(cmd, CAVAN_INPUT_IOC_NR_SHIFT, CAVAN_INPUT_IOC_NR_MASK)

#define CAVAN_INPUT_IOC_GET_SIZE(cmd) \
	CAVAN_INPUT_IOC_GET_VALUE(cmd, CAVAN_INPUT_IOC_SIZE_SHIFT, CAVAN_INPUT_IOC_SIZE_MASK)

#define CAVAN_INPUT_IOC_GET_CMD_RAW(cmd) \
	((cmd) & CAVAN_INPUT_IOC_LENGTH_TO_MASK(CAVAN_INPUT_IOC_TYPE_LEN + CAVAN_INPUT_IOC_NR_LEN))

#define CAVAN_INPUT_IOC(type, nr, size) \
	(((type) & CAVAN_INPUT_IOC_TYPE_MASK) << CAVAN_INPUT_IOC_TYPE_SHIFT | \
	((nr) & CAVAN_INPUT_IOC_NR_MASK) << CAVAN_INPUT_IOC_NR_SHIFT | \
	((size) & CAVAN_INPUT_IOC_SIZE_MASK) << CAVAN_INPUT_IOC_SIZE_SHIFT)

#define CAVAN_INPUT_CHIP_IOC_GET_NAME(len)	CAVAN_INPUT_IOC('I', 0x00, len)
#define CAVAN_INPUT_CHIP_IOC_GET_VENDOR(len)	CAVAN_INPUT_IOC('I', 0x01, len)
#define CAVAN_INPUT_CHIP_IOC_SET_FW_SIZE		CAVAN_INPUT_IOC('I', 0x02, 0)

#define CAVAN_INPUT_DEVICE_IOC_GET_TYPE		CAVAN_INPUT_IOC('I', 0x10, 0)
#define CAVAN_INPUT_DEVICE_IOC_GET_NAME(len)	CAVAN_INPUT_IOC('I', 0x11, len)
#define CAVAN_INPUT_DEVICE_IOC_SET_DELAY		CAVAN_INPUT_IOC('I', 0x12, 0)
#define CAVAN_INPUT_DEVICE_IOC_SET_ENABLE		CAVAN_INPUT_IOC('I', 0x13, 0)

#define CAVAN_INPUT_SENSOR_IOC_GET_MIN_DELAY		CAVAN_INPUT_IOC('S', 0x00, 0)
#define CAVAN_INPUT_SENSOR_IOC_GET_MAX_RANGE		CAVAN_INPUT_IOC('S', 0x01, 0)
#define CAVAN_INPUT_SENSOR_IOC_GET_RESOLUTION		CAVAN_INPUT_IOC('S', 0x02, 0)
#define CAVAN_INPUT_SENSOR_IOC_GET_POWER_CONSUME	CAVAN_INPUT_IOC('S', 0x03, 0)
#define CAVAN_INPUT_SENSOR_IOC_GET_AXIS_COUNT		CAVAN_INPUT_IOC('S', 0x04, 0)

#define pr_std_info(fmt, args ...) \
	ALOGD(fmt "\n", ##args)

#define pr_pos_info() \
	pr_std_info("%s => %s[%d]", __FILE__, __FUNCTION__, __LINE__)

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

enum cavan_input_device_type
{
	CAVAN_INPUT_DEVICE_TYPE_NONE,
	CAVAN_INPUT_DEVICE_TYPE_TOUCHSCREEN,
	CAVAN_INPUT_DEVICE_TYPE_ACCELEROMETER,
	CAVAN_INPUT_DEVICE_TYPE_MAGNETIC_FIELD,
	CAVAN_INPUT_DEVICE_TYPE_ORIENTATION,
	CAVAN_INPUT_DEVICE_TYPE_GYROSCOPE,
	CAVAN_INPUT_DEVICE_TYPE_LIGHT,
	CAVAN_INPUT_DEVICE_TYPE_PRESSURE,
	CAVAN_INPUT_DEVICE_TYPE_TEMPERATURE,
	CAVAN_INPUT_DEVICE_TYPE_PROXIMITY,
	CAVAN_INPUT_DEVICE_TYPE_GRAVITY,
	CAVAN_INPUT_DEVICE_TYPE_LINEAR_ACCELERATION,
	CAVAN_INPUT_DEVICE_TYPE_ROTATION_VECTOR
};

typedef enum
{
	false = 0,
	true
} bool;

struct cavan_sensor_device
{
	int data_fd;
	int ctrl_fd;
	char name[128];
	char vensor[128];

	float scale;
	float fake;
	pthread_mutex_t lock;

	bool enabled;
	struct pollfd *pfd;
	struct sensors_event_t event;

	struct cavan_sensor_device *prev;
	struct cavan_sensor_device *next;
};

struct cavan_sensor_poll_device
{
	struct sensors_poll_device_t device;

	pthread_mutex_t lock;

	size_t sensor_count;
	struct sensor_t *sensor_list;
	struct cavan_sensor_device **sensor_map;

	int pipefd[2];
	size_t pollfd_count;
	struct pollfd *pollfd_list;

	struct cavan_sensor_device *active_head;
	struct cavan_sensor_device *inactive_head;
};

int text_lhcmp(const char *text1, const char *text2);
char *text_copy(char *dest, const char *src);
char *text_ncopy(char *dest, const char *src, size_t size);

static inline int64_t timeval2nano(struct timeval *time)
{
	return time->tv_sec * 1000000000LL + time->tv_usec * 1000;
}

static inline void cavan_sensor_event_init(struct sensors_event_t *event)
{
	memset(event, 0, sizeof(*event));
	event->version = sizeof(*event);
}

static inline int cavan_input_get_devname(int fd, char *devname, size_t size)
{
	return ioctl(fd, EVIOCGNAME(size), devname);
}
