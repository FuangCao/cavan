#include <stdio.h>
#include <poll.h>
#include <sys/types.h>
#include <linux/limits.h>
#include <linux/input.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>

#define test_bit(bit, array) \
	((array)[(bit) >> 3] & (1 << ((bit) & 0x07)))

#define sizeof_bit_array(bits) \
	(((bits) + 7) >> 3)

#define NELEM(a) \
	(sizeof(a) / sizeof((a)[0]))

#define pr_pos_info() \
	printf("%s => %s[%d]\n", __FILE__, __FUNCTION__, __LINE__)

#define pr_red_info(fmt, args ...) \
	printf("\033[31m" fmt "\033[0m\n", ##args)

#define pr_green_info(fmt, args ...) \
	printf("\033[32m" fmt "\033[0m\n", ##args)

#define pr_bold_info(fmt, args ...) \
	printf("\033[1m" fmt "\033[0m\n", ##args)

#define pr_error_info(fmt, args ...) \
	if (errno) { \
		pr_red_info("%s[%d](" fmt "): %s", __FUNCTION__, __LINE__, ##args, strerror(errno)); \
	} else { \
		pr_red_info("%s[%d]:" fmt, __FUNCTION__, __LINE__, ##args); \
	}

#ifndef SYN_MT_REPORT
#define SYN_MT_REPORT		2
#define ABS_MT_POSITION_X	0x35	/* Center X ellipse position */
#define ABS_MT_POSITION_Y	0x36	/* Center Y ellipse position */
#define ABS_MT_TOUCH_MAJOR	0x30	/* Major axis of touching ellipse */
#endif

struct huamobile_touch_point
{
	int x;
	int y;
	int pressure;
};

struct huamobile_virtual_key
{
	int left;
	int right;
	int top;
	int bottom;
	int code;
};

struct huamobile_virtual_keypad
{
	int count;
	struct huamobile_virtual_key keys[8];
};

static int text_lhcmp(const char *text1, const char *text2)
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

static char *text_copy(char *dest, const char *src)
{
	while ((*dest = *src))
	{
		dest++;
		src++;
	}

	return dest;
}

static ssize_t open_input_devices(struct pollfd *fds, ssize_t size, int (*check)(int fd, const char *name, void *data), void *data)
{
	DIR *dp;
	struct dirent *entry;
	char pathname[PATH_MAX + 1], *filename;
	char devname[512];
	int fd;
	int count;
	int ret;

	filename = text_copy(pathname, "/dev/input/");
	dp = opendir(pathname);
	if (dp == NULL)
	{
		pr_error_info("open directory `%s'", pathname);
		return -ENOENT;
	}

	count = 0;

	while (count < size && (entry = readdir(dp)))
	{
		if (text_lhcmp("event", entry->d_name))
		{
			continue;
		}

		text_copy(filename, entry->d_name);
		fd = open(pathname, O_RDONLY);
		if (fd < 0)
		{
			pr_error_info("open file `%s'", pathname);
			continue;
		}

		ret = ioctl(fd, EVIOCGNAME(sizeof(devname)), devname);
		if (ret < 0)
		{
			pr_error_info("ioctl EVIOCGNAME");
			close(fd);
			continue;
		}

		if (check && check(fd, devname, data) < 0)
		{
			close(fd);
			pr_bold_info("Skipping %s => %s", pathname, devname);
			continue;
		}

		pr_bold_info("pathname %s, name = %s", pathname, devname);

		fds[count].fd = fd;
		fds[count].events = POLLIN;
		fds[count].revents = 0;
		count++;
	}

	return count;
}

static void close_input_devices(struct pollfd *fds, ssize_t size)
{
	struct pollfd *p;

	for (p = fds + size - 1; p >= fds; p--)
	{
		close(p->fd);
	}
}

static int parse_virtual_keymap(const char *devname, struct huamobile_virtual_keypad *keypad)
{
	int fd;
	ssize_t rdlen;
	int count;
	char buff[2048], *p, *p_end;
	struct huamobile_virtual_key *key;
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

	key = keypad->keys;

	for (p = buff, p_end = p + rdlen; p < p_end; p++)
	{
		if (text_lhcmp("0x01", p) || sscanf(p, "0x01:%d:%d:%d:%d:%d", &code, &x, &y, &width, &height) != 5)
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

	keypad->count = key - keypad->keys;

	return keypad->count;
}

static int touchscreen_check(int fd, const char *name, void *data)
{
#if 0
	int ret;
    uint8_t abs_bitmask[sizeof_bit_array(ABS_MAX + 1)];
    uint8_t key_bitmask[sizeof_bit_array(KEY_MAX + 1)];

    memset(key_bitmask, 0, sizeof(key_bitmask));
	ret = ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(key_bitmask)), key_bitmask);
    if (ret < 0)
	{
		pr_error_info("ioctl EVIOCGBIT EV_KEY");
		return ret;
	}

    memset(abs_bitmask, 0, sizeof(abs_bitmask));
	ret = ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(abs_bitmask)), abs_bitmask);
	if (ret < 0)
	{
		pr_error_info("ioctl EVIOCGBIT EV_ABS");
		return ret;
	}

	if (test_bit(ABS_MT_POSITION_X, abs_bitmask) && test_bit(ABS_MT_POSITION_Y, abs_bitmask))
	{
		pr_bold_info("Mutil Touch");
		return 0;
	}

	if (test_bit(BTN_TOUCH, key_bitmask) && test_bit(ABS_X, abs_bitmask) && test_bit(ABS_Y, abs_bitmask) && test_bit(ABS_Z, abs_bitmask) == 0)
	{
		pr_bold_info("Single Touch");
		return 0;
	}
#else
	int i;
	const char *dev_names[] =
		{"FT5216", "CY8C242"};

	for (i = 0; i < (int)NELEM(dev_names); i++)
	{
		if (text_lhcmp(dev_names[i], name) == 0 && parse_virtual_keymap(name, data) > 0)
		{
			return 0;
		}
	}
#endif

	return -EINVAL;
}

static ssize_t input_poll_events(struct pollfd *fds, size_t count, struct input_event *events, size_t size, int timeout)
{
	int ret;
	struct pollfd *p;
	struct input_event *event, *event_end;
	ssize_t rdlen;

	ret = poll(fds, count, timeout);
	if (ret < 0)
	{
		pr_error_info("poll");
		return ret;
	}

	for (p = fds + count - 1, event = events, event_end = event + size; p >= fds && event < event_end; p--)
	{
		if (p->revents == 0)
		{
			continue;
		}

		rdlen = read(p->fd, event, (event_end - event) * sizeof(struct input_event));
		if (rdlen < 0)
		{
			pr_error_info("read");
			return rdlen;
		}

		event += rdlen / sizeof(struct input_event);
	}

	return event - events;
}

static int poll_points(struct pollfd *fds, size_t count, void (*handler)(int id, int x, int y, void *data), void *data)
{
	int i;
	ssize_t ev_count;
	struct input_event events[32], *ep, *ep_end;
	struct huamobile_touch_point points[10];
	int point_count, pressed;

	if (count == 0 || handler == NULL)
	{
		pr_red_info("count == 0 || handler == NULL");
		return 0;
	}

	for (i = 0; i < (int)NELEM(points); i++)
	{
		points[i].x = points[i].y = points[i].pressure = -1;
	}

	pressed = 0;
	point_count = 0;

	while (1)
	{
		ev_count = input_poll_events(fds, count, events, NELEM(events), 0);
		if (ev_count < 0)
		{
			pr_red_info("input_poll_events");
			return ev_count;
		}

		for (ep = events, ep_end = ep + ev_count; ep < ep_end; ep++)
		{
			switch (ep->type)
			{
			case EV_ABS:
				switch (ep->code)
				{
				case ABS_X:
				case ABS_MT_POSITION_X:
					points[point_count].x = ep->value;
					break;

				case ABS_Y:
				case ABS_MT_POSITION_Y:
					points[point_count].y = ep->value;
					break;

				case ABS_PRESSURE:
				case ABS_MT_TOUCH_MAJOR:
					points[point_count].pressure = ep->value;
				}
				break;

			case EV_KEY:
				if (ep->code == BTN_TOUCH)
				{
					pressed = ep->value;
				}
				break;

			case EV_SYN:
				switch (ep->code)
				{
				case SYN_MT_REPORT:
					point_count++;
					break;

				case SYN_REPORT:
					if (point_count == 0 && pressed)
					{
						points[point_count++].pressure = 0;
					}

					for (i = 0; i < point_count; i++)
					{
						if (points[i].pressure > 0 && points[i].x > 0 && points[i].y > 0)
						{
							pr_bold_info("p%d = [%d, %d]", i, points[i].x, points[i].y);\
							handler(i, points[i].x, points[i].y, data);
							points[i].x = points[i].y = points[i].pressure = -1;
						}
					}

					point_count = 0;
					break;
				}
				break;
			}
		}
	}
}

static void point_handler(int id, int x, int y, void *data)
{
	struct huamobile_virtual_keypad *keypad = data;
	struct huamobile_virtual_key *key, *key_end;

	for (key = keypad->keys, key_end = key + keypad->count; key < key_end; key++)
	{
		if (y >= key->top && y <= key->bottom && x >= key->left && x <= key->right)
		{
			pr_bold_info("key code = %d", key->code);
		}
	}
}

int main(int argc, char *argv[])
{
	int ret;
	struct pollfd input_fd;
	ssize_t count;
	struct huamobile_virtual_keypad keypad;

	count = open_input_devices(&input_fd, 1, touchscreen_check, &keypad);
	if (count < 0)
	{
		pr_red_info("open_input_devices");
		return count;
	}

	ret = poll_points(&input_fd, count, point_handler, &keypad);
	if (ret < 0)
	{
		pr_red_info("poll_points");
	}

	close_input_devices(&input_fd, count);

	return 0;
}
