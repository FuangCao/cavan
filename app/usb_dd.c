// Fuang.Cao <cavan.cfa@gmail.com> 2011-10-20 14:01:08

#include <cavan.h>
#include <cavan/cftp.h>
#include <cavan/parser.h>

#define FILE_CREATE_DATE "2011-10-20 14:01:08"

static void show_usage(void)
{
	println("Usage:");
	println("usb_dd -w if=local_file of=remote_file");
	println("usb_dd -r if=remote_file of=local_file");
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
	int i;
	struct cavan_usb_descriptor usb_desc;
	struct cftp_descriptor cftp_desc;
	u32 bs, seek, skip, count;
	char input_file[128];
	char output_file[128];
	int (*cftp_client_handle)(struct cftp_descriptor *, const char *, u32, const char *, u32, size_t);

	cftp_client_handle = NULL;

	while ((c = getopt_long(argc, argv, "rRgGsSpPwWvVhH", long_option, &option_index)) != EOF)
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
		case 'g':
		case 'G':
			cftp_client_handle = cftp_client_receive_file;
			break;

		case 'w':
		case 'W':
		case 's':
		case 'S':
		case 'p':
		case 'P':
			cftp_client_handle = cftp_client_send_file;
			break;

		default:
			show_usage();
			return -EINVAL;
		}
	}

	if (cftp_client_handle == NULL)
	{
		error_msg("Please input transfer direction");
		show_usage();
		return -EINVAL;
	}

	bs = 1;
	seek = skip = count = 0;
	input_file[0] = output_file[0] = 0;

	for (i = optind; i < argc; i++)
	{
		char *p;

		parse_parameter(argv[i]);

		c = para_option[0];
		p = para_option + 1;

		switch (c)
		{
		case 'i':
			if (strcmp(p, "f") == 0)
			{
				strcpy(input_file, para_value);
			}
			else
			{
				goto out_unknown_option;
			}
			break;
		case 'o':
			if (strcmp(p, "f") == 0)
			{
				strcpy(output_file, para_value);
			}
			else
			{
				goto out_unknown_option;
			}
			break;
		case 'b':
			if (strcmp(p, "s") == 0)
			{
				bs = text2size(para_value, NULL);
			}
			else
			{
				goto out_unknown_option;
			}
			break;
		case 's':
			if (strcmp(p, "kip") == 0)
			{
				skip = text2size(para_value, NULL);
			}
			else if (strcmp(p, "eek") == 0)
			{
				seek = text2size(para_value, NULL);
			}
			else
			{
				goto out_unknown_option;
			}
			break;
		case 'c':
			if (strcmp(p, "ount") == 0)
			{
				count = text2size(para_value, NULL);
			}
			else
			{
				goto out_unknown_option;
			}
			break;
		default:
			goto out_unknown_option;
		}
	}

	if (input_file[0] == 0 || output_file[0] == 0)
	{
		error_msg("must specify if and of option");
		return -EINVAL;
	}

	system_command("killall adb");

	ret = cavan_find_usb_device(NULL, &usb_desc);
	if (ret < 0)
	{
		error_msg("cavan_find_usb_device");
		return ret;
	}

	cftp_descriptor_init(&cftp_desc);
	cftp_desc.data.type_void = &usb_desc;
	cftp_desc.send = cftp_usb_send_data;
	cftp_desc.receive = cftp_usb_receive_data;
	cftp_desc.max_xfer_length = CAVAN_USB_MAX_XFER_SIZE;

	ret = cftp_client_handle(&cftp_desc, input_file, skip * bs, output_file, seek * bs, count * bs);
	if (ret < 0)
	{
		pr_red_info("cftp_client_send_file failed");
	}

	cavan_usb_uninit(&usb_desc);

	return ret;

out_unknown_option:
	error_msg("unknown option \"%s\"", para_option);
	show_usage();
	return -EINVAL;
}
