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
	CAVAN_INPUT_MESSAGE_MOUSE_MOVE,
	CAVAN_INPUT_MESSAGE_MOUSE_WHEEL,
	CAVAN_INPUT_MESSAGE_MOUSE_TOUCH,
} cavan_input_message_type_t;

struct cavan_input_message_key
{
	const char *name;
	int code;
	int value;
};

struct cavan_input_message_move
{
	int id;
	int x;
	int y;
};

struct cavan_input_message_touch
{
	int id;
	int x;
	int y;
	int pressure;
};

struct cavan_input_message_mouse_move
{
	int x;
	int y;
};

struct cavan_input_message_mouse_wheel
{
	int code;
	int value;
};

struct cavan_input_message_mouse_touch
{
	int code;
	int value;
};

typedef struct cavan_input_message
{
	cavan_input_message_type_t type;

	union
	{
		struct cavan_input_message_key key;
		struct cavan_input_message_move move;
		struct cavan_input_message_touch touch;
		struct cavan_input_message_mouse_move mouse_move;
		struct cavan_input_message_mouse_wheel mouse_wheel;
		struct cavan_input_message_mouse_touch mouse_touch;
	};

	void *data;
	struct double_link_node node;
} cavan_input_message_t;

struct cavan_input_message_pool
{
	cavan_input_message_t *buff;
	struct double_link link;
};

struct cavan_input_message_queue
{
	struct double_link link;
	struct cavan_thread thread;
	struct cavan_input_message_pool pool;

	void *private_data;
	void (*handler)(cavan_input_message_t *message, void *data);
};

struct cavan_gsensor_event
{
	int x, y, z;
};

typedef struct cavan_touch_point
{
	int id;
	int x;
	int y;
	int pressure;
	int released;
} cavan_touch_point_t;

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
	struct cavan_event_service event_service;
	struct cavan_timer_service timer_service;

	void *private_data;
	int lcd_width, lcd_height;
	pthread_mutex_t lock;

	bool (*matcher)(struct cavan_event_matcher *matcher, void *data);

	void (*key_handler)(struct cavan_input_device *dev, const char *name, int code, int value, void *data);
	void (*mouse_wheel_handler)(struct cavan_input_device *dev, int code, int value, void *data);
	void (*mouse_move_handler)(struct cavan_input_device *dev, int x, int y, void *data);
	void (*mouse_touch_handler)(struct cavan_input_device *dev, int code, int value, void *data);
	void (*touch_handler)(struct cavan_input_device *dev, cavan_touch_point_t *point, void *data);
	void (*move_handler)(struct cavan_input_device *dev, cavan_touch_point_t *point, void *data);
	void (*gsensor_handler)(struct cavan_input_device *dev, struct cavan_gsensor_event *event, void *data);
};

void cavan_input_service_init(struct cavan_input_service *service, bool (*matcher)(struct cavan_event_matcher *, void *));
int cavan_input_service_start(struct cavan_input_service *service, void *data);
int cavan_input_service_stop(struct cavan_input_service *service);

void cavan_input_message_pool_append(struct cavan_input_message_pool *pool, cavan_input_message_t *message);
cavan_input_message_t *cavan_input_message_pool_pop(struct cavan_input_message_pool *pool);
int cavan_input_message_pool_init(struct cavan_input_message_pool *pool, size_t size);
void cavan_input_message_pool_deinit(struct cavan_input_message_pool *pool);

int cavan_input_message_queue_start(struct cavan_input_message_queue *queue, size_t size, void *data);
void cavan_input_message_queue_stop(struct cavan_input_message_queue *queue);
cavan_input_message_t *cavan_input_message_create(struct cavan_input_message_queue *queue);

static inline int cavan_input_service_join(struct cavan_input_service *service)
{
	return cavan_event_service_join(&service->event_service);
}

static inline u32 timeval2msec(struct timeval *time)
{
	return time->tv_sec * 1000 + time->tv_usec / 1000;
}

static inline void cavan_input_message_queue_append(struct cavan_input_message_queue *queue, cavan_input_message_t *message)
{
	double_link_append(&queue->link, &message->node);
	cavan_thread_resume(&queue->thread);
}

static inline void cavan_input_message_queue_remove(struct cavan_input_message_queue *queue, cavan_input_message_t *message)
{
	double_link_remove(&queue->link, &message->node);
}
