// Fuang.Cao <cavan.cfa@gmail.com> Thu May 12 09:51:29 CST 2011

#include <cavan.h>
#include <cavan/text.h>
#include <cavan/vfat.h>

static void show_usage(void)
{
	println("Usage:");
	println("vfat -rRlL pathname device");
}

int main(int argc, char *argv[])
{
	int ret;
	int c, action = -1;
	struct fat_info info;
	char pathname[1024];
	char volume[16];

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

	ret = fat_init(argv[optind], &info);
	if (ret < 0)
	{
		error_msg("fat_init");
		return ret;
	}

	get_fat_volume_label(&info, volume);
	println("volume = %s", volume);
	println("fat type = %s", fat_type_to_string(info.type));
	println("pathname = %s", pathname);

	if (action == 'r')
	{
		char buff[1024];

		ret = load_file(&info, pathname, buff, sizeof(buff));
		if (ret < 0)
		{
			error_msg("load_file");
			goto out_fat_deinit;
		}

		println("file \"%s\" is:", pathname);
		print_ntext(buff, ret);
		print_char('\n');
	}
	else
	{
		ret = list_directory(&info, pathname);
	}

out_fat_deinit:
	fat_deinit(&info);

	return ret;
}
