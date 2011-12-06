// Fuang.Cao <cavan.cfa@gmail.com> 2011-12-06 19:11:21

#include <cavan.h>
#include <cavan/mtd.h>

#define FILE_CREATE_DATE "2011-12-06 19:11:21"

static void show_usage(void)
{
	println("Usage:");
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
		},
	};
	int ret;
	struct cavan_mtd_descriptor desc;
	struct mtd_partition_descriptor *part;

	while ((c = getopt_long(argc, argv, "vVhH", long_option, &option_index)) != EOF)
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

		default:
			show_usage();
			return -EINVAL;
		}
	}

	assert(argc - optind > 0);

	ret = cavan_mtd_init(&desc, NULL);
	if (ret < 0)
	{
		error_msg("cavan_mtd_init");
		return ret;
	}

	part = cavan_mtd_find_partition_by_name(&desc, argv[optind]);
	if (part == NULL)
	{
		error_msg("cavan_mtd_find_partition_by_name");
		ret = -ENOENT;
		goto out_mtd_uninit;
	}

	cavan_show_mtd_partition(part);

	ret = cavan_mtd_erase_partition(part);
	if (ret < 0)
	{
		error_msg("cavan_mtd_erase_partition");
		goto out_mtd_uninit;
	}

	ret = 0;

out_mtd_uninit:
	cavan_mtd_uninit(&desc);

	return ret;
}
