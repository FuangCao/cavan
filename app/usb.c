// Fuang.Cao <cavan.cfa@gmail.com> Thu Sep  8 15:06:14 CST 2011

#include <cavan.h>
#include <cavan/event.h>
#include <cavan/usb.h>
#include <sys/wait.h>

#define DEVICE_SWAN_VK_DATA		"/sys/devices/platform/swan_vk.0/data"

#define FILE_CREATE_DATE "Thu Sep  8 15:06:15 CST 2011"

static void show_usage(void)
{
	println("Usage:");
}

static int swan_adb_server(void)
{
	int fd_adb, fd_adb_en, fd_data;
	ssize_t readlen, writelen;
	pid_t pid;
	int status;
	char buff[4096];

	pr_bold_pos();

	pid = fork();
	if (pid == 0)
	{
		execl("/system/bin/setprop", "setprop", "persist.service.adb.enable", "0", NULL);
	}

	waitpid(pid, &status, 0);
	if (WIFEXITED(status) == 0 || WEXITSTATUS(status) != 0)
	{
		error_msg("close adb server failed");
		return -1;
	}

	sleep(1);

	fd_data = open(DEVICE_SWAN_VK_DATA, O_WRONLY);
	if (fd_data < 0)
	{
		print_error("open device \"%s\" failed", DEVICE_ADB_ENABLE_PATH);
		return fd_data;
	}

	fd_adb_en = open(DEVICE_ADB_ENABLE_PATH, O_RDWR);
	if (fd_adb_en < 0)
	{
		print_error("open device \"%s\" failed", DEVICE_ADB_ENABLE_PATH);
		goto out_close_data;
	}

	fcntl(fd_adb_en, F_SETFD, FD_CLOEXEC);

label_open_adb:
	fd_adb = open(DEVICE_ADB_PATH, O_RDONLY);
	if (fd_adb < 0)
	{
		print_error("open device \"%s\" failed", DEVICE_ADB_PATH);
		goto out_close_adb_en;
	}

	fcntl(fd_adb, F_SETFD, FD_CLOEXEC);

	while (1)
	{
		readlen = cavan_adb_read_data(fd_adb, buff, sizeof(buff));
		if (readlen < 0)
		{
			print_error("read");
			close(fd_adb);
			goto label_open_adb;
		}

		writelen = write(fd_data, buff,  readlen);
		if (writelen < 0)
		{
			print_error("write");
			break;
		}
	}

	close(fd_adb);
out_close_adb_en:
	close(fd_adb_en);
out_close_data:
	close(fd_data);

	return -1;
}

static int swan_adb_client(const char *dev_path)
{
	int ret;
	struct cavan_usb_descriptor desc;
	struct pollfd event_fds[32];
	int count;
	ssize_t readlen, writelen;
	struct input_event event_buff[32];

	pr_bold_pos();

	system_command("killall adb");

	ret = cavan_find_usb_device(dev_path, &desc);
	if (ret < 0)
	{
		error_msg("cavan_find_usb_device failed");
		return ret;
	}

	pr_bold_info("usb device path = %s", desc.dev_path);
	pr_bold_info("usb device serial = %s", desc.serial);
	pr_bold_info("idProduct = 0x%04x", desc.dev_desc.idProduct);
	pr_bold_info("idVendor = 0x%04x", desc.dev_desc.idVendor);

	count = open_event_devices(event_fds, ARRAY_SIZE(event_fds), O_RDONLY);
	if (count <= 0)
	{
		pr_red_info("no input device found");
		goto out_usb_uninit;
	}

	while (1)
	{
		readlen = poll_event_devices(event_fds, count, event_buff,  sizeof(event_buff));
		if (readlen < 0)
		{
			print_error("poll_event_devices");
			break;
		}

		writelen = cavan_usb_write_data(&desc, event_buff, readlen);
		if (writelen < 0)
		{
			print_error("cavan_usb_bluk_write");
			break;
		}
	}

	close_event_devices(event_fds, count);
out_usb_uninit:
	cavan_usb_uninit(&desc);

	return -1;
}

int main(int argc, char *argv[])
{
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
		},
	};

	while ((c = getopt_long(argc, argv, "vVhH", long_option, &option_index)) != EOF)
	{
		switch (c)
		{
		case 'v':
		case 'V':
			show_author_info();
			println(FILE_CREATE_DATE);
			return 0;

		case 'h':
		case 'H':
			show_usage();
			return 0;

		default:
			show_usage();
			return -EINVAL;
		}
	}

	if (access(DEVICE_ADB_ENABLE_PATH, F_OK) < 0)
	{
		return swan_adb_client(argc > optind ? argv[optind] : NULL);
	}
	else
	{
		return swan_adb_server();
	}

	return 0;
}
