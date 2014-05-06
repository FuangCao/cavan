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
	const char *pathname = NULL;
	struct cavan_ext2_file *fp;

	while ((c = getopt(argc, argv, "r:R:l:L:")) != EOF)
	{
		switch (c)
		{
		case 'r':
		case 'R':
			action = 'r';
			pathname = optarg;
			break;

		case 'l':
		case 'L':
			action = 'l';
			pathname = optarg;
			break;

		default:
			show_usage();
			return -EINVAL;
		}
	}

	if (action < 0 || pathname == NULL)
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

	fp = cavan_ext2_open_file(&desc, pathname, O_RDONLY, 0777);
	if (fp == NULL)
	{
		ret = -ENOENT;
		pr_red_info("cavan_ext2_open_file");
		goto out_fat_deinit;
	}

	if (action == 'r')
	{
		char *data = alloca(fp->inode.size);

		ret = cavan_ext2_read_file(fp, data, fp->inode.size);
		if (ret < 0)
		{
			pr_red_info("cavan_ext2_read_file");
			goto out_cavan_ext2_close_file;
		}

		println("file \"%s\" is %d:", pathname, ret);
		print_ntext(data, ret);
		print_char('\n');
	}
	else
	{
		ret = 0;
	}

out_cavan_ext2_close_file:
	cavan_ext2_close_file(fp);
out_fat_deinit:
	ext2_deinit(&desc);

	return ret;
}

