/*
 * File:		input.h
 * Author:		Fuang Cao <cavan.cfa@gmail.com>
 * Created:		2012-11-14
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

#include <cavan.h>
#include <cavan/event.h>
#include <cavan/timer.h>
#include <cavan/queue.h>
#include <linux/input.h>

#define CAVAN_INPUT_MESSAGE_POOL_SIZE	20

#ifndef SYN_MT_REPORT
#define SYN_MT_REPORT		2
#define ABS_MT_SLOT			0x2f	/* MT slot being modified */
#define ABS_MT_TOUCH_MAJOR	0x30	/* Major axis of touching ellipse */
#define ABS_MT_TOUCH_MINOR	0x31	/* Minor axis (omit if circular) */
#define ABS_MT_WIDTH_MAJOR	0x32	/* Major axis of approaching ellipse */
#define ABS_MT_WIDTH_MINOR	0x33	/* Minor axis (omit if circular) */
#define ABS_MT_ORIENTATION	0x34	/* Ellipse orientation */
#define ABS_MT_POSITION_X	0x35	/* Center X ellipse position */
#define ABS_MT_POSITION_Y	0x36	/* Center Y ellipse position */
#define ABS_MT_TOOL_TYPE	0x37	/* Type of touching device */
#define ABS_MT_BLOB_ID		0x38	/* Group a set of packets as a blob */
#define ABS_MT_TRACKING_ID	0x39	/* Unique ID of initiated contact */
#define ABS_MT_PRESSURE		0x3a	/* Pressure on contact area */
#define ABS_MT_DISTANCE		0x3b	/* Contact hover distance */
#endif

typedef enum cavan_input_message_type
{
	CAVAN_INPUT_MESSAGE_KEY,
	CAVAN_INPUT_MESSAGE_MOVE,
	CAVAN_INPUT_MESSAGE_TOUCH,
	CAVAN_INPUT_MESSAGE_WHEEL,
	CAVAN_INPUT_MESSAGE_MOUSE_MOVE,
	CAVAN_INPUT_MESSAGE_MOUSE_TOUCH,
	CAVAN_INPUT_MESSAGE_ACCELEROMETER,
	CAVAN_INPUT_MESSAGE_MAGNETIC_FIELD,
	CAVAN_INPUT_MESSAGE_ORIENTATION,
	CAVAN_INPUT_MESSAGE_GYROSCOPE,
	CAVAN_INPUT_MESSAGE_LIGHT,
	CAVAN_INPUT_MESSAGE_PRESSURE,
	CAVAN_INPUT_MESSAGE_TEMPERATURE,
	CAVAN_INPUT_MESSAGE_PROXIMITY,
	CAVAN_INPUT_MESSAGE_GRAVITY,
	CAVAN_INPUT_MESSAGE_LINEAR_ACCELERATION,
	CAVAN_INPUT_MESSAGE_ROTATION_VECTOR
} cavan_input_message_type_t;

struct cavan_input_message_key
{
	const char *name;
	int code;
	int value;
};

struct cavan_input_message_point
{
	int id;
	int x;
	int y;
	int pressure;
	int released;
};

struct cavan_input_message_vector
{
	int x;
	int y;
	int z;
};

typedef struct cavan_input_message
{
	cavan_input_message_type_t type;

	union
	{
		int value;
		struct cavan_input_message_key key;
		struct cavan_input_message_point point;
		struct cavan_input_message_vector vector;
	};

	struct cavan_data_pool_node node;
} cavan_input_message_t;

struct cavan_input_message_queue
{
	void *private_data;
	struct cavan_data_queue queue;

	void (*handler)(struct cavan_input_message_queue *queue, cavan_input_message_t *message, void *data);
};

struct cavan_input_device
{
	struct cavan_event_device *event_dev;
	struct cavan_input_device *next;

	int (*probe)(struct cavan_input_device *dev, void *data);
	void (*remove)(struct cavan_input_device *dev, void *data);
	bool (*event_handler)(struct cavan_input_device *dev, struct input_event *event, void *data);
};

struct cavan_input_service
{
	struct cavan_data_queue queue;
	struct cavan_event_service event_service;
	struct cavan_timer_service timer_service;

	void *private_data;
	int lcd_width, lcd_height;
	pthread_mutex_t lock;

	bool (*matcher)(struct cavan_event_matcher *matcher, void *data);
	void (*handler)(cavan_input_message_t *message, void *data);
};

void cavan_input_service_init(struct cavan_input_service *service, bool (*matcher)(struct cavan_event_matcher *, void *));
int cavan_input_service_start(struct cavan_input_service *service, void *data);
void cavan_input_service_stop(struct cavan_input_service *service);
bool cavan_input_service_append_key_message(struct cavan_input_service *service, int type, const char *name, int code, int value);
bool cavan_input_service_append_vector_message(struct cavan_input_service *service, int type, int x, int y, int z);
bool cavan_input_service_append_point_message(struct cavan_input_service *service, int type, struct cavan_input_message_point *point);

static inline int cavan_input_service_join(struct cavan_input_service *service)
{
	return cavan_event_service_join(&service->event_service);
}

static inline u32 timeval2msec(struct timeval *time)
{
	return time->tv_sec * 1000 + time->tv_usec / 1000;
}

static inline cavan_input_message_t *cavan_input_service_get_message(struct cavan_input_service *service, int type)
{
	cavan_input_message_t *message;

	message = cavan_data_queue_get_node(&service->queue);
	if (message)
	{
		message->type = type;
	}

	return message;
}

static inline void cavan_input_service_append_message(struct cavan_input_service *service, cavan_input_message_t *message)
{
	cavan_data_queue_append(&service->queue, &message->node);
}
