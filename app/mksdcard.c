#include <cavan.h>
#include <cavan/text.h>
#include <cavan/device.h>

#define SDCARD_DEFAULE_DEVICE	"/dev/sdb"

static int mksdcard(const char *sd_device)
{
	int ret;

	umount_device(sd_device, MNT_DETACH);

	ret = system_command("sfdisk %s -uM << EOF\n64,,L\nEOF", sd_device);
	if (ret < 0)
	{
		print_error("system_command");
		return ret;
	}

	sleep(1);

	ret = system_command("mkfs.vfat %s1 -n sdcard", sd_device);
	if (ret < 0)
	{
		print_error("system_command");
		return ret;
	}

	right_msg("mksdcard Success");

	return 0;
}

int main(int argc, char *argv[])
{
	if (argc > 1)
	{
		return mksdcard(argv[1]);
	}
	else
	{
		return mksdcard(SDCARD_DEFAULE_DEVICE);
	}
}
