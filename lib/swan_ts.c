/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Thu Apr  5 18:45:28 CST 2012
 */

#include <cavan.h>
#include <cavan/swan_ts.h>

int swan_ts_do_calication(int argc, char *argv[])
{
	int fd;
	int ret;
	const char *devpath = argc > 1 ? argv[1] : SWAN_TS_MISC_DEVICE;

	fd = open(devpath, 0);
	if (fd < 0)
	{
		pr_red_info("Open device \"%s\" failed", devpath);
		return fd;
	}

	ret = ioctl(fd, SWAN_TS_IOCTL_CALIBRATION);
	if (ret < 0)
	{
		pr_red_info("Failed");
	}
	else
	{
		pr_green_info("OK");
	}

	close(fd);

	return ret;
}
