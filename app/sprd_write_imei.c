/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Sun Jan 27 22:03:56 CST 2013
 */

#include <cavan.h>
#include <cavan/math.h>
#include <cavan/sprd_diag.h>

#define FILE_CREATE_DATE "2013-01-27 22:03:56"

enum
{
	LOCAL_COMMAND_OPTION_UNKNOWN,
	LOCAL_COMMAND_OPTION_HELP,
	LOCAL_COMMAND_OPTION_VERSION,
	LOCAL_COMMAND_OPTION_IMEI1,
	LOCAL_COMMAND_OPTION_IMEI2,
	LOCAL_COMMAND_OPTION_IMEI3,
	LOCAL_COMMAND_OPTION_IMEI4,
	LOCAL_COMMAND_OPTION_BT_MAC,
	LOCAL_COMMAND_OPTION_WIFI_MAC,
	LOCAL_COMMAND_OPTION_DEVICE
};

static void show_usage(const char *command)
{
	println("Usage: %s [option] [devpath]", command);
	println("--help, -h, -H\t\tshow this help");
	println("--version, -v, -V\tshow version");
	println("--imei, --imei1\t\twrite imei1");
	println("--imei2\t\t\twrite imei2");
	println("--imei3\t\t\twrite imei3");
	println("--imei4\t\t\twrite imei4");
	println("--bt, --bt-mac\t\twrite bluetooth mac address");
	println("--wifi, --wifi-mac\twrite wifi mac address");
	println("--dev, -d, -D\t\tvbpipe device path");
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
			.name = "imei",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_IMEI1,
		},
		{
			.name = "imei1",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_IMEI1,
		},
		{
			.name = "imei2",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_IMEI2,
		},
		{
			.name = "imei3",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_IMEI3,
		},
		{
			.name = "imei4",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_IMEI4,
		},
		{
			.name = "wifi",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_WIFI_MAC,
		},
		{
			.name = "wifi-mac",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_WIFI_MAC,
		},
		{
			.name = "bt",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_BT_MAC,
		},
		{
			.name = "bt-mac",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_BT_MAC,
		},
		{
			.name = "dev",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_DEVICE,
		},
		{
			0, 0, 0, 0
		},
	};
	int fd;
	int ret;
	u8 mask = 0;
	char devpath[1024] = "/dev/vbpipe0";
	struct sprd_diag_imei_data imei;

	while ((c = getopt_long(argc, argv, "vVhHd:D:", long_option, &option_index)) != EOF)
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
			show_usage(argv[0]);
			return 0;

		case LOCAL_COMMAND_OPTION_IMEI1:
			mask |= SPRD_DIAG_MASK_IMEI1;
			sprd_diag_text2imei(optarg, imei.imei1, sizeof(imei.imei1));
			break;

		case LOCAL_COMMAND_OPTION_IMEI2:
			mask |= SPRD_DIAG_MASK_IMEI2;
			sprd_diag_text2imei(optarg, imei.imei2, sizeof(imei.imei2));
			break;

		case LOCAL_COMMAND_OPTION_IMEI3:
			mask |= SPRD_DIAG_MASK_IMEI3;
			sprd_diag_text2imei(optarg, imei.imei3, sizeof(imei.imei3));
			break;

		case LOCAL_COMMAND_OPTION_IMEI4:
			mask |= SPRD_DIAG_MASK_IMEI4;
			sprd_diag_text2imei(optarg, imei.imei4, sizeof(imei.imei4));
			break;

		case LOCAL_COMMAND_OPTION_WIFI_MAC:
			mask |= SPRD_DIAG_MASK_WIFI_MAC;
			sprd_diag_text2mac(optarg, imei.wifi_mac, sizeof(imei.wifi_mac));
			break;

		case LOCAL_COMMAND_OPTION_BT_MAC:
			mask |= SPRD_DIAG_MASK_BT_MAC;
			sprd_diag_text2mac(optarg, imei.bt_mac, sizeof(imei.bt_mac));
			break;

		case 'd':
		case 'D':
		case LOCAL_COMMAND_OPTION_DEVICE:
			text_copy(devpath, optarg);
			break;

		default:
			show_usage(argv[0]);
			return -EINVAL;
		}
	}

	if (optind < argc)
	{
		text_copy(devpath, argv[optind]);
	}

	fd = open(devpath, O_RDWR | O_SYNC);
	if (fd < 0)
	{
		pr_error_info("open %s", devpath);
		return fd;
	}

	if (mask)
	{
		println("mask = 0x%02x", mask);
		sprd_diag_show_imei(&imei);

		ret = sprd_diag_write_imei(fd, &imei, mask);
		if (ret < 0)
		{
			pr_red_info("sprd_diag_write_imei");
		}
	}
	else
	{
		ret = sprd_diag_read_imei(fd, &imei, 0xFF);
		if (ret < 0)
		{
			pr_red_info("sprd_diag_read_imei");
		}
		else
		{
			sprd_diag_show_imei(&imei);
		}
	}

	close(fd);

	return ret;
}
