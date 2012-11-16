/*
 * File:         input.c
 * Based on:
 * Author:       Fuang Cao <cavan.cfa@gmail.com>
 *
 * Created:	  2012-11-14
 * Description:  HUAMOBILE LIBRARY
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

#include <huamobile.h>

static struct huamobile_virtual_key *huamobile_input_find_virtual_key(struct huamobile_virtual_key *head, int x, int y)
{
	struct huamobile_virtual_key *key;

	for (key = head; key; key = key->next)
	{
		if (y >= key->top && y <= key->bottom && x >= key->left && x <= key->right)
		{
			return key;
		}
	}

	return NULL;
}

const char *huamobile_input_find_key_name(struct huamobile_keylayout_node *head, int code)
{
	struct huamobile_keylayout_node *node;

	for (node = head; node; node = node->next)
	{
		if (node->code == code)
		{
			return node->name;
		}
	}

	return NULL;
}

static int huamobile_input_parse_virtual_keymap(struct huamobile_input_device *dev)
{
	char pathname[1024];
	const char *mem, *p, *file_end;
	struct huamobile_virtual_key *key;
	size_t size;
	int x, y, width, height, code;

	sprintf(pathname, "/sys/board_properties/virtualkeys.%s", dev->name);
	mem = huamobule_file_read_all(pathname, &size);
	if (mem == NULL)
	{
		pr_red_info("huamobile_file_mmap");
		return -ENOMEM;
	}

	pr_bold_info("Parse virtual key file %s", pathname);

	dev->vk_head = NULL;
	file_end = mem + size;
	p = huamobile_text_skip_space_head(mem, file_end);

	while (p < file_end)
	{
		if (huamobile_text_lhcmp("0x01", p) || sscanf(p, "0x01:%d:%d:%d:%d:%d", &code, &x, &y, &width, &height) != 5)
		{
			p++;
			continue;
		}

		key = malloc(sizeof(*key));
		if (key == NULL)
		{
			pr_error_info("malloc");
			goto out_free_mem;
		}

		pr_bold_info("code = %d, x = %d, y = %d, width = %d, height = %d", code, x, y, width, height);

		width >>= 1;
		height >>= 1;

		key->left = x - width;
		key->right = x + width - 1;
		key->top = y - height;
		key->bottom = y + height - 1;
		key->code = code;
		key->name = huamobile_input_find_key_name(dev->kl_head, code);

		key->next = dev->vk_head;
		dev->vk_head = key;

		p = huamobile_text_nfind(p, file_end, ':', 5);
	}

out_free_mem:
	free((char *)mem);

	return 0;
}

static int huamobile_input_get_keylayout_pathname(struct huamobile_input_device *dev, char *pathname)
{
	int i;
	char *filename;
	const char *filenames[] =
	{dev->name, "Generic", "qwerty"};

	filename = huamobile_text_copy(pathname, "/system/usr/keylayout/");
	for (i = 0; i < (int)NELEM(filenames); i++)
	{
		sprintf(filename, "%s.kl", filenames[i]);
		if (access(pathname, R_OK) == 0)
		{
			return 0;
		}
	}

	return -ENOENT;
}

static int huamobile_input_parse_keylayout(struct huamobile_input_device *dev)
{
	int fd;
	int ret;
	char pathname[1024];
	const char *p, *line_end, *file_end;
	struct huamobile_keylayout_node *node;
	void *map;
	size_t size;

	ret = huamobile_input_get_keylayout_pathname(dev, pathname);
	if (ret < 0)
	{
		pr_red_info("huamobile_input_get_keylayout_pathname");
		return ret;
	}

	fd = huamobile_file_mmap(pathname, &map, &size, O_RDONLY);
	if (fd < 0)
	{
		pr_red_info("huamobile_file_mmap");
		return fd;
	}

	pr_bold_info("Parse keylayout file %s", pathname);

	dev->kl_head = NULL;
	p = map;
	file_end = p + size;
	
	node = malloc(sizeof(*node));
	if (node == NULL)
	{
		pr_error_info("malloc");
		goto out_file_unmap;
	}

	while (p < file_end)
	{
		line_end = huamobile_text_find_line_end(p, file_end);
		p = huamobile_text_skip_space_head(p, line_end);
		if (p == line_end || *p == '#')
		{
			goto label_goto_next_line;
		}

		ret = sscanf(p, "key %d %s", &node->code, node->name);
		if (ret != 2)
		{
			goto label_goto_next_line;
		}

		pr_bold_info("name = %s, code = %d", node->name, node->code);

		node->next = dev->kl_head;
		dev->kl_head = node;

		node = malloc(sizeof(*node));
		if (node == NULL)
		{
			pr_error_info("malloc");
			break;
		}

label_goto_next_line:
		p = huamobile_text_skip_line_end(line_end, file_end);
	}

	free(node);
out_file_unmap:
	huamobile_file_unmap(fd, map, size);

	return 0;
}

static void huamobile_input_close_devices(struct huamobile_input_service *service)
{
	struct huamobile_input_device *pdev, *pdev_next;
	struct huamobile_keylayout_node *kl, *kl_next;
	struct huamobile_virtual_key *vk, *vk_next;

	pr_pos_info();

	pthread_mutex_lock(&service->lock);

	for (pdev = service->dev_head; pdev; pdev = pdev->next)
	{
		if (service->remove)
		{
			service->remove(pdev, service->private_data);
		}

		close(pdev->fd);

		kl = pdev->kl_head;
		while (kl)
		{
			kl_next = kl->next;
			free(kl);
			kl = kl_next;;
		}

		vk = pdev->vk_head;
		while (vk)
		{
			vk_next = vk->next;
			free(vk);
			vk = vk_next;
		}

		pdev_next = pdev->next;
		free(pdev);
		pdev = pdev_next;
	}

	pthread_mutex_unlock(&service->lock);
}

static int huamobile_input_open_devices(struct huamobile_input_service *service)
{
	int fd;
	int ret;
	DIR *dp;
	struct dirent *entry;
	char pathname[PATH_MAX + 1], *filename;
	struct huamobile_input_device *pdev;

	pr_pos_info();

	filename = huamobile_text_copy(pathname, "/dev/input/");
	dp = opendir(pathname);
	if (dp == NULL)
	{
		pr_error_info("open directory `%s'", pathname);
		return -ENOENT;
	}

	pdev = malloc(sizeof(*pdev));
	if (pdev == NULL)
	{
		ret = -ENOMEM;
		pr_error_info("malloc");
		goto out_closedir;
	}

	while ((entry = readdir(dp)))
	{
		if (huamobile_text_lhcmp("event", entry->d_name))
		{
			continue;
		}

		huamobile_text_copy(filename, entry->d_name);
		pr_bold_info("pathname = %s", pathname);
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

		if (service->matcher && service->matcher(pdev, service->private_data) < 0)
		{
			pr_red_info("Can't match device %s, name = %s", pathname, pdev->name);
			close(fd);
			continue;
		}

		if (service->probe && service->probe(pdev, service->private_data) < 0)
		{
			pr_red_info("Faile to Init device %s, name = %s", pathname, pdev->name);
			close(fd);
			continue;
		}

		pr_green_info("Add device %s, name = %s", pathname, pdev->name);

		huamobile_input_parse_keylayout(pdev);
		huamobile_input_parse_virtual_keymap(pdev);

		pthread_mutex_lock(&service->lock);
		pdev->next = service->dev_head;
		service->dev_head = pdev;
		pthread_mutex_unlock(&service->lock);

		pdev = malloc(sizeof(*pdev));
		if (pdev == NULL)
		{
			ret = -ENOMEM;
			pr_error_info("malloc");
			huamobile_input_close_devices(service);
			goto out_closedir;
		}
	}

	ret = 0;

	free(pdev);
out_closedir:
	closedir(dp);

	return ret;
}

static size_t huamobile_input_device_count(struct huamobile_input_device *head)
{
	size_t count;

	for (count = 0; head; head = head->next)
	{
		count++;
	}

	return count;
}

static void *huamobile_input_service_handler(void *data)
{
	int ret;
	ssize_t rdlen;
	struct huamobile_input_service *service = data;
	struct pollfd fds[huamobile_input_device_count(service->dev_head) + 1], *pfd;
	struct huamobile_input_device *pdev;
	struct input_event events[32], *ep, *ep_end;

	pr_pos_info();

	pfd = fds;
	pfd->fd = service->pipefd[0];
	pfd->events = POLLIN;
	pfd->revents = 0;

	for (pdev = service->dev_head; pdev; pdev = pdev->next)
	{
		pfd++;
		pfd->fd = pdev->fd;
		pfd->events = POLLIN;
		pfd->revents = 0;
		pdev->pfd = pfd;
	}

	pthread_mutex_lock(&service->lock);
	service->state = HUA_INPUT_THREAD_STATE_RUNNING;
	pthread_mutex_unlock(&service->lock);

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

		for (pdev = service->dev_head; pdev; pdev = pdev->next)
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
				service->event_handler(pdev, ep, service->private_data);
			}
		}
	}

out_thread_exit:
	pthread_mutex_lock(&service->lock);
	service->state = HUA_INPUT_THREAD_STATE_STOPPED;
	pthread_mutex_unlock(&service->lock);

	pr_red_info("Huamobile input service exit");

	return NULL;
}

int huamobile_input_start_poll_thread(struct huamobile_input_service *service)
{
	int ret;

	pr_pos_info();

	pthread_mutex_lock(&service->lock);

	if (service->state == HUA_INPUT_THREAD_STATE_RUNNING)
	{
		pthread_mutex_unlock(&service->lock);
		return 0;
	}

	ret = pipe(service->pipefd);
	if (ret < 0)
	{
		pr_error_info("pipe");
		pthread_mutex_unlock(&service->lock);
		return ret;
	}

	ret = pthread_create(&service->thread, NULL, huamobile_input_service_handler, service);
	if (ret < 0)
	{
		pr_red_info("pthread_create");
		close(service->pipefd[0]);
		close(service->pipefd[1]);
		pthread_mutex_unlock(&service->lock);
		return ret;
	}

	pthread_mutex_unlock(&service->lock);

	return 0;
}

int huamobile_input_stop_poll_thread(struct huamobile_input_service *service)
{
	pr_pos_info();

	pthread_mutex_lock(&service->lock);

	while (service->state == HUA_INPUT_THREAD_STATE_RUNNING)
	{
		int ret;

		ret = huamobile_input_send_command(service, HUA_INPUT_COMMAND_STOP);
		if (ret < 0)
		{
			pr_error_info("huamobile_input_send_command");
			pthread_mutex_unlock(&service->lock);
			return ret;
		}

		pthread_mutex_unlock(&service->lock);
		huamobile_msleep(100);
		pthread_mutex_unlock(&service->lock);
	}

	close(service->pipefd[0]);
	close(service->pipefd[1]);

	pthread_mutex_unlock(&service->lock);

	return 0;
}

int huamobile_input_service_start(struct huamobile_input_service *service, void *data)
{
	int ret;

	pr_pos_info();

	if (service == NULL || service->event_handler == NULL)
	{
		pr_red_info("service == NULL || service->event_handler == NULL");
		return -EINVAL;
	}

	service->private_data = data;
	pthread_mutex_init(&service->lock, NULL);

	ret = huamobile_input_open_devices(service);
	if (ret < 0)
	{
		pr_red_info("huamobile_input_open_devices");
		return -ENOENT;
	}

	service->state = HUA_INPUT_THREAD_STATE_STOPPED;

	ret = huamobile_input_start_poll_thread(service);
	if (ret < 0)
	{
		pr_red_info("huamobile_input_start_poll_thread");
		huamobile_input_close_devices(service);
	}

	return 0;
}

int huamobile_input_service_stop(struct huamobile_input_service *service)
{
	int ret;

	pr_pos_info();

	ret = huamobile_input_stop_poll_thread(service);
	if (ret < 0)
	{
		pr_red_info("huamobile_input_stop_poll_thread");
		return ret;
	}

	huamobile_input_close_devices(service);
	pthread_mutex_destroy(&service->lock);

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

static int huamobile_ts_probe(struct huamobile_input_device *dev, void *data)
{
	int ret;
	int fd = dev->fd;
	int min, max, diff;
    uint8_t abs_bitmask[sizeof_bit_array(ABS_CNT)];
	struct huamobile_ts_service *service = data;
	struct huamobile_ts_device *ts;
	struct huamobile_touch_point *p, *p_end;

	pr_bold_info("lcd_width = %d, lcd_height = %d", service->lcd_width, service->lcd_height);

    memset(abs_bitmask, 0, sizeof(abs_bitmask));
	ret = ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(abs_bitmask)), abs_bitmask);
	if (ret < 0)
	{
		pr_error_info("ioctl EVIOCGBIT EV_ABS");
		return ret;
	}

	ts = malloc(sizeof(*ts));
	if (ts == NULL)
	{
		pr_red_info("malloc");
		return -ENOMEM;
	}

	ts->input_dev = dev;
	dev->private_data = ts;

	if (service->lcd_width > 0 && test_bit(ABS_MT_POSITION_X, abs_bitmask))
	{
		ret = huamobile_input_get_absinfo_base(fd, ABS_MT_POSITION_X, &min, &max);
	}
	else if (service->lcd_width > 0 && test_bit(ABS_X, abs_bitmask))
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
		ts->xscale = ((double)service->lcd_width) / diff;
		ts->xoffset = ((double)service->lcd_width) * min / diff;
	}

	if (service->lcd_height > 0 && test_bit(ABS_MT_POSITION_Y, abs_bitmask))
	{
		ret = huamobile_input_get_absinfo_base(fd, ABS_MT_POSITION_Y, &min, &max);
	}
	else if (service->lcd_height > 0 && test_bit(ABS_Y, abs_bitmask))
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
		ts->yscale = ((double)service->lcd_height) / diff;
		ts->yoffset = ((double)service->lcd_height) * min / diff;
	}

	pr_bold_info("xscale = %lf, xoffset = %lf", ts->xscale, ts->xoffset);
	pr_bold_info("yscale = %lf, yoffset = %lf", ts->yscale, ts->yoffset);

	if (service->probe)
	{
		ret = service->probe(ts, service->private_data);
		if (ret < 0)
		{
			free(ts);
			pr_red_info("service->dev_ops->init");
			return ret;
		}
	}

	ts->pressed = 0;
	ts->released = 1;
	ts->point_count = 0;

	for (p = ts->points, p_end = p + NELEM(ts->points); p < p_end; p++)
	{
		p->id = p->x = p->y = p->pressure = -1;
	}

	pthread_mutex_lock(&service->lock);
	ts->next = service->dev_head;
	service->dev_head = ts;
	pthread_mutex_unlock(&service->lock);

	return 0;
}

static void huamobile_ts_remove(struct huamobile_input_device *dev, void *data)
{
	free(dev->private_data);
}

static int huamobile_ts_event_handler(struct huamobile_input_device *dev, struct input_event *event, void *data)
{
	struct huamobile_ts_device *ts = dev->private_data;
	struct huamobile_ts_service *service = data;
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
				service->point_handler(ts, NULL, service->private_data);
				ts->released = 1;
			}
		}
		else
		{
			const char *name;

			name = huamobile_input_find_key_name(dev->kl_head, event->code);
			service->key_handler(ts, name, event->code, event->value, service->private_data);
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
				struct huamobile_virtual_key *key;

				for (p = ts->points, p_end = p + ts->point_count; p < p_end; p++)
				{
					key = huamobile_input_find_virtual_key(dev->vk_head, p->x, p->y);
					if (key)
					{
						int value = p->pressure > 0;

						if (key->value != value)
						{
							service->key_handler(ts, key->name, key->code, value, service->private_data);
						}

						key->value = value;
					}
					else if (p->pressure > 0)
					{
						p->x = p->x * ts->xscale - ts->xoffset;
						p->y = p->y * ts->yscale - ts->yoffset;
						service->point_handler(ts, p, service->private_data);
						ts->released = 0;
					}

					p->id = p->x = p->y = p->pressure = -1;
				}

				ts->point_count = 0;
			}
			else
			{
				struct huamobile_virtual_key *key;

				for (key = dev->vk_head; key; key = key->next)
				{
					if (key->value != 0)
					{
						service->key_handler(ts, key->name, key->code, 0, service->private_data);
					}

					key->value = 0;
				}

				if (ts->released == 0)
				{
					service->point_handler(ts, NULL, service->private_data);
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
	struct huamobile_ts_service *service = data;

	return service->matcher ? service->matcher(dev, service->private_data) : 0;
}

static int huamobile_ts_key_handler_dummy(struct huamobile_ts_device *dev, const char *name, int code, int value, void *data)
{
	pr_bold_info("name = %s, code = %d, value = %d", name, code, value);

	return 0;
}

static int huamobile_ts_point_handler_dummy(struct huamobile_ts_device *dev, struct huamobile_touch_point *point, void *data)
{
	if (point)
	{
		pr_bold_info("Release");
	}
	else
	{
		pr_bold_info("p[%d]: [%d, %d]", point->id, point->x, point->y);
	}

	return 0;
}

int huamobile_ts_service_start(struct huamobile_ts_service *service, void *data)
{
	struct huamobile_touch_point *p, *p_end;
	struct huamobile_input_service *input_service;

	if (service->point_handler == NULL && service->key_handler == NULL)
	{
		pr_red_info("ts->point_handler == NULL && ts->key_handler == NULL");
		return -EINVAL;
	}

	pthread_mutex_init(&service->lock, NULL);

	if (service->key_handler == NULL)
	{
		service->key_handler = huamobile_ts_key_handler_dummy;
	}

	if (service->point_handler == NULL)
	{
		service->point_handler = huamobile_ts_point_handler_dummy;
	}

	service->private_data = data;

	input_service = &service->input_service;
	input_service->event_handler = huamobile_ts_event_handler;
	input_service->matcher = huamobile_ts_matcher;
	input_service->probe = huamobile_ts_probe;
	input_service->remove = huamobile_ts_remove;

	return huamobile_input_service_start(input_service, service);
}
