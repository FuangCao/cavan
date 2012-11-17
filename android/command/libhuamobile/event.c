/*
 * File:		event.c
 * Based on:
 * Author:		Fuang Cao <cavan.cfa@gmail.com>
 *
 * Created:		2012-11-17
 * Description:	HUAMOBILE LIBRARY
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

#include <huamobile/event.h>
#include <huamobile/file.h>
#include <huamobile/text.h>

static struct huamobile_virtual_key *huamobile_event_free_virtual_keymap(struct huamobile_virtual_key *head)
{
	struct huamobile_virtual_key *key_next;

	while (head)
	{
		key_next = head->next;
		free(head);
		head = key_next;
	}

	return head;
}

static struct huamobile_keylayout_node *huamobile_event_free_keylayout(struct huamobile_keylayout_node *head)
{
	struct huamobile_keylayout_node *node_next;

	while (head)
	{
		node_next = head->next;
		free(head);
		head = node_next;
	}

	return head;
}

static int huamobile_event_parse_virtual_keymap(struct huamobile_event_device *dev)
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
		key->name = huamobile_event_find_key_name(dev, code);

		key->next = dev->vk_head;
		dev->vk_head = key;

		p = huamobile_text_nfind(p, file_end, ':', 5);
	}

out_free_mem:
	free((char *)mem);

	return 0;
}

static int huamobile_event_get_keylayout_pathname(struct huamobile_event_device *dev, char *pathname)
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

static int huamobile_event_parse_keylayout(struct huamobile_event_device *dev)
{
	int fd;
	int ret;
	char pathname[1024];
	uint8_t key_bitmask[sizeof_bit_array(KEY_CNT)];
	const char *p, *line_end, *file_end;
	struct huamobile_keylayout_node *node;
	void *map;
	size_t size;

	ret = huamobile_event_get_keylayout_pathname(dev, pathname);
	if (ret < 0)
	{
		pr_red_info("huamobile_event_get_keylayout_pathname");
		return ret;
	}

	memset(key_bitmask, 0, sizeof(key_bitmask));
	ret = ioctl(dev->fd, EVIOCGBIT(EV_KEY, sizeof(key_bitmask)), key_bitmask);
	if (ret < 0)
	{
		pr_error_info("ioctl EVIOCGBIT EV_KEY");
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
		if (ret != 2 || test_bit(node->code, key_bitmask) == 0)
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
			goto label_goto_next_line;
		}

label_goto_next_line:
		p = huamobile_text_skip_line_end(line_end, file_end);
	}

	free(node);
out_file_unmap:
	huamobile_file_unmap(fd, map, size);

	return 0;
}

static void huamobile_event_close_devices(struct huamobile_event_service *service)
{
	struct huamobile_event_device *pdev, *pdev_next;

	pr_pos_info();

	pthread_mutex_lock(&service->lock);

	pdev = service->dev_head;

	while (pdev)
	{
		if (pdev->remove)
		{
			pdev->remove(pdev, service->private_data);
		}

		close(pdev->fd);

		huamobile_event_free_virtual_keymap(pdev->vk_head);
		huamobile_event_free_keylayout(pdev->kl_head);

		pdev_next = pdev->next;
		pdev->destroy(pdev, service->private_data);
		pdev = pdev_next;
	}

	pthread_mutex_unlock(&service->lock);
}

static void huamobile_event_destroy_device_dummy(struct huamobile_event_device *dev, void *data)
{
	pr_pos_info();

	free(dev);
}

static int huamobile_event_open_devices(struct huamobile_event_service *service)
{
	int fd;
	int ret;
	DIR *dp;
	struct dirent *entry;
	char devname[1024];
	char pathname[PATH_MAX + 1], *filename;
	struct huamobile_event_device *dev;
	enum huamobile_event_device_type type;

	pr_pos_info();

	filename = huamobile_text_copy(pathname, "/dev/input/");
	dp = opendir(pathname);
	if (dp == NULL)
	{
		pr_error_info("open directory `%s'", pathname);
		return -ENOENT;
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

		ret = huamobile_event_get_devname(fd, devname, sizeof(devname));
		if (ret < 0)
		{
			pr_error_info("huamobile_event_get_devname");
			close(fd);
			continue;
		}

		if (service->matcher)
		{
			type = service->matcher(fd, devname, service->private_data);
		}
		else
		{
			type = HUA_EVENT_DEVICE_UNKNOWN;
		}

		dev = service->create_device(type, service->private_data);
		if (dev == NULL)
		{
			pr_red_info("create_device");
			close(fd);
			continue;
		}

		if (dev->destroy == NULL)
		{
			dev->destroy = huamobile_event_destroy_device_dummy;
		}

		if (dev->probe && dev->probe(dev, service->private_data) < 0)
		{
			pr_red_info("Faile to Init device %s, name = %s", pathname, devname);
			dev->destroy(dev, service->private_data);
			close(fd);
			continue;
		}

		dev->fd = fd;
		huamobile_text_copy(dev->name, devname);

		pr_green_info("Add device %s, name = %s", pathname, devname);

		huamobile_event_parse_keylayout(dev);
		huamobile_event_parse_virtual_keymap(dev);

		pthread_mutex_lock(&service->lock);
		dev->next = service->dev_head;
		service->dev_head = dev;
		pthread_mutex_unlock(&service->lock);
	}

	closedir(dp);

	return 0;
}

static size_t huamobile_event_device_count(struct huamobile_event_device *head)
{
	size_t count;

	for (count = 0; head; head = head->next)
	{
		count++;
	}

	return count;
}

static void *huamobile_event_service_handler(void *data)
{
	int ret;
	ssize_t rdlen;
	struct huamobile_event_service *service = data;
	struct pollfd fds[huamobile_event_device_count(service->dev_head) + 1], *pfd;
	struct huamobile_event_device *pdev;
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
				pdev->event_handler(pdev, ep, service->private_data);
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

struct huamobile_virtual_key *huamobile_event_find_virtual_key(struct huamobile_event_device *dev, int x, int y)
{
	struct huamobile_virtual_key *key;

	for (key = dev->vk_head; key; key = key->next)
	{
		if (y >= key->top && y <= key->bottom && x >= key->left && x <= key->right)
		{
			return key;
		}
	}

	return NULL;
}

const char *huamobile_event_find_key_name(struct huamobile_event_device *dev, int code)
{
	struct huamobile_keylayout_node *node;

	for (node = dev->kl_head; node; node = node->next)
	{
		if (node->code == code)
		{
			return node->name;
		}
	}

	return NULL;
}

int huamobile_event_start_poll_thread(struct huamobile_event_service *service)
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

	ret = pthread_create(&service->thread, NULL, huamobile_event_service_handler, service);
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

int huamobile_event_stop_poll_thread(struct huamobile_event_service *service)
{
	pr_pos_info();

	pthread_mutex_lock(&service->lock);

	while (service->state == HUA_INPUT_THREAD_STATE_RUNNING)
	{
		int ret;

		ret = huamobile_event_send_command(service, HUA_INPUT_COMMAND_STOP);
		if (ret < 0)
		{
			pr_error_info("huamobile_event_send_command");
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

static struct huamobile_event_device *huamobile_event_create_device_dummy(enum huamobile_event_device_type type, void *data)
{
	pr_pos_info();

	return malloc(sizeof(struct huamobile_event_device));
}

int huamobile_event_service_start(struct huamobile_event_service *service, void *data)
{
	int ret;

	pr_pos_info();

	if (service == NULL)
	{
		pr_red_info("service == NULL");
		return -EINVAL;
	}

	if (service->create_device == NULL)
	{
		service->create_device = huamobile_event_create_device_dummy;
	}

	service->private_data = data;
	pthread_mutex_init(&service->lock, NULL);

	ret = huamobile_event_open_devices(service);
	if (ret < 0)
	{
		pr_red_info("huamobile_event_open_devices");
		return -ENOENT;
	}

	service->state = HUA_INPUT_THREAD_STATE_STOPPED;

	ret = huamobile_event_start_poll_thread(service);
	if (ret < 0)
	{
		pr_red_info("huamobile_event_start_poll_thread");
		huamobile_event_close_devices(service);
	}

	return 0;
}

int huamobile_event_service_stop(struct huamobile_event_service *service)
{
	int ret;

	pr_pos_info();

	ret = huamobile_event_stop_poll_thread(service);
	if (ret < 0)
	{
		pr_red_info("huamobile_event_stop_poll_thread");
		return ret;
	}

	huamobile_event_close_devices(service);
	pthread_mutex_destroy(&service->lock);

	return 0;
}

bool huamobile_event_name_matcher(const char *devname, ...)
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

	return name ? true : false;
}

int huamobile_event_get_absinfo(int fd, int axis, int *min, int *max)
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
