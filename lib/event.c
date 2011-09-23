// Fuang.Cao <cavan.cfa@gmail.com> Wed May 25 10:12:17 CST 2011

#include <cavan.h>
#include <cavan/event.h>
#include <cavan/file.h>

int event_init_by_path(struct event_desc *desc, const char *pathname)
{
	int fd;
	int ret;

	fd = file_open_ro(pathname);
	if (fd < 0)
	{
		print_error("open file \"%s\" failed", pathname);
		return fd;
	}

	ret = device_get_devname(fd, desc->dev_name, sizeof(desc->dev_name));
	if (ret < 0)
	{
		close(fd);
		return ret;
	}

	text_copy(desc->dev_path, pathname);
	desc->fd = fd;

	return 0;
}

int event_init_by_name(struct event_desc *desc, const char *devname)
{
	int fd;
	char devpath[1024], *p;

	p = text_copy(devpath, INPUT_EVENT_PATH);
	p[1] = 0;

	for (*p = '0'; ; (*p)++)
	{
		int ret;
		char temp_name[1024];

		fd = file_open_ro(devpath);
		if (fd < 0)
		{
			return fd;
		}

		ret = device_get_devname(fd, temp_name, sizeof(temp_name));
		if (ret < 0)
		{
			error_msg("device_get_devname");
			goto label_close_fd;
		}

		println("devpath = %s, name = %s", devpath, temp_name);

		if (text_cmp(temp_name, devname) == 0)
		{
			break;
		}

label_close_fd:
		close(fd);
	}

	desc->fd = fd;
	text_copy(desc->dev_path, devpath);
	text_copy(desc->dev_name, devname);

	return 0;
}

void event_uninit(struct event_desc *desc)
{
	if (desc == NULL)
	{
		return;
	}

	close(desc->fd);
}

