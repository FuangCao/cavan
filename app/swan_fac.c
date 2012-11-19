#include <cavan.h>
#include <cavan/dd.h>
#include <cavan/file.h>
#include <cavan/text.h>
#include <cavan/image.h>

#define IMAGE_NAME			"factory.bin"
#define TARGET_DEVICE		"/dev/sdb"

struct image_t
{
	const char *name;
	off_t offset;
};

struct image_t images[] =
{
	{
		.name = "u-boot-factory.bin",
		.offset = 0,
	},
	{
		.name = "uImage",
		.offset = UIMAGE_OFFSET,
	},
	{
		.name = "busybox.img",
		.offset = RAMDISK_OFFSET,
	},
};

static int create_image(const char *img_name)
{
	int fd_in, fd_out;
	int ret;
	unsigned int i;

	fd_out = open(img_name, WRITE_FLAGS | O_TRUNC, 0777);
	if (fd_out < 0)
	{
		print_error("open iamge");
		return -1;
	}

	for (i = 0; i < ARRAY_SIZE(images); i++)
	{
		println("image name = %s", images[i].name);

		fd_in = open(images[i].name, READ_FLAGS);
		if (fd_in < 0)
		{
			ret = -1;
			print_error("open image");
			goto out_close_fd_out;
		}

		ret = fcavan_dd(fd_in, fd_out, 0, images[i].offset, 0, 0);
		if (ret < 0)
		{
			error_msg("fcavan_dd");
			goto out_cloae_fd_in;
		}

		close(fd_in);
	}

	ret = 0;
	goto out_close_fd_out;

out_cloae_fd_in:
	close(fd_in);
out_close_fd_out:
	close(fd_out);

	return ret;
}

static int check_image_crc(const char *dev_name, const char *img_name)
{
	u32 dev_crc, img_crc;
	int ret;
	struct stat st;

	ret = file_stat2(img_name, &st);
	if (ret < 0)
	{
		print_error("stat");
		return ret;
	}

	dev_crc = img_crc = 0;

	ret = file_ncrc32(dev_name, st.st_size, &dev_crc);
	if (ret < 0)
	{
		error_msg("file_ncrc32");
		return ret;
	}

	ret = file_ncrc32(img_name, st.st_size, &img_crc);
	if (ret < 0)
	{
		error_msg("file_ncrc32");
		return ret;
	}

	println("device crc = 0x%08x, image crc = 0x%08x", dev_crc, img_crc);

	if (dev_crc ^ img_crc)
	{
		error_msg("Data CRC Is Not Match");
		return -1;
	}

	right_msg("Data CRC Is Matched");

	return 0;
}

static int burn_image(const char *dev_name)
{
	int ret;

	ret = cavan_dd(IMAGE_NAME, dev_name, 0, 0, 0);
	if (ret < 0)
	{
		error_msg("cavan_dd");
		return ret;
	}

	ret = check_image_crc(dev_name, IMAGE_NAME);
	if (ret < 0)
	{
		return ret;
	}

	right_msg("Burn EMMC Successed\n");

	return 0;
}

static int make_sd_card(const char *dev_name)
{
	int ret;

	ret = system_command("sudo umount %s*", dev_name);
	if (ret < 0)
	{
		print_error("system_command");
		return ret;
	}

	ret = system_command("sudo sfdisk -uM %s << EOF\n100,,L\nEOF\n", dev_name);
	if (ret < 0)
	{
		print_error("system_command");
		return ret;
	}

	ret = system_command("sudo mkfs.vfat %s1", dev_name);
	if (ret < 0)
	{
		print_error("system_command");
	}

	return ret;
}

int main(int argc, char *argv[])
{
	const char *img_name;
	const char *dev_name;

	if (argc < 2)
	{
		return burn_image(TARGET_DEVICE);
	}
	else if (argv[1][0] != '-')
	{
		return burn_image(argv[1]);
	}

	if (argc < 3)
	{
		img_name = IMAGE_NAME;
		dev_name = TARGET_DEVICE;
	}
	else
	{
		img_name = dev_name = argv[2];
	}

	switch (argv[1][1])
	{
	case 'c':
	case 'C':
		return create_image(img_name);
	case 'b':
	case 'B':
		return burn_image(dev_name);
	case 't':
	case 'T':
		return check_image_crc(TARGET_DEVICE, img_name);
	case 'm':
	case 'M':
		return make_sd_card(dev_name);
	case 'a':
	case 'A':
		return burn_image(dev_name) >= 0 && make_sd_card(dev_name) >= 0;
	default:
		error_msg("unknown argument");
	}

	return -1;
}
