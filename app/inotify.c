// Fuang.Cao <cavan.cfa@gmail.com> 2011-12-22 14:17:52

#include <cavan.h>
#include <cavan/inotify.h>

#define FILE_CREATE_DATE "2011-12-22 14:17:52"

enum
{
	LOCAL_COMMAND_OPTION_UNKNOWN,
	LOCAL_COMMAND_OPTION_HELP,
	LOCAL_COMMAND_OPTION_VERSION,
	LOCAL_COMMAND_OPTION_COMMAND,
};

static void show_usage(void)
{
	println("Usage:");
	println("inotify -c command pathnames");
}

static int cavan_inotify_event_handle(const char *pathname, struct inotify_event *event, void *data)
{
	char command[1024];

	text_replace_text(data, command, "{}", pathname);

	if (system(command) == 0)
	{
		return 0;
	}

	return -1;
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
			.val = LOCAL_COMMAND_OPTION_HELP,
		},
		{
			.name = "version",
			.has_arg = no_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_VERSION,
		},
		{
			.name = "command",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_COMMAND,
		},
		{
			0, 0, 0, 0
		},
	};
	int ret;
	char command[1024];
	struct cavan_inotify_descriptor desc;

	command[0] = 0;

	while ((c = getopt_long(argc, argv, "vVhHc:C:", long_option, &option_index)) != EOF)
	{
		switch (c)
		{
		case 'v':
		case 'V':
		case LOCAL_COMMAND_OPTION_VERSION:
			show_author_info();
			println(FILE_CREATE_DATE);
			return 0;

		case 'h':
		case 'H':
		case LOCAL_COMMAND_OPTION_HELP:
			show_usage();
			return 0;

		case 'c':
		case 'C':
		case LOCAL_COMMAND_OPTION_COMMAND:
			text_copy(command, optarg);
			break;

		default:
			show_usage();
			return -EINVAL;
		}
	}

	if (command[0] == 0)
	{
		pr_red_info("Please give a command");
		show_usage();
		return -EINVAL;
	}

	if (optind >= argc)
	{
		pr_red_info("Please give some to listen");
		show_usage();
		return -EINVAL;
	}

	ret = cavan_inotify_init(&desc);
	if (ret < 0)
	{
		error_msg("cavan_inotify_init");
		return ret;
	}

	while (optind < argc)
	{
		ret = cavan_inotify_register_watch(&desc, argv[optind], IN_CLOSE_WRITE, command);
		if (ret < 0)
		{
			error_msg("cavan_inotify_register_watch");
			goto out_inotify_deinit;
		}

		optind++;
	}

	ret = cavan_inotify_event_loop(&desc, cavan_inotify_event_handle);

out_inotify_deinit:
	cavan_inotify_deinit(&desc);

	return ret;
}