char *event_to_text(struct input_event *event, char *text)
{
	char *text_bak = text;
	const char *event_code = "UNKNOWN";

	switch (event->type)
	{
	case EV_SYN:
		switch (event->code)
		{
		case SYN_REPORT:
			text_copy(text, "SYN_REPORT");
			return text_bak;

		case SYN_CONFIG:
			text_copy(text, "SYN_CONFIG");
			return text_bak;

		case SYN_MT_REPORT:
			text_copy(text, "SYN_MT_REPORT");
			return text_bak;

		default:
			sprintf(text, "EV_SYN[%d] = %d", event->code, event->value);
			return text;
		}
		break;
	case EV_KEY:
		switch (event->code)
		{
		case BTN_0:
			event_code = "BTN_0";
			break;
		case BTN_1:
			event_code = "BTN_1";
			break;
		case BTN_2:
			event_code = "BTN_2";
			break;
		case BTN_3:
			event_code = "BTN_3";
			break;
		case BTN_4:
			event_code = "BTN_4";
			break;
		case BTN_5:
			event_code = "BTN_5";
			break;
		case BTN_6:
			event_code = "BTN_6";
			break;
		case BTN_7:
			event_code = "BTN_7";
			break;
		case BTN_8:
			event_code = "BTN_8";
			break;
		case BTN_9:
			event_code = "BTN_9";
			break;
		case BTN_LEFT:
			event_code = "BTN_LEFT";
			break;
		case BTN_RIGHT:
			event_code = "BTN_RIGHT";
			break;
		case BTN_MIDDLE:
			event_code = "BTN_MIDDLE";
			break;
		case BTN_SIDE:
			event_code = "BTN_SIDE";
			break;
		case BTN_EXTRA:
			event_code = "BTN_EXTRA";
			break;
		case BTN_FORWARD:
			event_code = "BTN_FORWARD";
			break;
		case BTN_BACK:
			event_code = "BTN_BACK";
			break;
		case BTN_TASK:
			event_code = "BTN_TASK";
			break;
		case BTN_TRIGGER:
			event_code = "BTN_TRIGGER";
			break;
		case BTN_THUMB:
			event_code = "BTN_THUMB";
			break;
		case BTN_THUMB2:
			event_code = "BTN_THUMB2";
			break;
		case BTN_TOP:
			event_code = "BTN_TOP";
			break;
		case BTN_TOP2:
			event_code = "BTN_TOP2";
			break;
		case BTN_PINKIE:
			event_code = "BTN_PINKIE";
			break;
		case BTN_BASE:
			event_code = "BTN_BASE";
			break;
		case BTN_BASE2:
			event_code = "BTN_BASE2";
			break;
		case BTN_BASE3:
			event_code = "BTN_BASE3";
			break;
		case BTN_BASE4:
			event_code = "BTN_BASE4";
			break;
		case BTN_BASE5:
			event_code = "BTN_BASE5";
			break;
		case BTN_BASE6:
			event_code = "BTN_BASE6";
			break;
		case BTN_DEAD:
			event_code = "BTN_DEAD";
			break;
		case BTN_A:
			event_code = "BTN_A";
			break;
		case BTN_B:
			event_code = "BTN_B";
			break;
		case BTN_C:
			event_code = "BTN_C";
			break;
		case BTN_X:
			event_code = "BTN_X";
			break;
		case BTN_Y:
			event_code = "BTN_Y";
			break;
		case BTN_Z:
			event_code = "BTN_Z";
			break;
		case BTN_TL:
			event_code = "BTN_TL";
			break;
		case BTN_TR:
			event_code = "BTN_TR";
			break;
		case BTN_TL2:
			event_code = "BTN_TL2";
			break;
		case BTN_TR2:
			event_code = "BTN_TR2";
			break;
		case BTN_SELECT:
			event_code = "BTN_SELECT";
			break;
		case BTN_START:
			event_code = "BTN_START";
			break;
		case BTN_MODE:
			event_code = "BTN_MODE";
			break;
		case BTN_THUMBL:
			event_code = "BTN_THUMBL";
			break;
		case BTN_THUMBR:
			event_code = "BTN_THUMBR";
			break;
		case BTN_TOOL_PEN:
			event_code = "BTN_TOOL_PEN";
			break;
		case BTN_TOOL_RUBBER:
			event_code = "BTN_TOOL_RUBBER";
			break;
		case BTN_TOOL_BRUSH:
			event_code = "BTN_TOOL_BRUSH";
			break;
		case BTN_TOOL_PENCIL:
			event_code = "BTN_TOOL_PENCIL";
			break;
		case BTN_TOOL_AIRBRUSH:
			event_code = "BTN_TOOL_AIRBRUSH";
			break;
		case BTN_TOOL_FINGER:
			event_code = "BTN_TOOL_FINGER";
			break;
		case BTN_TOOL_MOUSE:
			event_code = "BTN_TOOL_MOUSE";
			break;
		case BTN_TOOL_LENS:
			event_code = "BTN_TOOL_LENS";
			break;
		case BTN_TOUCH:
			event_code = "BTN_TOUCH";
			break;
		case BTN_STYLUS:
			event_code = "BTN_STYLUS";
			break;
		case BTN_STYLUS2:
			event_code = "BTN_STYLUS2";
			break;
		case BTN_TOOL_DOUBLETAP:
			event_code = "BTN_TOOL_DOUBLETAP";
			break;
		case BTN_TOOL_TRIPLETAP:
			event_code = "BTN_TOOL_TRIPLETAP";
			break;
		case BTN_GEAR_DOWN:
			event_code = "BTN_GEAR_DOWN";
			break;
		case BTN_GEAR_UP:
			event_code = "BTN_GEAR_UP";
			break;
		default:
			sprintf(text, "EV_KEY[%d] = %d", event->code, event->value);
			return text;
		}
		break;
	case EV_REL:
		switch (event->code)
		{
		case REL_X:
			event_code = "REL_X";
			break;
		case REL_Y:
			event_code = "REL_Y";
			break;
		case REL_Z:
			event_code = "REL_Z";
			break;
		case REL_RX:
			event_code = "REL_RX";
			break;
		case REL_RY:
			event_code = "REL_RY";
			break;
		case REL_RZ:
			event_code = "REL_RZ";
			break;
		case REL_HWHEEL:
			event_code = "REL_HWHEEL";
			break;
		case REL_DIAL:
			event_code = "REL_DIAL";
			break;
		case REL_WHEEL:
			event_code = "REL_WHEEL";
			break;
		case REL_MISC:
			event_code = "REL_MISC";
			break;
		case REL_MAX:
			event_code = "REL_MAX";
			break;
		default:
			sprintf(text, "EV_REL[%d] = %d", event->code, event->value);
			return text;
		}
		break;
	case EV_ABS:
		switch (event->code)
		{
		case ABS_X:
			event_code = "ABS_X";
			break;
		case ABS_Y:
			event_code = "ABS_Y";
			break;
		case ABS_Z:
			event_code = "ABS_Z";
			break;
		case ABS_RX:
			event_code = "ABS_RX";
			break;
		case ABS_RY:
			event_code = "ABS_RY";
			break;
		case ABS_RZ:
			event_code = "ABS_RZ";
			break;
		case ABS_THROTTLE:
			event_code = "ABS_THROTTLE";
			break;
		case ABS_RUDDER:
			event_code = "ABS_RUDDER";
			break;
		case ABS_WHEEL:
			event_code = "ABS_WHEEL";
			break;
		case ABS_GAS:
			event_code = "ABS_GAS";
			break;
		case ABS_BRAKE:
			event_code = "ABS_BRAKE";
			break;
		case ABS_HAT0X:
			event_code = "ABS_HAT0X";
			break;
		case ABS_HAT0Y:
			event_code = "ABS_HAT0Y";
			break;
		case ABS_HAT1X:
			event_code = "ABS_HAT1X";
			break;
		case ABS_HAT1Y:
			event_code = "ABS_HAT1Y";
			break;
		case ABS_HAT2X:
			event_code = "ABS_HAT2X";
			break;
		case ABS_HAT2Y:
			event_code = "ABS_HAT2Y";
			break;
		case ABS_HAT3X:
			event_code = "ABS_HAT3X";
			break;
		case ABS_HAT3Y:
			event_code = "ABS_HAT3Y";
			break;
		case ABS_PRESSURE:
			event_code = "ABS_PRESSURE";
			break;
		case ABS_DISTANCE:
			event_code = "ABS_DISTANCE";
			break;
		case ABS_TILT_X:
			event_code = "ABS_TILT_X";
			break;
		case ABS_TILT_Y:
			event_code = "ABS_TILT_Y";
			break;
		case ABS_TOOL_WIDTH:
			event_code = "ABS_TOOL_WIDTH";
			break;
		case ABS_VOLUME:
			event_code = "ABS_VOLUME";
			break;
		case ABS_MISC:
			event_code = "ABS_MISC";
			break;
		case ABS_MT_SLOT:
			event_code = "ABS_MT_SLOT";
			break;
		case ABS_MT_TOUCH_MAJOR:
			event_code = "ABS_MT_TOUCH_MAJOR";
			break;
		case ABS_MT_TOUCH_MINOR:
			event_code = "ABS_MT_TOUCH_MINOR";
			break;
		case ABS_MT_WIDTH_MAJOR:
			event_code = "ABS_MT_WIDTH_MAJOR";
			break;
		case ABS_MT_WIDTH_MINOR:
			event_code = "ABS_MT_WIDTH_MINOR";
			break;
		case ABS_MT_ORIENTATION:
			event_code = "ABS_MT_ORIENTATION";
			break;
		case ABS_MT_POSITION_X:
			event_code = "ABS_MT_POSITION_X";
			break;
		case ABS_MT_POSITION_Y:
			event_code = "ABS_MT_POSITION_Y";
			break;
		case ABS_MT_TOOL_TYPE:
			event_code = "ABS_MT_TOOL_TYPE";
			break;
		case ABS_MT_BLOB_ID:
			event_code = "ABS_MT_BLOB_ID";
			break;
		case ABS_MT_TRACKING_ID:
			event_code = "ABS_MT_TRACKING_ID";
			break;
		case ABS_MT_PRESSURE:
			event_code = "ABS_MT_PRESSURE";
			break;
		case ABS_MAX:
			event_code = "ABS_MAX";
			break;
		default:
			sprintf(text, "EV_ABS[%d] = %d", event->code, event->value);
			return text;
		}
		break;
	case EV_MSC:
		text_copy(text, "EV_MSC");
		return text;

	case EV_SW:
		text_copy(text, "EV_SW");
		return text;

	case EV_LED:
		text_copy(text, "EV_LED");
		return text;

	case EV_SND:
		text_copy(text, "EV_SND");
		return text;

	case EV_REP:
		text_copy(text, "EV_REP");
		return text;

	case EV_FF:
		text_copy(text, "EV_FF");
		return text;

	case EV_PWR:
		text_copy(text, "EV_PWR");
		return text;

	case EV_FF_STATUS:
		text_copy(text, "EV_FF_STATUS");
		return text;

	default:
		return event_to_text_simple(event, text);
	}

	sprintf(text, "%s = %d", event_code, event->value);

	return text_bak;
}

