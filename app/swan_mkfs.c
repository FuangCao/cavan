#include <cavan.h>
#include <cavan/swan_dev.h>
#include <cavan/device.h>
#include <cavan/swan_upgrade.h>

static void show_usage(void)
{
	println("Usage:");
	println("swan_mkfs [--force-format] [--volume=label] dev_path");
}

int main(int argc, char *argv[])
{
	struct partition_desc part_descs[] =
	{
		{
			.major = 179,
			.minor = FIRST_MINOR + 1,
			.flags = MKFS_FLAG_TEST,
			.path = EMMC_DEVICE "p1",
			.label = EMMC_VFAT_DEFAULT_LABEL,
			.type = FS_VFAT,
		},
		{
			.major = 179,
			.minor = FIRST_MINOR + 5,
			.flags = 0,
			.path = EMMC_DEVICE "p5",
			.label = "data",
			.type = FS_EXT3,
		},
		{
			.major = 179,
			.minor = FIRST_MINOR + 6,
			.flags = 0,
			.path = EMMC_DEVICE "p6",
			.label = "cache",
			.type = FS_EXT3,
		},
	};
	struct partition_desc emmc_desc =
	{
		.major = 179,
		.minor = 0,
		.path = EMMC_DEVICE,
	};
	struct option long_option[] =
	{
		{
			.name = "force-format",
			.has_arg = 0,
			.flag = NULL,
			.val = 0,
		},
		{
			.name = "label",
			.has_arg = 1,
			.flag = NULL,
			.val = 1,
		},
		{
			.name = "volume",
			.has_arg = 1,
			.flag = NULL,
			.val = 1,
		},
		{
			0, 0, 0, 0
		},
	};
	int c;
	int option_index;

	while ((c = getopt_long(argc, argv, "", long_option, &option_index)) != EOF)
	{
		switch (c)
		{
		case 0:
			part_descs[0].flags &= ~MKFS_FLAG_TEST;
			break;

		case 1:
			strcpy(part_descs[0].label, optarg);
			break;

		default:
			show_usage();
			return -EINVAL;
		}
	}

	assert(argc > optind);

	if (argv[optind][0] >= '0' && argv[optind][0] <= '9')
	{
		emmc_desc.path[text_len(emmc_desc.path) - 1] = argv[1][0];
		return swan_mkfs(&emmc_desc, part_descs, ARRAY_SIZE(part_descs));
	}

	sprintf(part_descs[0].path, "%s1", argv[optind]);
	sprintf(part_descs[1].path, "%s5", argv[optind]);
	sprintf(part_descs[2].path, "%s6", argv[optind]);
	strcpy(emmc_desc.path, argv[optind]);

	return swan_mkfs(&emmc_desc, part_descs, ARRAY_SIZE(part_descs));
}
