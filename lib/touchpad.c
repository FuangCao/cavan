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

static void cavan_touchpad_touch(struct cavan_touchpad_device *touchpad, struct cavan_input_service *service, int value)
{
	if (value > 0)
	{
		if (touchpad->x > touchpad->right && touchpad->y > touchpad->bottom)
		{
			touchpad->btn_code = BTN_RIGHT;
		}
		else
		{
			touchpad->btn_code = BTN_LEFT;
		}
	}

	service->mouse_touch_handler(&touchpad->input_dev, touchpad->btn_code, value, service->private_data);
}

static void cavan_touchpad_timer_handler_down(struct cavan_timer *timer)
{
	struct cavan_touchpad_device *touchpad = MEMBER_TO_STRUCT(timer, struct cavan_touchpad_device, timer);

	if (touchpad->x > touchpad->right)
	{
		touchpad->mode = CAVAN_TOUCHPAD_MODE_WHEEL;
	}
	else if (touchpad->y > touchpad->bottom)
	{
		touchpad->mode = CAVAN_TOUCHPAD_MODE_HWHEEL;
	}
	else
	{
		touchpad->mode = CAVAN_TOUCHPAD_MODE_MOVE;
	}

	touchpad->xold = touchpad->x;
	touchpad->yold = touchpad->y;
}

static void cavan_touchpad_timer_handler_up(struct cavan_timer *timer)
{
	struct cavan_touchpad_device *touchpad = MEMBER_TO_STRUCT(timer, struct cavan_touchpad_device, timer);
	struct cavan_input_service *service = timer->private_data;

	touchpad->state = CAVAN_TOUCHPAD_STATE_IDEL;
	cavan_touchpad_touch(touchpad, service, 0);

	touchpad->mode = CAVAN_TOUCHPAD_MODE_NONE;
}

