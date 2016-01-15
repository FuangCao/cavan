// Fuang.Cao <cavan.cfa@gmail.com> Thu Jan 13 21:57:30 CST 2011

#include <cavan.h>
#include <cavan/file.h>
#include <cavan/image.h>

#define DEFAULT_RECOVERY_IMAGE	"/project/myandroid/out/target/product/imx51_bbg/recovery.img"
#define DEFAULT_DEVICE_PATH		"/dev/sdb4"

int main(int argc, char *argv[])
{
	int ret;
	const char *img_path;
	const char *dev_path;

	if (argc < 2) {
		img_path = DEFAULT_RECOVERY_IMAGE;
		dev_path = DEFAULT_DEVICE_PATH;
	} else if (argc < 3) {
		if (file_test(argv[1], "b") >= 0) {
			img_path = DEFAULT_RECOVERY_IMAGE;
			dev_path = argv[1];
		} else {
			img_path = argv[1];
			dev_path = DEFAULT_DEVICE_PATH;
		}
	} else {
		img_path = argv[1];
		dev_path = argv[2];

		adjust_image_device(&img_path, &dev_path);
	}

	ret = burn_normal_image(img_path, dev_path);
	if (ret < 0) {
		pr_err_info("Burn image \"%s\" to \"%s\" failed", img_path, dev_path);
	}

	return ret;
}


