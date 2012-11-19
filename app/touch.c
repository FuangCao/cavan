// Fuang.Cao <cavan.cfa@gmail.com> Fri Dec 31 02:20:05 CST 2010

#include <cavan.h>
#include <cavan/file.h>
#include <cavan/text.h>
#include <cavan/device.h>
#include <utime.h>

static void show_usage(void)
{
	println("Usage:");
	println("touch -t fs_type -l volume -s size filepath");
}

int main(int argc, char *argv[])
{
	int ret;
	int c;
	int opt_index;
	struct filesystem_desc *fs_desc = NULL;
	off_t length = 0;
	struct option long_opts[] =
	{
		{
			.name = "fs",
			.has_arg = required_argument,
			.val = 't',
		},
		{
			.name = "fs_type",
			.has_arg = required_argument,
			.val = 't',
		},
		{
			.name = "label",
			.has_arg = required_argument,
			.val = 'l',
		},
		{
			.name = "volume",
			.has_arg = required_argument,
			.val = 'l',
		},
		{
			0, 0, 0, 0
		},
	};
	struct partition_desc part_desc =
	{
		.major = 0,
		.minor = 0,
		.flags = 0,
		.path = "",
		.label = "",
		.type = -1,
	};

	while ((c = getopt_long(argc, argv, "s:S:l:L:t:T:", long_opts, &opt_index)) != EOF)
	{
		switch (c)
		{
		case 't':
		case 'T':
			fs_desc = get_fsdesc_by_name(optarg);
			break;

		case 'l':
		case 'L':
			text_copy(part_desc.label, optarg);
			break;

		case 's':
		case 'S':
			length = text2size(optarg, NULL);
			break;

		default:
			show_usage();
			return -EINVAL;
		}
	}

	if (optind >= argc)
	{
		show_usage();
		return -EINVAL;
	}

	if (length == 0)
	{
		return utime(argv[optind], NULL);
	}

	text_copy(part_desc.path, argv[optind]);

	ret = file_resize(part_desc.path, length);
	if (ret < 0)
	{
		print_error("file_resize");
		return ret;
	}

	if (fs_desc == NULL)
	{
		return 0;
	}

	part_desc.type = fs_desc->type;

	return partition_mkfs(&part_desc);
}
