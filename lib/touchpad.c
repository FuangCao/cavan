/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Thu Nov 22 00:10:41 CST 2012
 */

#include <cavan.h>
#include <cavan/touchpad.h>

bool cavan_touchpad_device_match(uint8_t *key_bitmask, uint8_t *abs_bitmask)
{
	if (test_bit(BTN_LEFT, key_bitmask) == 0 || \
		test_bit(BTN_RIGHT, key_bitmask) == 0 || \
		test_bit(BTN_TOUCH, key_bitmask) == 0 || \
		test_bit(ABS_X, abs_bitmask) == 0 || \
		test_bit(ABS_Y, abs_bitmask) == 0)
	{
		return false;
	}

	clean_bit(BTN_LEFT, key_bitmask);
	clean_bit(BTN_RIGHT, key_bitmask);
	clean_bit(BTN_MIDDLE, key_bitmask);
	clean_bit(BTN_TOUCH, key_bitmask);
	clean_bit(ABS_X, abs_bitmask);
	clean_bit(ABS_Y, abs_bitmask);

	return true;
}

bool cavan_touchpad_device_matcher(struct cavan_event_matcher *matcher, void *data)
{
	int ret;
	uint8_t key_bitmask[KEY_BITMASK_SIZE];
	uint8_t abs_bitmask[ABS_BITMASK_SIZE];

	ret = cavan_event_get_abs_bitmask(matcher->fd, abs_bitmask);
	if (ret < 0)
	{
		pr_error_info("cavan_event_get_abs_bitmask");
		return ret;
	}

	ret = cavan_event_get_key_bitmask(matcher->fd, key_bitmask);
	if (ret < 0)
	{
		pr_error_info("cavan_event_get_key_bitmask");
		return ret;
	}

	return cavan_touchpad_device_match(key_bitmask, abs_bitmask);
}

static bool cavan_touchpad_event_handler(struct cavan_input_device *dev, struct input_event *event, void *data)
{
	struct cavan_input_service *service = data;
	struct cavan_touchpad_device *touchpad = (struct cavan_touchpad_device *)dev;

	switch (event->type)
	{
	case EV_KEY:
		switch (event->code)
		{
		case BTN_TOUCH:
			touchpad->pressed = event->value;
			if (touchpad->pressed)
			{
				touchpad->time = timeval2msec(&event->time);
			}
			else if (timeval2msec(&event->time) - touchpad->time < 500)
			{
				service->mouse_touch_handler(dev, CAVAN_MOUSE_BUTTON_LEFT, 1, service->private_data);
				msleep(100);
				service->mouse_touch_handler(dev, CAVAN_MOUSE_BUTTON_LEFT, 0, service->private_data);
			}
			break;

		case BTN_LEFT:
			service->mouse_touch_handler(dev, CAVAN_MOUSE_BUTTON_LEFT, event->value, service->private_data);
			break;

		case BTN_RIGHT:
			service->mouse_touch_handler(dev, CAVAN_MOUSE_BUTTON_RIGHT, event->value, service->private_data);
			break;

		case BTN_0:
		case BTN_MIDDLE:
			service->mouse_touch_handler(dev, CAVAN_MOUSE_BUTTON_MIDDLE0, event->value, service->private_data);
			break;

		case BTN_1:
			service->mouse_touch_handler(dev, CAVAN_MOUSE_BUTTON_MIDDLE1, event->value, service->private_data);
			break;

		default:
			return false;
		}
		break;

	case EV_ABS:
		switch (event->code)
		{
		case ABS_X:
			touchpad->x = event->value;
			break;

		case ABS_Y:
			touchpad->y = event->value;
			break;

		case ABS_PRESSURE:
		case ABS_TOOL_WIDTH:
			break;

		default:
			return false;
		}
		break;

	case EV_SYN:
		if (touchpad->pressed)
		{
			if (touchpad->released)
			{
				touchpad->released = 0;
			}
			else
			{
				int x = (touchpad->x - touchpad->xold) * touchpad->xspeed;
				int y = (touchpad->y - touchpad->yold) * touchpad->yspeed;

				if (x || y)
				{
					service->mouse_move_handler(dev, x, y, service->private_data);
				}
			}

			touchpad->xold = touchpad->x;
			touchpad->yold = touchpad->y;
		}
		else
		{
			touchpad->released = 1;
		}
		break;

	default:
		return false;
	}

	return true;
}

static int cavan_touchpad_probe(struct cavan_input_device *dev, void *data)
{
	int ret;
	int min, max;
	int fd = dev->event_dev->fd;
	struct cavan_touchpad_device *touchpad = (struct cavan_touchpad_device *)dev;
	struct cavan_input_service *service = data;

	pr_bold_info("LCD: width = %d, height = %d", service->lcd_width, service->lcd_height);

	if (service->lcd_width > 0)
	{
		ret = cavan_event_get_absinfo(fd, ABS_X, &min, &max);
		if (ret < 0)
		{
			pr_red_info("cavan_event_get_absinfo");
			return ret;
		}

		pr_bold_info("x-min = %d, x-max = %d", min, max);
		touchpad->xspeed = ((double)service->lcd_width) / (max - min);
	}
	else
	{
		touchpad->xspeed = 1;
	}

	if (service->lcd_height > 0)
	{
		ret = cavan_event_get_absinfo(fd, ABS_Y, &min, &max);
		if (ret < 0)
		{
			pr_red_info("cavan_event_get_absinfo");
			return ret;
		}

		pr_bold_info("y-min = %d, y-max = %d", min, max);
		touchpad->yspeed = ((double)service->lcd_height) / (max - min);
	}
	else
	{
		touchpad->yspeed = 1;
	}

	pr_bold_info("xspeed = %lf, yspeed = %lf", touchpad->xspeed, touchpad->yspeed);

	return 0;
}

struct cavan_input_device *cavan_touchpad_device_create(void)
{
	struct cavan_touchpad_device *touchpad;
	struct cavan_input_device *dev;

	touchpad = malloc(sizeof(*touchpad));
	if (touchpad == NULL)
	{
		pr_error_info("malloc");
		return NULL;
	}

	touchpad->x = touchpad->xold = 0;
	touchpad->y = touchpad->yold = 0;
	touchpad->pressed = 0;
	touchpad->released = 1;

	dev = &touchpad->input_dev;
	dev->probe = cavan_touchpad_probe;
	dev->remove = NULL;
	dev->event_handler = cavan_touchpad_event_handler;

	return dev;
}
