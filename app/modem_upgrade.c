// Fuang.Cao <cavan.cfa@gmail.com> Tue May 17 17:47:42 CST 2011

#include <cavan.h>
#include <cavan/device.h>
#include <sys/wait.h>
#include <cavan/file.h>
#include <cavan/swan_upgrade.h>
#include <pthread.h>

#define STRING_LIST_MAX_LEN			MB(1)
#define STRING_LIST_NAME			"StringList.strings"
#define UPDATE_WIZARD_NAME			"UpdateWizard"

#define MODEM_VERSION_PROP_NAME		"IDS_PRODUCE_Version"
#define MODEM_TTY_DEVICE			"/dev/ttyUSB0"

#define BUSYBOX_MOUNT_POINT			"/mnt/busybox"
#define BUSYBOX_DEV_MOUNT_POINT		BUSYBOX_MOUNT_POINT "/dev"

#define MODEM_USB_POWER_PATH		"/sys/bus/usb/devices/2-1/power"
#define USB_POWER_WAKEUP_PATH		MODEM_USB_POWER_PATH "/wakeup"
#define USB_POWER_CONTROL_PATH		MODEM_USB_POWER_PATH "/control"
#define MODEM_POWER_PATH			"/sys/swan/power/swan_modem"

#define MODEM_ATCMD_READ_VERSION	"AT+CGMR"

struct modem_prop
{
	char name[64];
	char value[1024];
};

static ssize_t parse_string_list(const char *dirname, struct modem_prop *props, size_t size)
{
	char buff[STRING_LIST_MAX_LEN];
	char pathname[1024], *p, *end_p;
	size_t prop_count;
	ssize_t readlen;

	text_path_cat(pathname, dirname, STRING_LIST_NAME);

	readlen = file_read(pathname, buff, sizeof(buff));
	if (readlen < 0)
	{
		error_msg("file_read \"%s\" failed", pathname);
		return readlen;
	}

	prop_count = 0;
	p = buff;
	end_p = buff + readlen;

	while (p < end_p && prop_count < size)
	{
		int i;
		char *tmp_text;

		for (i = 0, tmp_text = props[prop_count].name; i < 2; i++, p++)
		{

			do {
				if (p >= end_p)
				{
					return prop_count;
				}
			} while (*p++ != '"');


			do {
				if (p >= end_p)
				{
					return prop_count;
				}

				*tmp_text++ = *p++;

			} while (*p != '"');

			tmp_text = props[prop_count].value;
		}

#ifdef CAVAN_DEBUG
		println("%s = %s", props[prop_count].name, props[prop_count].value);
#endif

		prop_count++;

		while (p < end_p && *p++ != '\n');
	}

	return prop_count;
}

static struct modem_prop *modem_find_prop(struct modem_prop *props, size_t size, const char *prop_name)
{
	struct modem_prop *prop_end;

	for (prop_end = props + size; props < prop_end; props++)
	{
		if (text_cmp(prop_name, props->name) == 0)
		{
			return props;
		}
	}

	return NULL;
}

static char *modem_read_new_version(const char *dirname, char *version, size_t size)
{
	ssize_t prop_count;
	struct modem_prop props[100];
	struct modem_prop *prop_tmp;

	prop_count = parse_string_list(dirname, props, ARRAY_SIZE(props));
	if (prop_count < 0)
	{
		error_msg("parse_string_list");
		return NULL;
	}

	prop_tmp = modem_find_prop(props, prop_count, MODEM_VERSION_PROP_NAME);
	if (prop_tmp == NULL)
	{
		return NULL;
	}

	text_ncopy(version, prop_tmp->value, size);

	return version;
}

static int file_puts_retry(const char *pathname, const char *value, int retry)
{
	while (retry-- && file_puts(pathname, value) < 0)
	{
		sleep(2);
	}

	return retry;
}

static int set_usb_power(void)
{
	int ret;

	ret = file_puts_retry(USB_POWER_CONTROL_PATH, "on", 10);
	if (ret < 0)
	{
#ifdef CAVAN_DEBUG
		warning_msg("write file \"%s\"", USB_POWER_CONTROL_PATH);
#endif
		return ret;
	}

	ret = file_puts_retry(USB_POWER_WAKEUP_PATH, "disabled", 10);
	if (ret < 0)
	{
#ifdef CAVAN_DEBUG
		warning_msg("write file \"%s\"", USB_POWER_WAKEUP_PATH);
#endif
		return ret;
	}

	return 0;
}

static int modem_power_enable(void)
{
	int ret;

	ret = file_write(MODEM_POWER_PATH, "off", 3);
	if (ret < 0)
	{
#ifdef CAVAN_DEBUG
		error_msg("write file \"%s\"", MODEM_POWER_PATH);
#endif
		return ret;
	}

	visual_ssleep(5);

	ret = file_write(MODEM_POWER_PATH, "on", 2);
	if (ret < 0)
	{
#ifdef CAVAN_DEBUG
		error_msg("write file \"%s\"", MODEM_POWER_PATH);
#endif
		return ret;
	}

	visual_ssleep(5);

	ret = file_wait(MODEM_TTY_DEVICE, "c", 10);
	if (ret < 0)
	{
#ifdef CAVAN_DEBUG
		error_msg("modem power enable failed");
#endif
		return ret;
	}

	visual_ssleep(5);

	return set_usb_power();
}

