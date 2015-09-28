// Fuang.Cao <cavan.cfa@gmail.com> Thu Apr 28 18:46:37 CST 2011

#include <cavan.h>
#include <cavan/file.h>
#include <cavan/device.h>

int main(int argc, char *argv[])
{
	int ret;
	int c;
	char loop_path[32];

	while ((c = getopt(argc, argv, "C:c:D:d:")) != EOF) {
		switch (c) {
		case 'c':
		case 'C':
			ret = file_set_loop(optarg, loop_path, 0);
			if (ret < 0) {
				error_msg("file_set_loop");
				return ret;
			}
			println("loop device is: %s", loop_path);
			return 0;

		case 'd':
		case 'D':
			ret = loop_clr_fd(optarg);
			if (ret < 0) {
				error_msg("loop_clr_fd");
				return ret;
			}
			return 0;

		default:
			error_msg("illegal option");
			return -EINVAL;
		}
	}

	error_msg("no function handle thie action");

	return 0;
}

