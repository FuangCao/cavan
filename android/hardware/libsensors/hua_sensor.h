/*
 * File:			hua_sensor.h
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
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <linux/input.h>
#include <utils/Log.h>
#include <utils/Atomic.h>
#include <utils/Timers.h>
#include <hardware/sensors.h>

#define HUA_SENSOR_IOC_LENGTH_TO_MASK(len) \
	((1 << (len)) - 1)

#define HUA_SENSOR_IOC_GET_VALUE(cmd, shift, mask) \
	(((cmd) >> (shift)) & (mask))

#define HUA_SENSOR_IOC_TYPE_LEN		8
#define HUA_SENSOR_IOC_NR_LEN		8
#define HUA_SENSOR_IOC_INDEX_LEN	4
#define HUA_SENSOR_IOC_SIZE_LEN		12

#define HUA_SENSOR_IOC_TYPE_MASK	HUA_SENSOR_IOC_LENGTH_TO_MASK(HUA_SENSOR_IOC_TYPE_LEN)
#define HUA_SENSOR_IOC_NR_MASK		HUA_SENSOR_IOC_LENGTH_TO_MASK(HUA_SENSOR_IOC_NR_LEN)
#define HUA_SENSOR_IOC_INDEX_MASK	HUA_SENSOR_IOC_LENGTH_TO_MASK(HUA_SENSOR_IOC_INDEX_LEN)
#define HUA_SENSOR_IOC_SIZE_MASK	HUA_SENSOR_IOC_LENGTH_TO_MASK(HUA_SENSOR_IOC_SIZE_LEN)

#define HUA_SENSOR_IOC_TYPE_SHIFT	0
#define HUA_SENSOR_IOC_NR_SHIFT		(HUA_SENSOR_IOC_TYPE_SHIFT + HUA_SENSOR_IOC_TYPE_LEN)
#define HUA_SENSOR_IOC_INDEX_SHIFT	(HUA_SENSOR_IOC_NR_SHIFT + HUA_SENSOR_IOC_NR_LEN)
#define HUA_SENSOR_IOC_SIZE_SHIFT	(HUA_SENSOR_IOC_INDEX_SHIFT + HUA_SENSOR_IOC_INDEX_LEN)

#define HUA_SENSOR_IOC_GET_TYPE(cmd) \
	HUA_SENSOR_IOC_GET_VALUE(cmd, HUA_SENSOR_IOC_TYPE_SHIFT, HUA_SENSOR_IOC_TYPE_MASK)

#define HUA_SENSOR_IOC_GET_NR(cmd) \
	HUA_SENSOR_IOC_GET_VALUE(cmd, HUA_SENSOR_IOC_NR_SHIFT, HUA_SENSOR_IOC_NR_MASK)

#define HUA_SENSOR_IOC_GET_INDEX(cmd) \
	HUA_SENSOR_IOC_GET_VALUE(cmd, HUA_SENSOR_IOC_INDEX_SHIFT, HUA_SENSOR_IOC_INDEX_MASK)

#define HUA_SENSOR_IOC_GET_SIZE(cmd) \
	HUA_SENSOR_IOC_GET_VALUE(cmd, HUA_SENSOR_IOC_SIZE_SHIFT, HUA_SENSOR_IOC_SIZE_MASK)

#define HUA_SENSOR_IOC_GET_CMD_RAW(cmd) \
	((cmd) & HUA_SENSOR_IOC_LENGTH_TO_MASK(HUA_SENSOR_IOC_TYPE_LEN + HUA_SENSOR_IOC_NR_LEN))

#define HUA_SENSOR_IOC(type, nr, index, size) \
	(((type) & HUA_SENSOR_IOC_TYPE_MASK) << HUA_SENSOR_IOC_TYPE_SHIFT | \
	((nr) & HUA_SENSOR_IOC_NR_MASK) << HUA_SENSOR_IOC_NR_SHIFT | \
	((index) & HUA_SENSOR_IOC_INDEX_MASK) << HUA_SENSOR_IOC_INDEX_SHIFT | \
	((size) & HUA_SENSOR_IOC_SIZE_MASK) << HUA_SENSOR_IOC_SIZE_SHIFT)

#define HUA_SENSOR_IOC_GET_CHIP_NAME(len)			HUA_SENSOR_IOC('H', 0x01, 0, len)
#define HUA_SENSOR_IOC_GET_CHIP_VENDOR(len)			HUA_SENSOR_IOC('H', 0x02, 0, len)
#define HUA_SENSOR_IOC_GET_SENSOR_COUNT				HUA_SENSOR_IOC('H', 0x03, 0, 0)
#define HUA_SENSOR_IOC_GET_MIN_DELAY				HUA_SENSOR_IOC('H', 0x04, 0, 0)
#define HUA_SENSOR_IOC_GET_SENSOR_TYPE(index)		HUA_SENSOR_IOC('H', 0x05, index, 0)
#define HUA_SENSOR_IOC_GET_SENSOR_NAME(index, len)	HUA_SENSOR_IOC('H', 0x06, index, len)
#define HUA_SENSOR_IOC_GET_MAX_RANGE(index)			HUA_SENSOR_IOC('H', 0x07, index, 0)
#define HUA_SENSOR_IOC_GET_RESOLUTION(index)		HUA_SENSOR_IOC('H', 0x08, index, 0)
#define HUA_SENSOR_IOC_GET_POWER_CONSUME(index)		HUA_SENSOR_IOC('H', 0x09, index, 0)
#define HUA_SENSOR_IOC_SET_DELAY(index)				HUA_SENSOR_IOC('H', 0x0A, index, 0)
#define HUA_SENSOR_IOC_SET_ENABLE(index)			HUA_SENSOR_IOC('H', 0x0B, index, 0)

#define pr_std_info(fmt, args ...) \
	LOGD(fmt "\n", ##args)

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

enum hua_sensor_type
{
	HUA_SENSOR_TYPE_NONE,
	HUA_SENSOR_TYPE_ACCELEROMETER,
	HUA_SENSOR_TYPE_MAGNETIC_FIELD,
	HUA_SENSOR_TYPE_ORIENTATION,
	HUA_SENSOR_TYPE_GYROSCOPE,
	HUA_SENSOR_TYPE_LIGHT,
	HUA_SENSOR_TYPE_PRESSURE,
	HUA_SENSOR_TYPE_TEMPERATURE,
	HUA_SENSOR_TYPE_PROXIMITY,
	HUA_SENSOR_TYPE_GRAVITY,
	HUA_SENSOR_TYPE_LINEAR_ACCELERATION,
	HUA_SENSOR_TYPE_ROTATION_VECTOR
};

typedef enum
{
	false = 0,
	true
} bool;

struct hua_sensor_device
{
	char name[128];

	unsigned int type;
	unsigned int max_range;
	unsigned int min_delay;
	unsigned int power_consume;
	unsigned int resolution;

	int event_type;

	union
	{
		int event_code;

		struct
		{
			int xcode;
			int ycode;
			int zcode;
		};
	};

	bool updated;
	bool enabled;
	float scale;
	pthread_mutex_t lock;

	int index;
	struct hua_sensor_chip *chip;
	struct sensors_event_t event;
	struct hua_sensor_device *next;

	bool (*event_handler)(struct hua_sensor_device *dev, struct input_event *event);
};

struct hua_sensor_chip
{
	int data_fd;
	int ctrl_fd;
	char name[128];
	char vensor[128];

	size_t use_count;
	pthread_mutex_t lock;

	unsigned int min_delay;
	unsigned int sensor_count;
	struct pollfd *pfd;
	struct hua_sensor_device *sensor_list;

	struct hua_sensor_chip *prev;
	struct hua_sensor_chip *next;
};

struct hua_sensor_poll_device
{
	struct sensors_poll_device_t device;

	pthread_mutex_t lock;

	size_t chip_count;
	size_t sensor_count;
	struct sensor_t *sensor_list;
	struct hua_sensor_device **sensor_map;

	int pipefd[2];
	size_t pollfd_count;
	struct pollfd *pollfd_list;

	struct hua_sensor_chip *active_head;
	struct hua_sensor_chip *inactive_head;
};

int text_lhcmp(const char *text1, const char *text2);
char *text_copy(char *dest, const char *src);
char *text_ncopy(char *dest, const char *src, size_t size);
struct sensors_event_t *hua_sensor_device_sync_event(struct hua_sensor_device *sensor, size_t sensor_count, struct sensors_event_t *data, size_t data_size);
bool hua_sensor_device_report_event(struct hua_sensor_device *sensor, size_t count, struct input_event *event);
int hua_sensor_device_init(struct hua_sensor_device *sensor, struct hua_sensor_chip *chip, int index);
int hua_sensor_device_probe(struct hua_sensor_device *sensor, struct sensor_t *sensor_hal);

static inline struct sensors_event_t *hua_sensor_chip_sync_event(struct hua_sensor_chip *chip, struct sensors_event_t *data, size_t data_size)
{
	return hua_sensor_device_sync_event(chip->sensor_list, chip->sensor_count, data, data_size);
}

static inline bool hua_sensor_chip_report_event(struct hua_sensor_chip *chip, struct input_event *event)
{
	return hua_sensor_device_report_event(chip->sensor_list, chip->sensor_count, event);
}

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
