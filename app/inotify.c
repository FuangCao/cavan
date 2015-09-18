// Fuang.Cao <cavan.cfa@gmail.com> 2011-12-22 14:17:52

#include <cavan.h>
#include <cavan/inotify.h>
#include <cavan/command.h>

#define FILE_CREATE_DATE "2011-12-22 14:17:52"

static void show_usage(void)
{
	println("Usage:");
	println("inotify -c command pathnames");
}

static int cavan_inotify_event_handle(struct cavan_inotify_descriptor *desc, struct cavan_inotify_watch *watch, struct inotify_event *event)
{
	char buff[1024];
	const char *pathname;
	const char *command = desc->private_data;

	if (event->len > 0)
	{
		pathname = buff;
		text_path_cat(buff, sizeof(buff), watch->pathname, event->name);
	}
	else
	{
		pathname = watch->pathname;
	}

	if (command && command[0])
	{
		setenv("CAVAN_INOTIFY_PATH", pathname, 1);

		if (system(command) != 0)
		{
			return -EFAULT;
		}
	}
	else
	{
		println("%s", pathname);
	}

	return 0;
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
			.val = CAVAN_COMMAND_OPTION_HELP,
		},
		{
			.name = "version",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_VERSION,
		},
		{
			.name = "command",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_COMMAND,
		},
		{
			0, 0, 0, 0
		},
	};
	int ret;
	const char *command = NULL;
	struct cavan_inotify_descriptor desc;

	while ((c = getopt_long(argc, argv, "vVhHc:C:", long_option, &option_index)) != EOF)
	{
		switch (c)
		{
		case 'v':
		case 'V':
		case CAVAN_COMMAND_OPTION_VERSION:
			show_author_info();
			println(FILE_CREATE_DATE);
			return 0;

		case 'h':
		case 'H':
		case CAVAN_COMMAND_OPTION_HELP:
			show_usage();
			return 0;

		case 'c':
		case 'C':
		case CAVAN_COMMAND_OPTION_COMMAND:
			command = optarg;
			break;

		default:
			show_usage();
			return -EINVAL;
		}
	}

	if (optind >= argc)
	{
		pr_red_info("Please give some to listen");
		show_usage();
		return -EINVAL;
	}

	desc.handle = cavan_inotify_event_handle;

	ret = cavan_inotify_init(&desc, __UNCONST(command));
	if (ret < 0)
	{
		error_msg("cavan_inotify_init");
		return ret;
	}

	while (optind < argc)
	{
		ret = cavan_inotify_register_watch(&desc, argv[optind], IN_CLOSE_WRITE);
		if (ret < 0)
		{
			error_msg("cavan_inotify_register_watch");
			goto out_inotify_deinit;
		}

		optind++;
	}

	ret = cavan_inotify_event_loop(&desc);

out_inotify_deinit:
	cavan_inotify_deinit(&desc);

	return ret;
}
