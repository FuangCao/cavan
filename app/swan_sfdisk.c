#include <cavan.h>
#include <cavan/swan_dev.h>
#include <cavan/text.h>
#include <cavan/swan_upgrade.h>

static void show_usage(void)
{
	println("Usage:");
}

int main(int argc, char *argv[])
{
	int ret;
	int c;
	int option_index;
	struct option long_option[] =
	{
		{
			.name = "help",
			.has_arg = no_argument,
			.flag = NULL,
			.val = 'h',
		},
		{
			.name = "part-system",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 's',
		},
		{
			.name = "part-recovery",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 'r',
		},
		{
			.name = "part-userdata",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 'u',
		},
		{
			.name = "part-data",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 'd',
		},
		{
			.name = "part-cache",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 'c',
		},
		{
			.name = "part-vendor",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 'v',
		},
		{
			0, 0, 0, 0
		},
	};
	struct partition_desc emmc_desc =
	{
		.major = 179,
		.minor = FIRST_MINOR,
		.path = EMMC_DEVICE,
		.label = "no label",
		.type = -1,
		.flags = 0,
	};
	struct swan_emmc_partition_table part_table =
	{
		.system_size = SYSTEM_SIZE,
		.recovery_size = RECOVERY_SIZE,
		.userdata_size = USERDATA_SIZE,
		.cache_size = CACHE_SIZE,
		.vendor_size = 0,
	};

	while ((c = getopt_long(argc, argv, "hH2:s:S:4:r:R:5:u:U:d:D:6:c:C:7:v:V:", long_option, &option_index)) != EOF)
	{
		switch (c)
		{
		case 'h':
		case 'H':
			show_usage();
			return 0;

		case '2':
		case 's':
		case 'S':
			part_table.system_size = text2size_mb(optarg);
			break;

		case '4':
		case 'r':
		case 'R':
			part_table.recovery_size = text2size_mb(optarg);
			break;

		case '5':
		case 'u':
		case 'U':
		case 'd':
		case 'D':
			part_table.userdata_size = text2size_mb(optarg);
			break;

		case '6':
		case 'c':
		case 'C':
			part_table.cache_size = text2size_mb(optarg);
			break;

		case '7':
		case 'v':
		case 'V':
			part_table.vendor_size = text2size_mb(optarg);
			break;

		default:
			show_usage();
			return -EINVAL;
		}
	}

	assert(optind < argc);

	if (text_is_number(argv[optind]))
	{
		text_copy(emmc_desc.path + text_len(emmc_desc.path) - 1, argv[optind]);
	}
	else
	{
		text_copy(emmc_desc.path, argv[optind]);
	}

	optind++;

	if (optind < argc)
	{
		ret = sscanf(argv[optind], "%d,%d,%d,%d,%d", &part_table.system_size, &part_table.recovery_size, &part_table.userdata_size, &part_table.cache_size, &part_table.vendor_size);
		if (ret < 4 || ret > 5)
		{
			error_msg("argument fault %d", ret);
			return -EINVAL;
		}

		if (ret == 4)
		{
			part_table.vendor_size = 0;
		}
	}

	show_swan_emmc_partation_table(&part_table);
	show_partition_desc(&emmc_desc);

	return swan_sfdisk(&emmc_desc, &part_table);
}
