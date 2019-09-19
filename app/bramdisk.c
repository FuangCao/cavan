#define CAVAN_CMD_NAME bramdisk

// Fuang.Cao <cavan.cfa@gmail.com> Sun Jan  9 17:23:29 CST 2011

#include <cavan.h>
#include <cavan/image.h>
#include <cavan/file.h>

#define DEFAULT_RAMDISK_PATH		"/project/rootfs/upgrade"
#define DEFAULT_DEVICE_PATH			"/dev/sdb"

int main(int argc, char *argv[])
{
	int ret;
	const char *ramdisk_path;
	const char *dev_path;

	if (argc < 2) {
		ramdisk_path = DEFAULT_RAMDISK_PATH;
		dev_path = DEFAULT_DEVICE_PATH;
	} else if (argc < 3) {
		if (file_test(argv[1], "b") >= 0) {
			ramdisk_path = DEFAULT_RAMDISK_PATH;
			dev_path = argv[1];
		} else {
			ramdisk_path = argv[1];
			dev_path = DEFAULT_DEVICE_PATH;
		}
	} else {
		ramdisk_path = argv[1];
		dev_path = argv[2];

		adjust_image_device(&ramdisk_path, &dev_path);
	}

	if (file_test(ramdisk_path, "d") >= 0) {
		if (image_is(ramdisk_path, "ramdisk")) {
			ret = burn_directory(ramdisk_path, dev_path, 0);
		} else {
			ret = burn_directory(ramdisk_path, dev_path, 1);
		}
	} else if (image_is(ramdisk_path, "uramdisk.img")) {
		ret = burn_uramdisk(ramdisk_path, dev_path, 0);
	} else if (image_is(ramdisk_path, "busybox.img")) {
		ret = burn_uramdisk(ramdisk_path, dev_path, 1);
	} else {
		ret = burn_ramdisk(ramdisk_path, dev_path, 0);
	}

	if (ret < 0) {
		pr_err_info("Burn ramdisk failed");
		return ret;
	}

	return 0;
}
