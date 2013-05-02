// Fuang.Cao <cavan.cfa@gmail.com> Sun May 15 11:31:08 CST 2011

#include <cavan.h>
#include <cavan/uevent.h>
#include <cavan/text.h>
#include <cavan/device.h>

int main(int argc, char *argv[])
{
	int ret;
	struct uevent_desc udesc;

	ret = uevent_init(&udesc);
	if (ret < 0)
	{
		error_msg("uevent_init");
		return ret;
	}

	while (1)
	{
		char devpath[1024];

		ret = get_partition_add_uevent(&udesc);
		if (ret < 0)
		{
			error_msg("get_partition_add_uevent");
			break;
		}

		if (uevent_get_propery_devname(&udesc, devpath) == NULL)
		{
			continue;
		}

		if (file_test(devpath, "b") < 0)
		{
			continue;
		}

		println("partition \"%s\" added", devpath);

		mount_main(devpath, NULL, NULL, NULL);
	}

	uevent_deinit(&udesc);

	return 0;
}
