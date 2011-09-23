// Fuang.Cao <cavan.cfa@gmail.com> Sat Jan  8 19:08:38 CST 2011

#include <cavan.h>
#include <cavan/dd.h>
#include <cavan/image.h>

#define DEFAULT_UBOOT_PATH			"/project/uboot_imx/u-boot.bin"
#define TEMP_UBOOT_NOPADDING_PATH	"/tmp/u-boot-no-padding.bin"

int main(int argc, char *argv[])
{
	int ret;
	const char *uboot_path;
	const char *uboot_np_path;

	if (argc < 2)
	{
		uboot_path = DEFAULT_UBOOT_PATH;
	}
	else
	{
		uboot_path = argv[1];
	}

	if (argc < 3)
	{
		uboot_np_path = TEMP_UBOOT_NOPADDING_PATH;
	}
	else
	{
		uboot_np_path = argv[2];
	}

	ret = uboot2uboot_np(uboot_path, uboot_np_path);
	if (ret < 0)
	{
		error_msg("uboot2uboot_np");
		return ret;
	}

	return 0;
}
