// Fuang.Cao <cavan.cfa@gmail.com> Wed Jun 22 17:08:27 CST 2011

#include <cavan.h>
#include <cavan/ext2.h>

static void show_usage(void)
{
	println("Usage:");
	println("ext2 -rRlL pathname device");
}

int main(int argc, char *argv[])
{
	int ret;
	int c, action = -1;
	struct ext2_desc desc;
	char pathname[1024];

	while ((c = getopt(argc, argv, "r:R:l:L:")) != EOF)
	{
		switch (c)
		{
		case 'r':
		case 'R':
			action = 'r';
			text_copy(pathname, optarg);
			break;

		case 'l':
		case 'L':
			action = 'l';
			text_copy(pathname, optarg);
			break;

		default:
			show_usage();
			return -EINVAL;
		}
	}

	if (action < 0)
	{
		error_msg("no action");
		show_usage();
		return -EINVAL;
	}

	if (optind >= argc)
	{
		error_msg("please input deivce name");
		show_usage();
		return -ENOENT;
	}

	ret = ext2_init(&desc, argv[optind]);
	if (ret < 0)
	{
		error_msg("fat_init");
		return ret;
	}

	println("volume = %s", desc.super_block.volume_name);
	println("pathname = %s", pathname);

	if (action == 'r')
	{
		char buff[1024];

		ret = ext2_read_file(&desc, pathname, buff, sizeof(buff));
		if (ret < 0)
		{
			error_msg("ext2_read_file");
			goto out_fat_deinit;
		}

		println("file \"%s\" is:", pathname);
		print_ntext(buff, ret);
		print_char('\n');
	}
	else
	{
		ret = ext2_list_directory(&desc, pathname);
	}

out_fat_deinit:
	ext2_deinit(&desc);

	return ret;
}

