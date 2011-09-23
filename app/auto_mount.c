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
		char devname[64], devpath[32];

		ret = get_partition_add_uevent(&udesc, devname);
		if (ret < 0)
		{
			error_msg("get_partition_add_uevent");
			break;
		}

		text_path_cat(devpath, "/dev", devname);

		println("partition \"%s\" added", devpath);

		mount_main(devpath, NULL, NULL, NULL);
	}

	uevent_uninit(&udesc);

	return 0;
}