static int modem_send_at_command(const char *command, char *buff, size_t size)
{
	int fd;
	int ret;
	ssize_t rwlen;
	char *p;

	fd = open(MODEM_TTY_DEVICE, O_RDWR | O_SYNC | O_TRUNC | O_NOCTTY);
	if (fd < 0)
	{
		print_error("open device \"%s\" failed", MODEM_TTY_DEVICE);
		return fd;
	}

	ret = set_tty_mode(fd, 4);
	if (ret < 0)
	{
		error_msg("set_tty_mode");
		goto out_close_fd;
	}

	p = text_cat3(buff, 2, command, "\r\n");

	rwlen = write(fd, buff, p - buff - 1);
	if (rwlen < 0)
	{
		print_error("write command \"%s\" failed", command);
		ret = rwlen;
		goto out_restore_tty;
	}

	ret = read(fd, buff, size);

out_restore_tty:
	restore_tty_attr(fd);
out_close_fd:
	close(fd);

	return ret;
}

static char *modem_read_old_version(char *version, size_t size)
{
	ssize_t readlen;
	char buff[1024];

	readlen = modem_send_at_command(MODEM_ATCMD_READ_VERSION, buff, sizeof(buff));
	if (readlen < 0)
	{
		print_error("modem_send_at_command");
		return NULL;
	}

	buff[readlen] = 0;
	text_get_line(buff, version, 3);

	return version;
}

static int modem_if_need_upgrade(const char *dirname, int retry)
{
	char new_version[512], old_version[512];

	if (modem_read_new_version(dirname, new_version, sizeof(new_version)) == NULL)
	{
		error_msg("modem_read_new_version");
		return -EFAULT;
	}

	println("new version = %s", new_version);

	while (retry-- && modem_read_old_version(old_version, sizeof(old_version)) == NULL);
	if (retry < 0)
	{
		error_msg("modem_read_old_version");
		return -EFAULT;
	}

	println("old version = %s", old_version);

#if 0
	return text_version_cmp(new_version, old_version, '.') > 0
#else
	return text_cmp(new_version, old_version) != 0;
#endif
}

void *set_usb_power_handle(void *data)
{
	while (1)
	{
		set_usb_power();
		sleep(2);
	}

	return NULL;
}

static int upgrade_modem(const char *resource)
{
	int ret;
	char update_wizard[1024];
	pid_t pid;
	pthread_t usb_thread;

	text_path_cat(update_wizard, resource, UPDATE_WIZARD_NAME);

	ret = chmod(update_wizard, 0777);
	if (ret < 0)
	{
		print_error("chmod \"%s\"", update_wizard);
		return ret;
	}

	pid = fork();
	if (pid < 0)
	{
		print_error("fork");
		return pid;
	}

	if (pid == 0)
	{
		if (file_test("/bin/sh", "x") == 0)
		{
			ret = system_command("%s %s | tee %s", update_wizard, resource, SWAN_CONSOLE_DEVICE);

			exit(ret);
		}
		else
		{
			ret = execl(update_wizard, update_wizard, resource, NULL);
			if (ret < 0)
			{
				error_msg("execl");
			}

			exit(-1);
		}
	}

	pthread_create(&usb_thread, NULL, set_usb_power_handle, NULL);

	waitpid(pid, &ret, 0);
	if (!WIFEXITED(ret) || WEXITSTATUS(ret) != 0)
	{
		error_msg("upgrade modem failed");
		return -1;
	}

	return 0;
}

static void show_usage(void)
{
	println("Usage:");
	println("modem_upgrade resource");
	println("modem_upgrade -v");
}

int main(int argc, char *argv[])
{
	int ret;
	int c;
	int option_index;
	struct option long_option[] =
	{
		{
			.name = "help",
			.has_arg = no_argument,
			.flag = NULL,
			.val = 'h',
		},
		{
			.name = "version",
			.has_arg = no_argument,
			.flag = NULL,
			.val = 'v',
		},
		{
			.name = "verbose",
			.has_arg = no_argument,
			.flag = NULL,
			.val = 'p',
		},
		{
			0, 0, 0, 0
		},
	};
	char resource_path[1024];

	while ((c = getopt_long(argc, argv, "vVhHpP", long_option, &option_index)) != EOF)
	{
		switch (c)
		{
		case 'v':
		case 'V':
			{
				char version[512];

				if (modem_power_enable() < 0)
				{
					error_msg("can't open modem power");
					return -EFAULT;
				}

				if (modem_read_old_version(version, sizeof(version)) == NULL)
				{
					error_msg("modem_read_old_version");
					return -EFAULT;
				}

				println("current version = %s", version);
				return 0;
			}

		case 'p':
		case 'P':
			open_console(SWAN_CONSOLE_DEVICE);
			break;

		case 'h':
		case 'H':
			show_usage();
			return 0;

		default:
			show_usage();
			return -EINVAL;
		}
	}

	assert(argc > optind);

	if (modem_power_enable() < 0)
	{
		error_msg("can't open modem power");
		return -EFAULT;
	}

	text_path_cat(resource_path, argv[optind], NULL);

	ret = modem_if_need_upgrade(resource_path, 10);
	if (ret < 0)
	{
		error_msg("modem_if_need_upgrade");
		return ret;
	}

	if (ret == 0)
	{
		right_msg("current version is newest, don't need upgrade");
		return 0;
	}

	return upgrade_modem(resource_path);
}
