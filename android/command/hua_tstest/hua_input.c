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

int huamobile_input_parse_virtual_keymap(const char *devname, struct huamobile_virtual_key *keys, size_t count)
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

ssize_t huamobile_input_open_devices(struct huamobile_input_device *devs, size_t count, int (*matcher)(struct huamobile_input_device *dev, void *data), void *data)
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

	pdev = devs;
	pdev_end = pdev + count;

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

		if (matcher && matcher(pdev, data) < 0)
		{
			pr_red_info("Can't match device %s, name = %s", pathname, pdev->name);
			close(fd);
			continue;
		}

		pr_green_info("Add device %s, name = %s", pathname, pdev->name);

		pdev->fd = fd;
		ret = huamobile_input_parse_virtual_keymap(pdev->name, pdev->vkeys, NELEM(pdev->vkeys));
		pdev->vkey_count = ret < 0 ? 0 : ret;
		pdev++;
	}

	closedir(dp);

	return pdev - devs;
}

void huamobile_input_close_devices(struct huamobile_input_device *devs, size_t count)
{
	struct huamobile_input_device *pdev;

	pr_pos_info();

	for (pdev = devs + count - 1; pdev >= devs; pdev--)
	{
		close(pdev->fd);
	}
}

void *huamobile_input_thread_handler(void *data)
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
	pfd++;

	pdev = thread->input_devs;
	pdev_end = pdev + thread->dev_count;

	for (pfd_end = pfd + thread->dev_count; pdev < pdev_end; pdev++)
	{
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
		ret = poll(fds, NELEM(fds), 0);
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

	count = huamobile_input_open_devices(thread->input_devs, NELEM(thread->input_devs), thread->matcher, thread->private_data);
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

int huamobile_touch_screen_matcher_multi(struct huamobile_input_device *dev, void *data)
{
	int ret;
    uint8_t abs_bitmask[sizeof_bit_array(ABS_MAX + 1)];

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

int huamobile_touch_screen_matcher_single(struct huamobile_input_device *dev, void *data)
{
	int ret;
    uint8_t abs_bitmask[sizeof_bit_array(ABS_MAX + 1)];
    uint8_t key_bitmask[sizeof_bit_array(KEY_MAX + 1)];

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

int huamobile_touch_screen_matcher(struct huamobile_input_device *dev, void *data)
{
	int ret;
    uint8_t abs_bitmask[sizeof_bit_array(ABS_MAX + 1)];
    uint8_t key_bitmask[sizeof_bit_array(KEY_MAX + 1)];

	pr_pos_info();

    memset(abs_bitmask, 0, sizeof(abs_bitmask));
	ret = ioctl(dev->fd, EVIOCGBIT(EV_ABS, sizeof(abs_bitmask)), abs_bitmask);
	if (ret < 0)
	{
		pr_error_info("ioctl EVIOCGBIT EV_ABS");
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
    uint8_t abs_bitmask[sizeof_bit_array(ABS_MAX + 1)];

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

struct huamobile_virtual_key *huamobile_input_find_virtual_key(struct huamobile_virtual_key *key, struct huamobile_virtual_key *key_end, int x, int y)
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

int huamobile_ts_event_handler(struct huamobile_input_device *dev, struct input_event *event, void *data)
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
		}
		else
		{
			ts->key_handler(dev, event->code, event->value);
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
			if (ts->point_count == 0 && ts->pressed && p->x >= 0 && p->y >= 0)
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

						if (key->value != value)
						{
							ts->key_handler(dev, key->code, value);
							key->value = value;
						}
					}
					else if (p->pressure > 0)
					{
						ts->point_handler(dev, p, ts->private_data);
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
					if (key->value != 0)
					{
						ts->key_handler(dev, key->code, 0);
						key->value = 0;
					}
				}
			}
			break;
		}
		break;
	}

	return 0;
}

int huamobile_ts_start(struct huamobile_ts_device *ts, void *data)
{
	struct huamobile_touch_point *p, *p_end;
	struct huamobile_input_thread *thread;

	if (ts->point_handler == NULL || ts->key_handler == NULL)
	{
		pr_red_info("ts->point_handler == NULL || ts->key_handler == NULL");
		return -EINVAL;
	}

	ts->private_data = data;
	ts->pressed = 0;
	ts->point_count = 0;

	for (p = ts->points, p_end = p + NELEM(ts->points); p < p_end; p++)
	{
		p->id = p->x = p->y = p->pressure = -1;
	}

	thread = &ts->thread;
	thread->event_handler = huamobile_ts_event_handler;
	thread->matcher = ts->matcher;

	return huamobile_input_thread_start(thread, ts);
}
