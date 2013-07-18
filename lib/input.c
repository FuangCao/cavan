/*
 * File:		input.c
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

#include <cavan/input.h>
#include <cavan/mouse.h>
#include <cavan/keypad.h>
#include <cavan/gsensor.h>
#include <cavan/touchpad.h>
#include <cavan/touchscreen.h>

#define CAVAN_INPUT_SUPPORT_GSENSOR	0

static struct cavan_input_device *cavan_input_device_create(uint8_t *key_bitmask, uint8_t *abs_bitmask, uint8_t *rel_bitmask)
{
#if CAVAN_INPUT_SUPPORT_GSENSOR
	if (cavan_gsensor_device_match(abs_bitmask))
	{
		pr_green_info("G-Sensor Matched");
		return cavan_gsensor_create();
	}
#endif

	if (cavan_touchpad_device_match(key_bitmask, abs_bitmask))
	{
		pr_green_info("Touch Pad Matched");
		return cavan_touchpad_device_create();
	}

	if (cavan_multi_touch_device_match(abs_bitmask))
	{
		pr_green_info("Muti Touch Panel Matched");
		return cavan_multi_touch_device_create();
	}

	if (cavan_single_touch_device_match(abs_bitmask, key_bitmask))
	{
		pr_green_info("Single Touch Panel Matched");
		return cavan_single_touch_device_create();
	}

	if (cavan_mouse_device_match(key_bitmask, rel_bitmask))
	{
		pr_green_info("Mouse Matched");
		return cavan_mouse_create();
	}

	if (cavan_keypad_device_match(key_bitmask))
	{
		pr_green_info("Keypad Matched");
		return cavan_keypad_create();
	}

	return NULL;
}

static int cavan_input_device_probe(struct cavan_event_device *event_dev, void *data)
{
	int ret;
	int fd = event_dev->fd;
	uint8_t key_bitmask[KEY_BITMASK_SIZE];
	uint8_t abs_bitmask[ABS_BITMASK_SIZE];
	uint8_t rel_bitmask[REL_BITMASK_SIZE];
	struct cavan_input_device *dev, *head, *tail;

	ret = cavan_event_get_abs_bitmask(fd, abs_bitmask);
	if (ret < 0)
	{
		pr_error_info("cavan_event_get_abs_bitmask");
		return ret;
	}

	ret = cavan_event_get_key_bitmask(fd, key_bitmask);
	if (ret < 0)
	{
		pr_error_info("cavan_event_get_key_bitmask");
		return ret;
	}

	ret = cavan_event_get_rel_bitmask(fd, rel_bitmask);
	if (ret < 0)
	{
		pr_error_info("cavan_event_get_rel_bitmask");
		return ret;
	}

	head = tail = NULL;

	while (1)
	{
		dev = cavan_input_device_create(key_bitmask, abs_bitmask, rel_bitmask);
		if (dev == NULL)
		{
			break;
		}

		dev->event_dev = event_dev;

		if (dev->probe && dev->probe(dev, data) < 0)
		{
			free(dev);
			continue;
		}

		if (head)
		{
			tail->next = dev;
			tail = dev;
		}
		else
		{
			head = tail = dev;
		}
	}

	if (head == NULL)
	{
		pr_red_info("can't recognize device");
		return -EINVAL;
	}

	tail->next = NULL;
	event_dev->private_data = head;

	return 0;
}

static void cavan_input_device_remove(struct cavan_event_device *event_dev, void *data)
{
	struct cavan_input_device *dev = event_dev->private_data, *next;

	while (dev)
	{
		if (dev->remove)
		{
			dev->remove(dev, data);
		}

		next = dev->next;
		free(dev);
		dev = next;
	}
}

static bool cavan_input_device_event_handler(struct cavan_event_device *event_dev, struct input_event *event, void *data)
{
	struct cavan_input_device *dev = event_dev->private_data;

	switch(event->type)
	{
	case EV_SYN:
		while (dev)
		{
			dev->event_handler(dev, event, data);
			dev = dev->next;
		}
		return true;

	case EV_MSC:
		return true;

	default:
		while (dev)
		{
			if (dev->event_handler(dev, event, data))
			{
				return true;
			}

			dev = dev->next;
		}
	}

	return false;
}

static bool cavan_input_device_matcher(struct cavan_event_matcher *matcher, void *data)
{
	struct cavan_input_service *service = data;

	if (service->matcher)
	{
		return service->matcher(matcher, service->private_data);
	}

	return true;
}

void cavan_input_service_init(struct cavan_input_service *service, bool (*matcher)(struct cavan_event_matcher *, void *))
{
	cavan_event_service_init(&service->event_service, cavan_input_device_matcher);

	service->lcd_width = -1;
	service->lcd_height = -1;

	service->matcher = matcher;
	service->handler = NULL;
}

static void cavan_input_message_queue_handler(void *addr, void *data)
{
	struct cavan_input_service *service = data;

	service->handler(addr, service->private_data);
}

static void cavan_input_message_queue_handler_dummy(void *addr, void *data)
{
	struct cavan_input_message_key *key;
	struct cavan_input_message_point *point;
	struct cavan_input_message_vector *vector;
	cavan_input_message_t *message = addr;

	switch (message->type)
	{
	case CAVAN_INPUT_MESSAGE_KEY:
		key = &message->key;
		pr_std_info("key: name = %s, code = %d, value = %d", key->name, key->code, key->value);
		break;

	case CAVAN_INPUT_MESSAGE_MOVE:
		point = &message->point;
		pr_std_info("move[%d] = [%d, %d]", point->id, point->x, point->y);
		break;

	case CAVAN_INPUT_MESSAGE_TOUCH:
		point = &message->point;
		pr_std_info("touch[%d] = [%d, %d]", point->id, point->x, point->y);
		break;

	case CAVAN_INPUT_MESSAGE_WHEEL:
		key = &message->key;
		pr_std_info("wheel[%d] = %d", key->code, key->value);
		break;

	case CAVAN_INPUT_MESSAGE_MOUSE_MOVE:
		vector = &message->vector;
		pr_std_info("mouse_move = [%d, %d]", vector->x, vector->y);
		break;

	case CAVAN_INPUT_MESSAGE_MOUSE_TOUCH:
		key = &message->key;
		pr_std_info("mouse_touch[%d] = %d", key->code, key->value);
		break;

	case CAVAN_INPUT_MESSAGE_ACCELEROMETER:
		vector = &message->vector;
		pr_std_info("Accelerometer = [%d, %d, %d]", vector->x, vector->y, vector->z);
		break;

	case CAVAN_INPUT_MESSAGE_MAGNETIC_FIELD:
		vector = &message->vector;
		pr_std_info("Magnetic_Field = [%d, %d, %d]", vector->x, vector->y, vector->z);
		break;

	case CAVAN_INPUT_MESSAGE_ORIENTATION:
		vector = &message->vector;
		pr_std_info("Orientation = [%d, %d, %d]", vector->x, vector->y, vector->z);
		break;

	case CAVAN_INPUT_MESSAGE_GYROSCOPE:
		vector = &message->vector;
		pr_std_info("Gyroscope = [%d, %d, %d]", vector->x, vector->y, vector->z);
		break;

	case CAVAN_INPUT_MESSAGE_GRAVITY:
		vector = &message->vector;
		pr_std_info("Gravity = [%d, %d, %d]", vector->x, vector->y, vector->z);
		break;

	case CAVAN_INPUT_MESSAGE_LINEAR_ACCELERATION:
		vector = &message->vector;
		pr_std_info("Linear_Acceleration = [%d, %d, %d]", vector->x, vector->y, vector->z);
		break;

	case CAVAN_INPUT_MESSAGE_ROTATION_VECTOR:
		vector = &message->vector;
		pr_std_info("Rotation_Vector = [%d, %d, %d]", vector->x, vector->y, vector->z);
		break;

	case CAVAN_INPUT_MESSAGE_LIGHT:
		pr_std_info("Light = [%d]", message->value);
		break;

	case CAVAN_INPUT_MESSAGE_PRESSURE:
		pr_std_info("Pressure = [%d]", message->value);
		break;

	case CAVAN_INPUT_MESSAGE_TEMPERATURE:
		pr_std_info("Temperature = [%d]", message->value);
		break;

	case CAVAN_INPUT_MESSAGE_PROXIMITY:
		pr_std_info("Proximity = [%d]", message->value);
		break;

	default:
		pr_red_info("Invalid message type %d", message->type);
	}
}

int cavan_input_service_start(struct cavan_input_service *service, void *data)
{
	int ret;
	struct cavan_event_service *event_service;

	if (service == NULL)
	{
		pr_red_info("service == NULL");
		ERROR_RETURN(EINVAL);
	}

	pthread_mutex_init(&service->lock, NULL);
	service->private_data = data;

	if (service->handler)
	{
		service->queue.handler = cavan_input_message_queue_handler;
	}
	else
	{
		service->queue.handler = cavan_input_message_queue_handler_dummy;
	}

	ret = cavan_data_queue_run(&service->queue, MOFS(cavan_input_message_t, node), \
			sizeof(cavan_input_message_t), CAVAN_INPUT_MESSAGE_POOL_SIZE, service);
	if (ret < 0)
	{
		pr_red_info("cavan_data_queue_run");
		return ret;
	}

	event_service = &service->event_service;
	event_service->matcher = cavan_input_device_matcher;
	event_service->probe = cavan_input_device_probe;
	event_service->remove = cavan_input_device_remove;
	event_service->event_handler = cavan_input_device_event_handler;

	ret = cavan_timer_service_start(&service->timer_service);
	if (ret < 0)
	{
		pr_red_info("cavan_timer_service_start");
		goto out_cavan_data_queue_stop;
	}

	ret = cavan_event_service_start(event_service, service);
	if (ret < 0)
	{
		pr_red_info("cavan_event_service_start");
		goto out_timer_service_stop;
	}

	return 0;

out_timer_service_stop:
	cavan_timer_service_stop(&service->timer_service);
out_cavan_data_queue_stop:
	cavan_data_queue_stop(&service->queue);
	cavan_data_queue_deinit(&service->queue);
	return ret;
}

void cavan_input_service_stop(struct cavan_input_service *service)
{
	cavan_event_service_stop(&service->event_service);
	cavan_timer_service_stop(&service->timer_service);
	cavan_data_queue_stop(&service->queue);
	cavan_data_queue_deinit(&service->queue);
	pthread_mutex_destroy(&service->lock);
}

bool cavan_input_service_append_key_message(struct cavan_input_service *service, int type, const char *name, int code, int value)
{
	cavan_input_message_t *message;
	struct cavan_input_message_key *key;

	message = cavan_input_service_get_message(service, type);
	if (message == NULL)
	{
		return false;
	}

	key = &message->key;
	key->name = name;
	key->code = code;
	key->value = value;

	cavan_input_service_append_message(service, message);

	return true;
}

bool cavan_input_service_append_vector_message(struct cavan_input_service *service, int type, int x, int y, int z)
{
	cavan_input_message_t *message;
	struct cavan_input_message_vector *vector;

	message = cavan_input_service_get_message(service, type);
	if (message == NULL)
	{
		return false;
	}

	vector = &message->vector;
	vector->x = x;
	vector->y = y;
	vector->z = z;

	cavan_input_service_append_message(service, message);

	return true;
}

bool cavan_input_service_append_point_message(struct cavan_input_service *service, int type, struct cavan_input_message_point *point)
{
	cavan_input_message_t *message;

	message = cavan_input_service_get_message(service, type);
	if (message == NULL)
	{
		return false;
	}

	message->point = *point;
	cavan_input_service_append_message(service, message);

	return true;
}

char cavan_keycode2ascii(int code, bool shift_down)
{
	const char *ascii_map =
		"**1234567890-=*\t"
		"qwertyuiop[]**"
		"asdfghjkl;'`*\\"
		"zxcvbnm,./*** ";
	const char *ascii_map_shift =
		"**!@#$%^&*()_+*\t"
		"QWERTYUIOP{}**"
		"ASDFGHJKL:\"~*|"
		"ZXCVBNM<>?*** ";

	if (code < 0 || code > KEY_SPACE)
	{
		return '*';
	}

	return shift_down ? ascii_map_shift[code] : ascii_map[code];
}
