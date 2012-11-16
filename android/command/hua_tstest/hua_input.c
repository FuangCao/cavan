/*
 * File:         hua_input.c
 * Based on:
 * Author:       Fuang Cao <cavan.cfa@gmail.com>
 *
 * Created:	  2012-11-14
 * Description:  HUAMOBILE INPUT DEVICE TEST
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

#include "hua_input.h"
#include <stdarg.h>

int huamobile_input_msleep(useconds_t ms)
{
	int ret;

	while (ms--)
	{
		ret = usleep(1000);
		if (ret < 0)
		{
			return ret;
		}
	}

	return 0;
}

int huamobile_input_ssleep(useconds_t ss)
{
	int ret;

	while (ss--)
	{
		ret = usleep(1000 * 1000);
		if (ret < 0)
		{
			return ret;
		}
	}

	return 0;
}

int huamobile_input_text_lhcmp(const char *text1, const char *text2)
{
	while (*text1)
	{
		if (*text1 != *text2)
		{
			return *text1 - *text2;
		}

		text1++;
		text2++;
	}

	return 0;
}

char *huamobile_input_text_copy(char *dest, const char *src)
{
	while ((*dest = *src))
	{
		dest++;
		src++;
	}

	return dest;
}

static int huamobile_input_parse_virtual_keymap(const char *devname, struct huamobile_virtual_key *keys, size_t count)
{
	int fd;
	ssize_t rdlen;
	char buff[2048], *p, *p_end;
	struct huamobile_virtual_key *key, *key_end;
	int x, y, width, height, code;

	sprintf(buff, "/sys/board_properties/virtualkeys.%s", devname);
	fd = open(buff, O_RDONLY);
	if (fd < 0)
	{
		pr_red_info("open file `%s'", buff);
		return fd;
	}

	rdlen = read(fd, buff, sizeof(buff));
	if (rdlen < 0)
	{
		pr_error_info("read");
		close(fd);
		return rdlen;
	}

	for (p = buff, p_end = p + rdlen, key = keys, key_end = key + count; key < key_end && p < p_end; p++)
	{
		if (huamobile_input_text_lhcmp("0x01", p) || sscanf(p, "0x01:%d:%d:%d:%d:%d", &code, &x, &y, &width, &height) != 5)
		{
			continue;
		}

		pr_bold_info("code = %d, x = %d, y = %d, width = %d, height = %d", code, x, y, width, height);

		width >>= 1;
		height >>= 1;

		key->left = x - width;
		key->right = x + width - 1;
		key->top = y - height;
		key->bottom = y + height - 1;
		key->code = code;
		key++;

		for (count = 0; p < p_end; p++)
		{
			if (*p == ':' && ++count > 5)
			{
				break;
			}
		}
	}

	close(fd);

	return key - keys;
}

static ssize_t huamobile_input_open_devices(struct huamobile_input_thread *thread)
{
	int fd;
	int ret;
	DIR *dp;
	struct dirent *entry;
	char pathname[PATH_MAX + 1], *filename;
	struct huamobile_input_device *pdev, *pdev_end;

	pr_pos_info();

	filename = huamobile_input_text_copy(pathname, "/dev/input/");
	dp = opendir(pathname);
	if (dp == NULL)
	{
		pr_error_info("open directory `%s'", pathname);
		return -ENOENT;
	}

	pdev = thread->input_devs;
	pdev_end = pdev + NELEM(thread->input_devs);

	while (pdev < pdev_end && (entry = readdir(dp)))
	{
		if (huamobile_input_text_lhcmp("event", entry->d_name))
		{
			continue;
		}

		huamobile_input_text_copy(filename, entry->d_name);
		fd = open(pathname, O_RDONLY);
		if (fd < 0)
		{
			pr_error_info("open file `%s'", pathname);
			continue;
		}

		ret = ioctl(fd, EVIOCGNAME(sizeof(pdev->name)), pdev->name);
		if (ret < 0)
		{
			pr_error_info("ioctl EVIOCGNAME");
			close(fd);
			continue;
		}

		pdev->fd = fd;

		if (thread->matcher && thread->matcher(pdev, thread->private_data) < 0)
		{
			pr_red_info("Can't match device %s, name = %s", pathname, pdev->name);
			close(fd);
			continue;
		}

		if (thread->init && thread->init(pdev, thread->private_data) < 0)
		{
			pr_red_info("Faile to Init device %s, name = %s", pathname, pdev->name);
			close(fd);
			continue;
		}

		pr_green_info("Add device %s, name = %s", pathname, pdev->name);

		ret = huamobile_input_parse_virtual_keymap(pdev->name, pdev->vkeys, NELEM(pdev->vkeys));
		pdev->vkey_count = ret < 0 ? 0 : ret;
		pdev++;
	}

	closedir(dp);

	return pdev - thread->input_devs;
}

static void huamobile_input_close_devices(struct huamobile_input_device *devs, size_t count)
{
	struct huamobile_input_device *pdev;

	pr_pos_info();

	for (pdev = devs + count - 1; pdev >= devs; pdev--)
	{
		close(pdev->fd);
	}
}

static void *huamobile_input_thread_handler(void *data)
{
	int ret;
	ssize_t rdlen;
	struct huamobile_input_thread *thread = data;
	struct pollfd fds[thread->dev_count + 1], *pfd, *pfd_end;
	struct huamobile_input_device *pdev, *pdev_end;
	struct input_event events[32], *ep, *ep_end;

	pr_pos_info();

	pfd = fds;
	pfd->fd = thread->pipefd[0];
	pfd->events = POLLIN;
	pfd->revents = 0;

	pdev = thread->input_devs;
	pdev_end = pdev + thread->dev_count;

	for (pfd_end = pfd + thread->dev_count; pdev < pdev_end; pdev++)
	{
		pfd++;
		pfd->fd = pdev->fd;
		pfd->events = POLLIN;
		pfd->revents = 0;
		pdev->pfd = pfd;
	}

	pthread_mutex_lock(&thread->lock);
	thread->state = HUA_INPUT_THREAD_STATE_RUNNING;
	pthread_mutex_unlock(&thread->lock);

	while (1)
	{
		ret = poll(fds, NELEM(fds), -1);
		if (ret < 0)
		{
			pr_error_info("poll");
			break;
		}

		if (fds[0].revents)
		{
			pr_green_info("Thread should stop");
			break;
		}

		for (pdev = thread->input_devs; pdev < pdev_end; pdev++)
		{
			if (pdev->pfd->revents == 0)
			{
				continue;
			}

			rdlen = read(pdev->fd, events, sizeof(events));
			if (rdlen < 0)
			{
				pr_error_info("read");
				goto out_thread_exit;
			}

			for (ep = events, ep_end = ep + (rdlen / sizeof(events[0])); ep < ep_end; ep++)
			{
				thread->event_handler(pdev, ep, thread->private_data);
			}
		}
	}

out_thread_exit:
	pthread_mutex_lock(&thread->lock);
	thread->state = HUA_INPUT_THREAD_STATE_STOPPED;
	pthread_mutex_unlock(&thread->lock);

	pr_red_info("Huamobile input thread exit");

	return NULL;
}

int huamobile_input_start_poll_thread(struct huamobile_input_thread *thread)
{
	int ret;

	pr_pos_info();

	pthread_mutex_lock(&thread->lock);

	if (thread->state == HUA_INPUT_THREAD_STATE_RUNNING)
	{
		pthread_mutex_unlock(&thread->lock);
		return 0;
	}

	ret = pipe(thread->pipefd);
	if (ret < 0)
	{
		pr_error_info("pipe");
		pthread_mutex_unlock(&thread->lock);
		return ret;
	}

	ret = pthread_create(&thread->thread, NULL, huamobile_input_thread_handler, thread);
	if (ret < 0)
	{
		pr_red_info("pthread_create");
		close(thread->pipefd[0]);
		close(thread->pipefd[1]);
		pthread_mutex_unlock(&thread->lock);
		return ret;
	}

	pthread_mutex_unlock(&thread->lock);

	return 0;
}

int huamobile_input_stop_poll_thread(struct huamobile_input_thread *thread)
{
	pr_pos_info();

	pthread_mutex_lock(&thread->lock);

	while (thread->state == HUA_INPUT_THREAD_STATE_RUNNING)
	{
		int ret;

		ret = huamobile_input_send_command(thread, HUA_INPUT_COMMAND_STOP);
		if (ret < 0)
		{
			pr_error_info("huamobile_input_send_command");
			pthread_mutex_unlock(&thread->lock);
			return ret;
		}

		pthread_mutex_unlock(&thread->lock);
		huamobile_input_msleep(100);
		pthread_mutex_unlock(&thread->lock);
	}

	close(thread->pipefd[0]);
	close(thread->pipefd[1]);

	pthread_mutex_unlock(&thread->lock);

	return 0;
}

int huamobile_input_thread_start(struct huamobile_input_thread *thread, void *data)
{
	ssize_t count;
	int ret;

	pr_pos_info();

	if (thread->event_handler == NULL)
	{
		pr_red_info("thread->event_handler == NULL");
		return -EINVAL;
	}

	thread->private_data = data;

	count = huamobile_input_open_devices(thread);
	if (count < 0)
	{
		pr_red_info("huamobile_input_open_devices");
		return count;
	}

	if (count == 0)
	{
		pr_red_info("No input devices found");
		return -ENOENT;
	}

	thread->dev_count = count;
	thread->state = HUA_INPUT_THREAD_STATE_STOPPED;
	pthread_mutex_init(&thread->lock, NULL);

	ret = huamobile_input_start_poll_thread(thread);
	if (ret < 0)
	{
		pr_red_info("huamobile_input_start_poll_thread");
		huamobile_input_close_devices(thread->input_devs, thread->dev_count);
	}

	return 0;
}

int huamobile_input_thread_stop(struct huamobile_input_thread *thread)
{
	int ret;

	pr_pos_info();

	ret = huamobile_input_stop_poll_thread(thread);
	if (ret < 0)
	{
		pr_red_info("huamobile_input_stop_poll_thread");
		return ret;
	}

	huamobile_input_close_devices(thread->input_devs, thread->dev_count);
	pthread_mutex_destroy(&thread->lock);

	return 0;
}

int huamobile_touch_screen_matcher_multi(struct huamobile_ts_device *ts, struct huamobile_input_device *dev, void *data)
{
	int ret;
    uint8_t abs_bitmask[sizeof_bit_array(ABS_CNT)];

	pr_pos_info();

    memset(abs_bitmask, 0, sizeof(abs_bitmask));

	ret = ioctl(dev->fd, EVIOCGBIT(EV_ABS, sizeof(abs_bitmask)), abs_bitmask);
	if (ret < 0)
	{
		pr_red_info("ioctl EVIOCGBIT EV_ABS");
		return ret;
	}

	if (test_bit(ABS_MT_POSITION_X, abs_bitmask) && test_bit(ABS_MT_POSITION_Y, abs_bitmask))
	{
		return 0;
	}

	return -EINVAL;
}

int huamobile_touch_screen_matcher_single(struct huamobile_ts_device *ts, struct huamobile_input_device *dev, void *data)
{
	int ret;
    uint8_t abs_bitmask[sizeof_bit_array(ABS_CNT)];
    uint8_t key_bitmask[sizeof_bit_array(KEY_CNT)];

	pr_pos_info();

    memset(abs_bitmask, 0, sizeof(abs_bitmask));
	ret = ioctl(dev->fd, EVIOCGBIT(EV_ABS, sizeof(abs_bitmask)), abs_bitmask);
	if (ret < 0)
	{
		pr_error_info("ioctl EVIOCGBIT EV_ABS");
		return ret;
	}

	if (test_bit(ABS_X, abs_bitmask) == 0 || test_bit(ABS_Y, abs_bitmask) == 0 || test_bit(ABS_Z, abs_bitmask))
	{
		return 0;
	}

    memset(key_bitmask, 0, sizeof(key_bitmask));
	ret = ioctl(dev->fd, EVIOCGBIT(EV_KEY, sizeof(key_bitmask)), key_bitmask);
	if (ret < 0)
	{
		pr_error_info("ioctl EVIOCGBIT EV_KEY");
		return ret;
	}

	if (test_bit(BTN_TOUCH, key_bitmask))
	{
		return 0;
	}

	return -EINVAL;
}

int huamobile_touch_screen_matcher(struct huamobile_ts_device *ts, struct huamobile_input_device *dev, void *data)
{
	int ret;
    uint8_t abs_bitmask[sizeof_bit_array(ABS_CNT)];
    uint8_t key_bitmask[sizeof_bit_array(KEY_CNT)];

	pr_pos_info();

    memset(abs_bitmask, 0, sizeof(abs_bitmask));
	ret = ioctl(dev->fd, EVIOCGBIT(EV_ABS, sizeof(abs_bitmask)), abs_bitmask);
	if (ret < 0)
	{
		pr_error_info("ioctl EVIOCGBIT EV_ABS, sizeof(abs_bitmask) = %d", sizeof(abs_bitmask));
		return ret;
	}

	if (test_bit(ABS_MT_POSITION_X, abs_bitmask) && test_bit(ABS_MT_POSITION_Y, abs_bitmask))
	{
		pr_green_info("Deivce %s mutil touch screen", dev->name);
		return 0;
	}

    memset(key_bitmask, 0, sizeof(key_bitmask));
	ret = ioctl(dev->fd, EVIOCGBIT(EV_KEY, sizeof(key_bitmask)), key_bitmask);
	if (ret < 0)
	{
		pr_error_info("ioctl EVIOCGBIT EV_KEY");
		return ret;
	}

	if (test_bit(BTN_TOUCH, key_bitmask) && test_bit(ABS_X, abs_bitmask) && test_bit(ABS_Y, abs_bitmask) && test_bit(ABS_Z, abs_bitmask) == 0)
	{
		pr_green_info("Deivce %s single touch screen", dev->name);
		return 0;
	}

	return -EINVAL;
}

int huamobile_gsensor_matcher(struct huamobile_input_device *dev, void *data)
{
	int ret;
    uint8_t abs_bitmask[sizeof_bit_array(ABS_CNT)];

	pr_pos_info();

    memset(abs_bitmask, 0, sizeof(abs_bitmask));
	ret = ioctl(dev->fd, EVIOCGBIT(EV_ABS, sizeof(abs_bitmask)), abs_bitmask);
	if (ret < 0)
	{
		pr_error_info("ioctl EVIOCGBIT EV_ABS");
		return ret;
	}

	if (test_bit(ABS_X, abs_bitmask) && test_bit(ABS_Y, abs_bitmask) && test_bit(ABS_Y, abs_bitmask))
	{
		return 0;
	}

	return -EINVAL;
}

int huamobile_input_name_matcher(const char *devname, ...)
{
	va_list ap;
	const char *name;

	va_start(ap, devname);

	while (1)
	{
		name = va_arg(ap, const char *);
		if (name == NULL)
		{
			break;
		}

		if (strcmp(devname, name) == 0)
		{
			break;
		}
	}

	va_end(ap);

	return name ? 0 : -EINVAL;
}

static int huamobile_input_get_absinfo_base(int fd, int axis, int *min, int *max)
{
	int ret;
	struct input_absinfo info;

	ret = ioctl(fd, EVIOCGABS(axis), &info);
	if (ret < 0)
	{
		pr_error_info("ioctl EVIOCGABS");
		return ret;
	}

	*min = info.minimum;
	*max = info.maximum;

	return 0;
}

static int huamobile_ts_init(struct huamobile_input_device *dev, void *data)
{
	int ret;
	int fd = dev->fd;
	int min, max, diff;
    uint8_t abs_bitmask[sizeof_bit_array(ABS_CNT)];
	struct huamobile_ts_device *ts = data;

	pr_bold_info("lcd_width = %d, lcd_height = %d", ts->lcd_width, ts->lcd_height);

    memset(abs_bitmask, 0, sizeof(abs_bitmask));
	ret = ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(abs_bitmask)), abs_bitmask);
	if (ret < 0)
	{
		pr_error_info("ioctl EVIOCGBIT EV_ABS");
		return ret;
	}

	if (ts->lcd_width > 0 && test_bit(ABS_MT_POSITION_X, abs_bitmask))
	{
		ret = huamobile_input_get_absinfo_base(fd, ABS_MT_POSITION_X, &min, &max);
	}
	else if (ts->lcd_width > 0 && test_bit(ABS_X, abs_bitmask))
	{
		ret = huamobile_input_get_absinfo_base(fd, ABS_X, &min, &max);
	}
	else
	{
		ret = -1;
	}

	if (ret < 0)
	{
		ts->xscale = 1;
		ts->xoffset = 0;
	}
	else
	{
		pr_bold_info("x-min = %d, x-max = %d", min, max);

		diff = max - min;
		ts->xscale = ((double)ts->lcd_width) / diff;
		ts->xoffset = ((double)ts->lcd_width) * min / diff;
	}

	if (ts->lcd_height > 0 && test_bit(ABS_MT_POSITION_Y, abs_bitmask))
	{
		ret = huamobile_input_get_absinfo_base(fd, ABS_MT_POSITION_Y, &min, &max);
	}
	else if (ts->lcd_height > 0 && test_bit(ABS_Y, abs_bitmask))
	{
		ret = huamobile_input_get_absinfo_base(fd, ABS_Y, &min, &max);
	}
	else
	{
		ret = -1;
	}

	if (ret < 0)
	{
		ts->yscale = 1;
		ts->yoffset = 0;
	}
	else
	{
		pr_bold_info("y-min = %d, y-max = %d", min, max);
		diff = max - min;
		ts->yscale = ((double)ts->lcd_height) / diff;
		ts->yoffset = ((double)ts->lcd_height) * min / diff;
	}

	pr_bold_info("xscale = %lf, xoffset = %lf", ts->xscale, ts->xoffset);
	pr_bold_info("yscale = %lf, yoffset = %lf", ts->yscale, ts->yoffset);

	return ts->init ? ts->init(ts, dev, ts->private_data) : 0;
}

static struct huamobile_virtual_key *huamobile_input_find_virtual_key(struct huamobile_virtual_key *key, struct huamobile_virtual_key *key_end, int x, int y)
{
	while (key < key_end)
	{
		if (y >= key->top && y <= key->bottom && x >= key->left && x <= key->right)
		{
			return key;
		}

		key++;
	}

	return NULL;
}

static int huamobile_ts_event_handler(struct huamobile_input_device *dev, struct input_event *event, void *data)
{
	struct huamobile_ts_device *ts = data;
	struct huamobile_touch_point *p, *p_end;

	switch (event->type)
	{
	case EV_ABS:
		switch (event->code)
		{
		case ABS_X:
		case ABS_MT_POSITION_X:
			ts->points[ts->point_count].x = event->value;
			break;

		case ABS_Y:
		case ABS_MT_POSITION_Y:
			ts->points[ts->point_count].y = event->value;
			break;

		case ABS_PRESSURE:
		case ABS_MT_TOUCH_MAJOR:
			ts->points[ts->point_count].pressure = event->value;
			break;

		case ABS_MT_TRACKING_ID:
			ts->points[ts->point_count].id = event->value;
			break;
		}
		break;

	case EV_KEY:
		if (event->code == BTN_TOUCH)
		{
			ts->pressed = event->value;
			if (ts->pressed == 0 && ts->released == 0)
			{
				if (ts->point_handler)
				{
					ts->point_handler(ts, dev, NULL, ts->private_data);
				}

				ts->released = 1;
			}
		}
		else if (ts->key_handler)
		{
			ts->key_handler(ts, dev, event->code, event->value, ts->private_data);
		}
		break;

	case EV_SYN:
		switch (event->code)
		{
		case SYN_MT_REPORT:
			p = ts->points + ts->point_count;
			if (p->id < 0)
			{
				p->id = ts->point_count;
			}

			if (p->x >= 0 && p->y >= 0 && p->pressure >= 0)
			{
				ts->point_count++;
			}
			break;

		case SYN_REPORT:
			p = ts->points + ts->point_count;
			if (ts->point_count == 0 && ts->pressed > 0 && p->x >= 0 && p->y >= 0)
			{
				p->pressure = 1;
				ts->point_count++;
			}

			if (ts->point_count)
			{
				struct huamobile_virtual_key *key, *key_end;

				key_end = dev->vkeys + dev->vkey_count;

				for (p = ts->points, p_end = p + ts->point_count; p < p_end; p++)
				{
					key = huamobile_input_find_virtual_key(dev->vkeys, key_end, p->x, p->y);
					if (key)
					{
						int value = p->pressure > 0;

						if (key->value != value && ts->key_handler)
						{
							ts->key_handler(ts, dev, key->code, value, ts->private_data);
						}

						key->value = value;
					}
					else if (p->pressure > 0)
					{
						if (ts->point_handler)
						{
							p->x = p->x * ts->xscale - ts->xoffset;
							p->y = p->y * ts->yscale - ts->yoffset;

							ts->point_handler(ts, dev, p, ts->private_data);
						}

						ts->released = 0;
					}

					p->id = p->x = p->y = p->pressure = -1;
				}

				ts->point_count = 0;
			}
			else
			{
				struct huamobile_virtual_key *key, *key_end;

				for (key = dev->vkeys, key_end = key + dev->vkey_count; key < key_end; key++)
				{
					if (key->value != 0 && ts->key_handler)
					{
						ts->key_handler(ts, dev, key->code, 0, ts->private_data);
					}

					key->value = 0;
				}

				if (ts->released == 0 && ts->point_handler)
				{
					ts->point_handler(ts, dev, NULL, ts->private_data);
				}

				ts->released = 1;
			}
			break;
		}
		break;
	}

	return 0;
}

static int huamobile_ts_matcher(struct huamobile_input_device *dev, void *data)
{
	struct huamobile_ts_device *ts = data;

	return ts->matcher ? ts->matcher(ts, dev, ts->private_data) : 0;
}

int huamobile_ts_start(struct huamobile_ts_device *ts, void *data)
{
	struct huamobile_touch_point *p, *p_end;
	struct huamobile_input_thread *thread;

	if (ts->point_handler == NULL && ts->key_handler == NULL)
	{
		pr_red_info("ts->point_handler == NULL && ts->key_handler == NULL");
		return -EINVAL;
	}

	ts->private_data = data;
	ts->pressed = 0;
	ts->released = 1;
	ts->point_count = 0;

	for (p = ts->points, p_end = p + NELEM(ts->points); p < p_end; p++)
	{
		p->id = p->x = p->y = p->pressure = -1;
	}

	thread = &ts->thread;
	thread->event_handler = huamobile_ts_event_handler;
	thread->matcher = huamobile_ts_matcher;
	thread->init = huamobile_ts_init;

	return huamobile_input_thread_start(thread, ts);
}
