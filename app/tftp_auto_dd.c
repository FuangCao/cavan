// Fuang.Cao <cavan.cfa@gmail.com> Sun May 15 12:23:41 CST 2011

#include <cavan.h>
#include <cavan/uevent.h>
#include <cavan/device.h>
#include <cavan/dd.h>
#include <cavan/file.h>
#include <cavan/text.h>
#include <cavan/tftp.h>
#include <cavan/image.h>

static void show_usage(void)
{
	println("Usage:");
	println("tftp_auto_dd [--ip=ip] [--port=port] [-ukrscd]");
	println("-u: u-boot.bin");
	println("-k: uImage");
	println("-r: uramdisk.img");
	println("-s: system.img");
	println("-c: recovery.img");
	println("-d: userdata.img");
}

int main(int argc, char *argv[])
{
	int c;
	int ret;
	int delay;
	const char *hostname;
	u16 port = 0;
	int option_index;
	struct option long_options[] =
	{
		{
			.name = "ip",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 0,
		},
		{
			.name = "port",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 1,
		},
		{
			.name = "delay",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 2,
		},
		{
			.name = "help",
			.has_arg = no_argument,
			.flag = NULL,
			.val = 'h',
		},
		{
			0, 0, 0, 0
		},
	};
	struct dd_desc descs[6], *p, *end_p;
	struct uevent_desc udesc;

	hostname = NULL;
	end_p = descs + ARRAY_SIZE(descs);
	p = descs;
	delay = 0;

	while ((c = getopt_long(argc, argv, "U:u:K:k:R:r:S:s:C:c:D:d:hH", long_options, &option_index)) != EOF)
	{
		switch (c)
		{
			case 0:
				hostname = optarg;
				break;

			case 1:
				port = text2value(optarg, NULL, 10);
				break;

			case 2:
				delay = text2value(optarg, NULL, 10);
				break;

			case 'u':
			case 'U':
				text_copy(p->in, optarg);
				text_copy(p->out, "/dev/sdb");
				p->bs = 1;
				p->seek = UBOOT_OFFSET;
				p->skip = UBOOT_PADDING_SIZE;
				p->count = 0;
				p++;
				break;

			case 'k':
			case 'K':
				text_copy(p->in, optarg);
				text_copy(p->out, "/dev/sdb");
				p->bs = 1;
				p->seek = UIMAGE_OFFSET;
				p->skip = 0;
				p->count = 0;
				p++;
				break;

			case 'r':
			case 'R':
				text_copy(p->in, optarg);
				text_copy(p->out, "/dev/sdb");
				p->bs = 1;
				p->seek = RAMDISK_OFFSET;
				p->skip = 0;
				p->count = 0;
				p++;
				break;

			case 's':
			case 'S':
				text_copy(p->in, optarg);
				text_copy(p->out, "/dev/sdb2");
				p->bs = 0;
				p->seek = 0;
				p->skip = 0;
				p->count = 0;
				p++;
				break;

			case 'c':
			case 'C':
				text_copy(p->in, optarg);
				text_copy(p->out, "/dev/sdb4");
				p->bs = 0;
				p->seek = 0;
				p->skip = 0;
				p->count = 0;
				p++;
				break;

			case 'd':
			case 'D':
				text_copy(p->in, optarg);
				text_copy(p->out, "/dev/sdb5");
				p->bs = 0;
				p->seek = 0;
				p->skip = 0;
				p->count = 0;
				p++;
				break;

			case 'h':
			case 'H':
				show_usage();
				return 0;

			default:
				show_usage();
				return -EINVAL;
		}
	}

	end_p = p;

	if (hostname == NULL)
	{
		hostname = cavan_get_server_hostname();
	}

	if (port == 0)
	{
		port = cavan_get_server_port(TFTP_DD_DEFAULT_PORT);
	}

	println("ip address = %s, port = %d, image count = %" PRINT_FORMAT_SIZE, hostname, port, end_p - descs);

	ret = uevent_init(&udesc);
	if (ret < 0)
	{
		error_msg("uevent_init");
		return ret;
	}

	while (1)
	{
		char devpath[1024];

		ret = get_disk_add_uevent(&udesc);
		if (ret < 0)
		{
			error_msg("get_disk_add_uevent");
			break;
		}

		if (uevent_get_propery_devname(&udesc, devpath) == NULL)
		{
			continue;
		}

		if (file_test(devpath, "b") < 0)
		{
			continue;
		}

		println("disk \"%s\" added", devpath);

		visual_ssleep(delay);

		ret = 0;

		for (p = descs; p < end_p; p++)
		{
			mem_copy(p->out + 5, devpath + 5, 3);

			if (file_test(p->out, "b") < 0)
			{
				ret = -ENODEV;
				error_msg("\"%s\" is not a block device", p->out);
				break;
			}

			umount_partition(p->out, MNT_DETACH);
			ret |= tftp_client_receive_file(hostname, port, p->in, p->out, \
					p->skip * p->bs, p->seek * p->bs, p->count * p->bs);
		}

		umount_device(devpath, MNT_DETACH);

		if (ret < 0)
		{
			println_red("Failed");
		}
		else
		{
			println_green("Burn success, you can remove the disk now");
		}
	}

	uevent_deinit(&udesc);

	return 0;
}

