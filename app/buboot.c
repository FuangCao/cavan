// Fuang.Cao <cavan.cfa@gmail.com> Sat Jan  8 18:59:37 CST 2011

#include <cavan.h>
#include <cavan/text.h>
#include <cavan/dd.h>
#include <cavan/device.h>
#include <cavan/image.h>
#include <cavan/file.h>

#define DEFAULT_UBOOT_PATH		"/project/uboot_imx/u-boot.bin"
#define DEFAULT_SD_DEVICE		"/dev/sdb"

int main(int argc, char *argv[])
{
	int ret;
	const char *uboot_path;
	const char *dev_path;

	if (argc < 2) {
		uboot_path = DEFAULT_UBOOT_PATH;
		dev_path = DEFAULT_SD_DEVICE;
	} else if (argc < 3) {
		if (file_test(argv[1], "b") >= 0) {
			uboot_path = DEFAULT_UBOOT_PATH;
			dev_path = argv[1];
		} else {
			uboot_path = argv[1];
			dev_path = DEFAULT_SD_DEVICE;
		}
	} else {
		uboot_path = argv[1];
		dev_path = argv[2];

		adjust_image_device(&uboot_path, &dev_path);
	}

	if (image_is(uboot_path, "u-boot-no-padding")) {
		ret = burn_uboot_np(uboot_path, dev_path);
		if (ret < 0) {
			error_msg("burn_uboot_np");
			return ret;
		}
	} else {
		ret = burn_uboot(uboot_path, dev_path);
		if (ret < 0) {
			error_msg("burn_uboot");
			return ret;
		}
	}

	return 0;
}
