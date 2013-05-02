// Fuang.Cao <cavan.cfa@gmail.com> 2011-10-20 17:42:01

#include <cavan.h>
#include <cavan/cftp.h>
#include <cavan/parser.h>

#define FILE_CREATE_DATE "2011-10-20 17:42:01"

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
			0, 0, 0, 0
		},
	};
	int ret;
	int i;
	struct cavan_usb_descriptor usb_desc;
	struct cftp_descriptor cftp_desc;
	int (*cftp_client_handle)(struct cftp_descriptor *, const char *, u32, const char *, u32, size_t);
	char temp_name[512], *p_name;

	cftp_client_handle = NULL;

	while ((c = getopt_long(argc, argv, "rRgGwWSspPvVhH", long_option, &option_index)) != EOF)
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

	p_name = text_path_cat(temp_name, argv[--argc], NULL);

	for (i = optind; i < argc; i++)
	{
		int ret;

		text_basename_base(p_name, argv[i]);

		ret = cftp_client_handle(&cftp_desc, argv[i], 0, temp_name, 0, 0);
		if (ret < 0)
		{
			error_msg("cftp send or receive file \"%s\" failed", argv[i]);
			return ret;
		}
	}

	cavan_usb_deinit(&usb_desc);

	return ret;
}