ssize_t read_events(struct event_desc *desc, struct input_event *buff, size_t count)
{
	ssize_t readlen;

	readlen = ffile_read(desc->fd, buff, sizeof(*buff) * count);
	if (readlen < 0)
	{
		return readlen;
	}

	return readlen / sizeof(*buff);
}

ssize_t read_events_timeout(struct event_desc *desc, struct input_event *buff, size_t count, long timeout)
{
	int ret;

	ret = file_select_read(desc->fd, timeout);
	if (ret <= 0)
	{
		return -ETIMEDOUT;
	}

	return read_events(desc, buff, count);
}

int read_event_by_type(struct event_desc *desc, u16 type, struct input_event *event)
{
	while (1)
	{
		ssize_t readlen;

		readlen = ffile_read(desc->fd, event, sizeof(*event));
		if (readlen < 0)
		{
			return readlen;
		}

		if (readlen != sizeof(*event))
		{
			continue;
		}

		if (event->type == type)
		{
			break;
		}
	}

	return 0;
}

ssize_t open_event_devices(struct pollfd *event_fds, size_t max_count, int flags)
{
	int i;
	ssize_t count;
	char tmp_path[1024], *p;
	char dev_name[1024];

	p = text_copy(tmp_path, "/dev/input/event");

	for (i = 0, count = 0; i < max_count; i++)
	{
		__value2text(i, p, 0, 0, 10);

		event_fds[count].fd = open(tmp_path, flags);
		if (event_fds[count].fd < 0)
		{
			continue;
		}

		if (device_get_devname(event_fds[count].fd, dev_name, sizeof(dev_name)) < 0)
		{
			pr_red_info("device_get_devname");
			close(event_fds[count].fd);
			continue;
		}

		pr_green_info("device path = %s, device name = %s", tmp_path, dev_name);

		event_fds[count].events = POLLIN;
		event_fds[count].revents = 0;
		count++;
	}

	return count;
}

void close_event_devices(struct pollfd *event_fds, size_t count)
{
	struct pollfd *event_end;

	for(event_end = event_fds + count; event_fds < event_end; event_fds++)
	{
		close(event_fds->fd);
	}
}

ssize_t poll_event_devices(struct pollfd *event_fds, size_t count, void *buff, size_t length)
{
	int ret;
	ssize_t readlen;
	void *buff_end;
	struct pollfd *event_end;

	ret = poll(event_fds, count, -1);
	if (ret < 0)
	{
		return ret;
	}

	for (event_end = event_fds + count, buff_end = buff + length; event_fds < event_end && buff < buff_end; event_fds++)
	{
		if (event_fds->revents == 0)
		{
			continue;
		}

		readlen = read(event_fds->fd, buff, buff_end - buff);
		if (readlen < 0)
		{
			print_error("read");
			return readlen;
		}

		buff += readlen;
	}

	return length - (buff_end - buff);
}
