// Fuang.Cao <cavan.cfa@gmail.com> 2011-10-28 12:46:16

#include <cavan.h>
#include <cavan/ftp.h>
#include <cavan/command.h>

#define FILE_CREATE_DATE "2011-10-28 12:46:16"

static void show_usage(const char *command)
{
	println("Usage: %s [OPTION] [HOME]", command);
	println("-H, -h, --help\t\t\t\t%s", cavan_help_message_help);
	println("-S, -s, --super\t\t\t\t%s", cavan_help_message_super);
	println("-D, -d, --daemon\t\t\t%s", cavan_help_message_daemon);
	println("-m, -c, --min\t\t\t\t%s", cavan_help_message_daemon_min);
	println("-M, -C, --max\t\t\t\t%s", cavan_help_message_daemon_max);
	println("-V, -v, --verbose\t\t\t%s", cavan_help_message_verbose);
	println("-p, --port PORT\t\t\t\t%s", cavan_help_message_port);
	println("-L, -l, --log [PATHNAME]\t\t%s", cavan_help_message_logfile);
	println("--url [URL]\t\t\t\t%s", cavan_help_message_url);
	println("-P, --pt, --protocol PROTOCOL\t\t%s", cavan_help_message_protocol);
}

int main(int argc, char *argv[])
{
	int c;
	int ret;
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
			.name = "root",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_ROOT,
		},
		{
			.name = "super",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_SUPER,
		},
		{
			.name = "daemon",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_DAEMON,
		},
		{
			.name = "min",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_DAEMON_MIN,
		},
		{
			.name = "max",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_DAEMON_MAX,
		},
		{
			.name = "verbose",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_VERBOSE,
		},
		{
			.name = "super",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_SUPER,
		},
		{
			.name = "port",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_PORT,
		},
		{
			.name = "log",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_LOGFILE,
		},
		{
			.name = "url",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_URL,
		},
		{
			.name = "protocol",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_PROTOCOL,
		},
		{
			.name = "pt",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_PROTOCOL,
		},
		{0, 0, 0, 0},
	};
	struct network_url *url;
	struct cavan_ftp_service *ftp;
	struct cavan_dynamic_service *service;

	service = cavan_dynamic_service_create(sizeof(struct cavan_ftp_service));
	if (service == NULL)
	{
		pr_red_info("cavan_dynamic_service_create");
		return -ENOMEM;
	}

	ftp = cavan_dynamic_service_get_data(service);

	url = &ftp->url;
	network_url_init(url, "ftp", NULL, FTP_CTRL_PORT, NULL);
	text_copy(ftp->home, "/");

	while ((c = getopt_long(argc, argv, "hHvVdDp:P:s:S:c:C:m:M:l:L:r:R:s:S:", long_option, &option_index)) != EOF)
	{
		switch (c)
		{
		case 'h':
		case 'H':
		case CAVAN_COMMAND_OPTION_HELP:
			show_usage(argv[0]);
			ret = 0;
			goto out_cavan_dynamic_service_destroy;

		case CAVAN_COMMAND_OPTION_VERSION:
			show_author_info();
			println(FILE_CREATE_DATE);
			ret = 0;
			goto out_cavan_dynamic_service_destroy;

		case 'v':
		case 'V':
		case CAVAN_COMMAND_OPTION_VERBOSE:
			service->verbose = true;
			break;

		case 'd':
		case 'D':
		case CAVAN_COMMAND_OPTION_DAEMON:
			service->as_daemon = true;
			break;

		case 'c':
		case 'm':
		case CAVAN_COMMAND_OPTION_DAEMON_MIN:
			service->min = text2value_unsigned(optarg, NULL, 10);
			break;

		case 'C':
		case 'M':
		case CAVAN_COMMAND_OPTION_DAEMON_MAX:
			service->max = text2value_unsigned(optarg, NULL, 10);
			break;

		case 'l':
		case 'L':
		case CAVAN_COMMAND_OPTION_LOGFILE:
			service->logfile = optarg;
			break;


		case 's':
		case 'S':
		case CAVAN_COMMAND_OPTION_SUPER:
			service->super_permission = text_bool_value(optarg);
			break;

		case 'p':
		case CAVAN_COMMAND_OPTION_PORT:
			url->port = text2value_unsigned(optarg, NULL, 10);
			break;

		case CAVAN_COMMAND_OPTION_URL:
			if (network_url_parse(url, optarg) == NULL)
			{
				pr_red_info("invalid url %s", optarg);
				return -EINVAL;
			}
			break;

		case 'P':
		case CAVAN_COMMAND_OPTION_PROTOCOL:
			url->protocol = optarg;
			break;


		case 'r':
		case 'R':
		case CAVAN_COMMAND_OPTION_ROOT:
			if (realpath(optarg, ftp->home) == NULL)
			{
				pr_error_info("Get directory `%s' realpath failed", optarg);
				return -ENOENT;
			}
			break;

		default:
			show_usage(argv[0]);
			ret = -EINVAL;
			goto out_cavan_dynamic_service_destroy;
		}
	}

	if (argc > optind && realpath(argv[optind], ftp->home) == NULL)
	{
		pr_error_info("realpath");
		ret = -EFAULT;
		goto out_cavan_dynamic_service_destroy;
	}

	ret = ftp_service_run(service);

out_cavan_dynamic_service_destroy:
	cavan_dynamic_service_destroy(service);
	return ret;
}
