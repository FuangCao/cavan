// Fuang.Cao <cavan.cfa@gmail.com> 2011-10-20 14:01:11

#include <cavan.h>
#include <cavan/cftp.h>
#include <sys/wait.h>

#define FILE_CREATE_DATE "2011-10-20 14:01:11"

static void show_usage(void)
{
	println("Usage:");
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
	struct cftp_descriptor cftp_desc;
	int fd_adb_en, fd_adb;
	const char *adb_dev_enable_path;
	const char *adb_dev_path;
	pid_t pid;
	int status;

	adb_dev_enable_path = DEVICE_ADB_ENABLE_PATH;
	adb_dev_path = DEVICE_ADB_PATH;

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

	fd_adb_en = open(adb_dev_enable_path, O_RDWR);
	if (fd_adb_en < 0)
	{
		print_error("open device \"%s\" failed", adb_dev_enable_path);
		return fd_adb_en;
	}

	fcntl(fd_adb_en, F_SETFD, FD_CLOEXEC);

	cftp_descriptor_init(&cftp_desc);
	cftp_desc.send = cftp_adb_send_data;
	cftp_desc.receive = cftp_adb_receive_data;
	cftp_desc.max_xfer_length = CAVAN_USB_MAX_XFER_SIZE;

	while (1)
	{
		fd_adb = open(adb_dev_path, O_RDWR);
		if (fd_adb < 0)
		{
			print_error("open device \"%s\" failed", adb_dev_path);
			break;
		}

		fcntl(fd_adb, F_SETFD, FD_CLOEXEC);

		cftp_desc.fd = fd_adb;
		cftp_desc.data.type_int = fd_adb;

		cftp_service_heandle(&cftp_desc);

		close(fd_adb);
	}

	close(fd_adb_en);

	return 0;
}
