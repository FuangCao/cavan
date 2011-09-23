// Fuang.Cao <cavan.cfa@gmail.com> Sat Jan  8 17:53:34 CST 2011

#include <cavan.h>
#include <cavan/text.h>
#include <cavan/dd.h>
#include <cavan/device.h>
#include <cavan/image.h>
#include <cavan/file.h>

#define DEFAULT_ZIMGE_PATH			"/project/kernel_imx/arch/arm/boot/zImage"
#define DEFAULT_SD_DEVICE			"/dev/sdb"
#define TEMP_UIMAGE_PATH			"/tmp/uImage"

int main(int argc, char *argv[])
{
	int ret;
	const char *dev_path;
	const char *img_path;

	if (argc < 2)
	{
		img_path = DEFAULT_ZIMGE_PATH;
		dev_path = DEFAULT_SD_DEVICE;
	}
	else if (argc < 3)
	{
		if (file_test(argv[1], "b") >= 0)
		{
			img_path = DEFAULT_ZIMGE_PATH;
			dev_path = argv[1];
		}
		else
		{
			img_path = argv[1];
			dev_path = DEFAULT_SD_DEVICE;
		}
	}
	else
	{
		img_path = argv[1];
		dev_path = argv[2];

		adjust_image_device(&img_path, &dev_path);
	}

	if (image_is(img_path, "zImage"))
	{
		ret = burn_zImage(img_path, dev_path);
		if (ret < 0)
		{
			error_msg("burn_zImage");
			return ret;
		}
	}
	else
	{
		ret = burn_uImage(img_path, dev_path);
		if (ret < 0)
		{
			error_msg("burn_uImage");
			return ret;
		}
	}

	return 0;
}