static bool cavan_touchpad_event_handler(struct cavan_input_device *dev, struct input_event *event, void *data)
{
	int xdiff, ydiff;
	struct cavan_input_service *service = data;
	struct cavan_touchpad_device *touchpad = (struct cavan_touchpad_device *)dev;

	switch (event->type)
	{
	case EV_KEY:
		switch (event->code)
		{
		case BTN_TOUCH:
			switch (touchpad->state)
			{
			case CAVAN_TOUCHPAD_STATE_DOWN1:
				if (touchpad->mode == CAVAN_TOUCHPAD_MODE_NONE)
				{
					touchpad->state = CAVAN_TOUCHPAD_STATE_UP1;
					cavan_timer_remove_node(&service->timer_service, &touchpad->timer);
					cavan_touchpad_touch(touchpad, service, 1);

					touchpad->timer.private_data = service;
					touchpad->timer.handler = cavan_touchpad_timer_handler_up;
					cavan_timer_add_node(&service->timer_service, &touchpad->timer, 200);
				}
				else
				{
					touchpad->state = CAVAN_TOUCHPAD_STATE_IDEL;
					touchpad->mode = CAVAN_TOUCHPAD_MODE_NONE;
				}
				break;

			case CAVAN_TOUCHPAD_STATE_UP1:
				touchpad->state = CAVAN_TOUCHPAD_STATE_DOWN2;
				cavan_timer_remove_node(&service->timer_service, &touchpad->timer);

				touchpad->timer.private_data = service;
				touchpad->timer.handler = cavan_touchpad_timer_handler_down;
				cavan_timer_add_node(&service->timer_service, &touchpad->timer, 200);
				break;

			case CAVAN_TOUCHPAD_STATE_DOWN2:
				cavan_touchpad_touch(touchpad, service, 0);
				if (touchpad->mode == CAVAN_TOUCHPAD_MODE_NONE)
				{
					touchpad->state = CAVAN_TOUCHPAD_STATE_UP2;
					cavan_touchpad_touch(touchpad, service, 1);

					touchpad->timer.private_data = service;
					touchpad->timer.handler = cavan_touchpad_timer_handler_up;
					cavan_timer_add_node(&service->timer_service, &touchpad->timer, 200);
				}
				else
				{
					touchpad->state = CAVAN_TOUCHPAD_STATE_IDEL;
					touchpad->mode = CAVAN_TOUCHPAD_MODE_NONE;
				}
				break;

			case CAVAN_TOUCHPAD_STATE_UP2:
				cavan_timer_remove_node(&service->timer_service, &touchpad->timer);
				cavan_touchpad_touch(touchpad, service, 0);
			case CAVAN_TOUCHPAD_STATE_IDEL:
				if (event->value)
				{
					touchpad->state = CAVAN_TOUCHPAD_STATE_DOWN1;

					touchpad->timer.private_data = service;
					touchpad->timer.handler = cavan_touchpad_timer_handler_down;
					cavan_timer_add_node(&service->timer_service, &touchpad->timer, 100);
				}
				else
				{
					touchpad->state = CAVAN_TOUCHPAD_STATE_IDEL;
					touchpad->mode = CAVAN_TOUCHPAD_MODE_NONE;
				}
				break;
			}
			break;

		case BTN_LEFT:
		case BTN_RIGHT:
		case BTN_MIDDLE:
		case BTN_0:
		case BTN_1:
			service->mouse_touch_handler(dev, event->code, event->value, service->private_data);
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
		switch (touchpad->mode)
		{
		case CAVAN_TOUCHPAD_MODE_WHEEL:
			ydiff = (touchpad->yold - touchpad->y) * touchpad->yspeed;
			if (ydiff)
			{
				service->mouse_wheel_handler(dev, REL_HWHEEL, ydiff > 0 ? 1 : -1, service->private_data);
				touchpad->yold = touchpad->y;
			}
			break;

		case CAVAN_TOUCHPAD_MODE_HWHEEL:
			xdiff = (touchpad->xold - touchpad->x) * touchpad->xspeed;
			if (xdiff)
			{
				service->mouse_wheel_handler(dev, REL_WHEEL, xdiff > 0 ? 1 : -1, service->private_data);
				touchpad->xold = touchpad->x;
			}
			break;

		case CAVAN_TOUCHPAD_MODE_MOVE:
			xdiff = (touchpad->x - touchpad->xold) * touchpad->xspeed;
			ydiff = (touchpad->y - touchpad->yold) * touchpad->yspeed;

			if (xdiff || ydiff)
			{
				service->mouse_move_handler(dev, xdiff, ydiff, service->private_data);

				touchpad->xold = touchpad->x;
				touchpad->yold = touchpad->y;
			}
			break;

		case CAVAN_TOUCHPAD_MODE_NONE:
			break;
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
	int min, max, diff;
	int width, height;
	int fd = dev->event_dev->fd;
	struct cavan_touchpad_device *touchpad = (struct cavan_touchpad_device *)dev;
	struct cavan_input_service *service = data;

	pr_bold_info("LCD: width = %d, height = %d", service->lcd_width, service->lcd_height);

	ret = cavan_event_get_absinfo(fd, ABS_X, &min, &max);
	if (ret < 0)
	{
		pr_red_info("cavan_event_get_absinfo");
		return ret;
	}

	diff = max - min;
	width = service->lcd_height > 0 ? service->lcd_height : diff;
	pr_bold_info("x-min = %d, x-max = %d, diff = %d, width = %d", min, max, diff, width);

	touchpad->xspeed = ((double)width) / diff;
	touchpad->right = max - (diff >> CAVAN_TOUCHPAD_EDGE_SHIFT);
	pr_bold_info("xspeed = %lf, right = %d", touchpad->xspeed, touchpad->right);

	ret = cavan_event_get_absinfo(fd, ABS_Y, &min, &max);
	if (ret < 0)
	{
		pr_red_info("cavan_event_get_absinfo");
		return ret;
	}

	diff = max - min;
	height = service->lcd_height > 0 ? service->lcd_height : diff;
	pr_bold_info("y-min = %d, y-max = %d, diff = %d, height = %d", min, max, diff, height);

	touchpad->yspeed = ((double)height) / diff;
	touchpad->bottom = max - (diff >> CAVAN_TOUCHPAD_EDGE_SHIFT);
	pr_bold_info("yspeed = %lf, bottom = %d", touchpad->yspeed, touchpad->bottom);

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
	touchpad->state = CAVAN_TOUCHPAD_STATE_IDEL;
	touchpad->mode = CAVAN_TOUCHPAD_MODE_NONE;

	dev = &touchpad->input_dev;
	dev->probe = cavan_touchpad_probe;
	dev->remove = NULL;
	dev->event_handler = cavan_touchpad_event_handler;

	return dev;
}
