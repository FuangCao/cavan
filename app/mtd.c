// Fuang.Cao <cavan.cfa@gmail.com> 2011-12-06 19:11:21

#include <cavan.h>
#include <cavan/mtd.h>
#include <cavan/command.h>

#define FILE_CREATE_DATE "2011-12-06 19:11:21"

static void show_usage(void)
{
	println("Usage:");
	println("mtd -l [partition]");
	println("mtd -e partition");
	println("mtd -r partition file");
	println("mtd -w partition file");
	println("mtd -i partition image");
}

int main(int argc, char *argv[])
{
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
			.name = "version",
			.has_arg = no_argument,
			.flag = NULL,
			.val = 'v',
		},
		{
			.name = "erase",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 'e',
		},
		{
			.name = "write",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 'w',
		},
		{
			.name = "read",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 'r',
		},
		{
			.name = "list",
			.has_arg = no_argument,
			.flag = NULL,
			.val = 'l',
		},
		{
			.name = "image",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 'i',
		},
		{
			0, 0, 0, 0
		},
	};
	int ret;
	struct cavan_mtd_descriptor desc;
	struct mtd_partition_descriptor *part;
	int action;
	char partname[64];

	action = CAVAN_COMMAND_OPTION_NONE;
	partname[0] = 0;

	while ((c = getopt_long(argc, argv, "vVhHlLw:W:e:E:r:R:i:I:", long_option, &option_index)) != EOF)
	{
		switch (c)
		{
		case 'v':
		case 'V':
			show_author_info();
			println(FILE_CREATE_DATE);
			return 0;

		case 'h':
		case 'H':
			show_usage();
			return 0;

		case 'r':
		case 'R':
			action = CAVAN_COMMAND_OPTION_READ;
			text_copy(partname, optarg);
			break;

		case 'w':
		case 'W':
			action = CAVAN_COMMAND_OPTION_WRITE;
			text_copy(partname, optarg);
			break;

		case 'e':
		case 'E':
			action = CAVAN_COMMAND_OPTION_ERASE;
			text_copy(partname, optarg);
			break;

		case 'l':
		case 'L':
			action = CAVAN_COMMAND_OPTION_LIST;
			break;

		case 'i':
		case 'I':
			action = CAVAN_COMMAND_OPTION_IMAGE;
			text_copy(partname, optarg);
			break;

		default:
			show_usage();
			return -EINVAL;
		}
	}

	ret = cavan_mtd_init(&desc, NULL);
	if (ret < 0)
	{
		error_msg("cavan_mtd_init");
		return ret;
	}

	if (action == CAVAN_COMMAND_OPTION_LIST)
	{
		if (optind < argc)
		{
			struct mtd_partition_info *info;

			info = cavan_mtd_get_partition_info_by_name(&desc, argv[optind]);
			if (info == NULL)
			{
				error_msg("cavan_mtd_get_partition_info_by_name");
				ret = -1;
				goto out_mtd_deinit;
			}

			cavan_mtd_show_parts_info(info, 1);
		}
		else
		{
			cavan_mtd_show_parts_info(desc.part_infos, desc.part_count);
		}
		ret = 0;
		goto out_mtd_deinit;
	}

	if (partname[0] == 0)
	{
		show_usage();
		ret = -EINVAL;
		goto out_mtd_deinit;
	}

	part = cavan_mtd_open_partition2(&desc, partname, O_RDWR);
	if (part == NULL)
	{
		error_msg("cavan_mtd_open_partition2");
		ret = -1;
		goto out_mtd_deinit;
	}

	cavan_mtd_show_parts_info(part->part_info, 1);

	switch (action)
	{
	case CAVAN_COMMAND_OPTION_ERASE:
		ret = cavan_mtd_erase_partition(part);
		break;

	case CAVAN_COMMAND_OPTION_READ:
		ret = 0;
		break;

	case CAVAN_COMMAND_OPTION_WRITE:
		if (optind < argc)
		{
			ret = cavan_mtd_write_partition2(part, argv[optind]);
		}
		else
		{
			pr_red_info("Please input filename");
			ret = -EINVAL;
		}
		break;

	case CAVAN_COMMAND_OPTION_IMAGE:
		if (optind < argc)
		{
			ret = cavan_mtd_write_image2(part, argv[optind]);
		}
		else
		{
			pr_red_info("Please input filename");
			ret = -EINVAL;
		}
		break;

	default:
		pr_red_info("unknown action");
		ret = -EINVAL;
		goto out_close_part;
	}

out_close_part:
	cavan_mtd_close_partition(part);
out_mtd_deinit:
	cavan_mtd_deinit(&desc);

	return ret;
}
