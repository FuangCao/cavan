#include <cavan.h>
#include <cavan/text.h>
#include <cavan/dd.h>
#include <cavan/image.h>

static char *get_device(int index)
{
	switch (index) {
	case 0:
		return "/dev/mmcblk0";
	case 1:
		return "/dev/mmcblk1";
	default:
		pr_err_info("Unknown Device");
	}

	return NULL;
}

static int extend_partition(const char *dev_path)
{
	system_command("e2fsck -fy %s", dev_path);
	system_command("resize2fs -pf %s", dev_path);

	return 0;
}

static inline int copy_to_emmc(const char *img_name, const char *img_path, const char *target_device, off_t seek_out)
{
	if (img_path) {
		img_name = img_path;
	}

	return cavan_dd(img_name, target_device, 0, seek_out, 0);
}

int main(int argc, char *argv[])
{
	int ret;
	const char *target_device;
	const char *img_path;

	if (argv[1][0] != '-') {
		printf("Unknown Option\n");
		return -1;
	}

	if (argc >= 4) {
		target_device = get_device(argv[2][0] - '0');
		img_path = argv[3];
	} else {
		target_device = get_device(0);
		if (argc >= 3) {
			img_path = argv[2];
		} else {
			img_path = NULL;
		}
	}

	switch (argv[1][1]) {
	case 'u':
	case 'U':
		if (img_path == NULL) {
			return cavan_dd("u-boot-no-padding.bin", target_device, 0, UBOOT_OFFSET, 0);
		} else if (strcmp(text_basename(img_path), "u-boot.bin") == 0) {
			return cavan_dd(img_path, target_device, UBOOT_PADDING_SIZE, UBOOT_OFFSET, 0);
		} else {
			return cavan_dd(img_path, target_device, 0, UBOOT_OFFSET, 0);
		}

	case 'k':
	case 'K':
		return copy_to_emmc("uImage", img_path, target_device, UIMAGE_OFFSET);

	case 'l':
	case 'L':
		return copy_to_emmc("logo.bmp", img_path, target_device, LOGO_OFFSET);

	case 'b':
	case 'B':
		return copy_to_emmc("busybox.img", img_path, target_device, BUSYBOX_OFFSET);

	case 'c':
	case 'C':
		return copy_to_emmc("charge.bmps", img_path, target_device, CARTOON_OFFSET);

	case 's':
	case 'S':
		target_device = format_text("%sp2", target_device);
		ret = copy_to_emmc("system.img", img_path, target_device, 0);
		if (ret < 0) {
			pr_err_info("copy_to_emmc");
			return ret;
		}
		break;

	case 'd':
	case 'D':
		target_device = format_text("%sp5", target_device);
		ret = copy_to_emmc("userdata.img", img_path, target_device, 0);
		if (ret < 0) {
			pr_err_info("copy_to_emmc");
			return ret;
		}
		break;

	case 'r':
	case 'R':
		switch (argv[1][2]) {
		case 'a':
		case 'A':
			return copy_to_emmc("uramdisk.img", img_path, target_device, RAMDISK_OFFSET);

		case 'e':
		case 'E':
			target_device = format_text("%sp4", target_device);
			ret = copy_to_emmc("recovery.img", img_path, target_device, 0);
			if (ret < 0) {
				pr_err_info("copy_to_emmc");
				return ret;
			}
			break;

		default:
			printf("Unknown Option\n");
			return -1;
		}
		break;

	default:
		printf("Unknown Option\n");
		return -1;
	}

	return extend_partition(target_device);
}
