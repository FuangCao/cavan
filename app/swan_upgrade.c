#include <cavan.h>
#include <cavan/swan_upgrade.h>
#include <cavan/device.h>
#include <cavan/swan_pkg.h>
#include <cavan/swan_dev.h>

enum swan_upgrade_option
{
	SWAN_UPGRADE_OPTION_I200,
	SWAN_UPGRADE_OPTION_MODEM,
	SWAN_UPGRADE_OPTION_I600,
	SWAN_UPGRADE_OPTION_I700,
	SWAN_UPGRADE_OPTION_WIPE_VFAT,
	SWAN_UPGRADE_OPTION_WIPE_DATA,
	SWAN_UPGRADE_OPTION_WIPE_CACHE,
	SWAN_UPGRADE_OPTION_WIPE_VENDOR,
	SWAN_UPGRADE_OPTION_CHECK_VERSION,
	SWAN_UPGRADE_OPTION_RESIZE,
	SWAN_UPGRADE_OPTION_EXCLUDE,
};

static void show_usage(void)
{
	println("Usage:");
	println("swan_upgrade -c [-f output_file] [-d file_dir]");
	println("swan_upgrade -x [-f upgrade_file] [-d output_dir]");
	println("--help");
	println("-l, -L, --label, --volume: specify vfat partition volume label");
	println("--i200, --I200: set machine to i200");
	println("--i600, --I600: set machine to i600");
	println("--i700, --I700: set machine to i700");
	println("-t, --type: i200, i600, i700");
	println("--wipe-vfat: no, yes, 0, 1");
	println("--wipe-data: no, yes, 0, 1");
	println("--wipe-cache: no, yes, 0, 1");
	println("--wipe-vendor: no, yes, 0, 1");
	println("--check-version: no, yes, 0, 1");
	println("--shrink, --resize: no, yes, 0, 1");
	println("--skip, --exclude: exclude some images");
	println("-2, --ps, --part-system: system partition size");
	println("-4, --pr, --part-recovery: recovery partition size");
	println("-5, --pu, --pd, --part-userdata, --part-data: userdata partition size");
	println("-6, --pc, --part-cache: cache partition size");
	println("-7, --pv, --part-vendor: vendor partition size");
}

int main(int argc, char *argv[])
{
	int option_index;
	int c;
	char dir_path[128];
	char file_path[128];
	int (*handle)(const char *, const char *);
	struct option long_option[] =
	{
		{
			.name = "i200",
			.has_arg = no_argument,
			.flag = NULL,
			.val = SWAN_UPGRADE_OPTION_I200,
		},
		{
			.name = "I200",
			.has_arg = no_argument,
			.flag = NULL,
			.val = SWAN_UPGRADE_OPTION_I200,
		},
		{
			.name = "i600",
			.has_arg = no_argument,
			.flag = NULL,
			.val = SWAN_UPGRADE_OPTION_I600,
		},
		{
			.name = "I600",
			.has_arg = no_argument,
			.flag = NULL,
			.val = SWAN_UPGRADE_OPTION_I600,
		},
		{
			.name = "i700",
			.has_arg = no_argument,
			.flag = NULL,
			.val = SWAN_UPGRADE_OPTION_I700,
		},
		{
			.name = "I700",
			.has_arg = no_argument,
			.flag = NULL,
			.val = SWAN_UPGRADE_OPTION_I700,
		},
		{
			.name = "modem",
			.has_arg = no_argument,
			.flag = NULL,
			.val = SWAN_UPGRADE_OPTION_MODEM,
		},
		{
			.name = "wipe-vfat",
			.has_arg = optional_argument,
			.flag = NULL,
			.val = SWAN_UPGRADE_OPTION_WIPE_VFAT,
		},
		{
			.name = "wipe-data",
			.has_arg = optional_argument,
			.flag = NULL,
			.val = SWAN_UPGRADE_OPTION_WIPE_DATA,
		},
		{
			.name = "wipe-cache",
			.has_arg = optional_argument,
			.flag = NULL,
			.val = SWAN_UPGRADE_OPTION_WIPE_CACHE,
		},
		{
			.name = "wipe-vendor",
			.has_arg = optional_argument,
			.flag = NULL,
			.val = SWAN_UPGRADE_OPTION_WIPE_VENDOR,
		},
		{
			.name = "check-version",
			.has_arg = optional_argument,
			.flag = NULL,
			.val = SWAN_UPGRADE_OPTION_CHECK_VERSION,
		},
		{
			.name = "exclude",
			.has_arg = required_argument,
			.flag = NULL,
			.val = SWAN_UPGRADE_OPTION_EXCLUDE,
		},
		{
			.name = "skip",
			.has_arg = required_argument,
			.flag = NULL,
			.val = SWAN_UPGRADE_OPTION_EXCLUDE,
		},
		{
			.name = "resize",
			.has_arg = required_argument,
			.flag = NULL,
			.val = SWAN_UPGRADE_OPTION_RESIZE,
		},
		{
			.name = "shrink",
			.has_arg = required_argument,
			.flag = NULL,
			.val = SWAN_UPGRADE_OPTION_RESIZE,
		},
		{
			.name = "label",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 'l',
		},
		{
			.name = "volume",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 'l',
		},
		{
			.name = "type",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 't',
		},
		{
			.name = "help",
			.has_arg = no_argument,
			.flag = NULL,
			.val = 'h',
		},
		{
			.name = "part-system",
			.has_arg = required_argument,
			.flag = NULL,
			.val = '2',
		},
		{
			.name = "ps",
			.has_arg = required_argument,
			.flag = NULL,
			.val = '2',
		},
		{
			.name = "part-recovery",
			.has_arg = required_argument,
			.flag = NULL,
			.val = '4',
		},
		{
			.name = "pr",
			.has_arg = required_argument,
			.flag = NULL,
			.val = '4',
		},
		{
			.name = "part-userdata",
			.has_arg = required_argument,
			.flag = NULL,
			.val = '5',
		},
		{
			.name = "pu",
			.has_arg = required_argument,
			.flag = NULL,
			.val = '5',
		},
		{
			.name = "part-data",
			.has_arg = required_argument,
			.flag = NULL,
			.val = '5',
		},
		{
			.name = "pd",
			.has_arg = required_argument,
			.flag = NULL,
			.val = '5',
		},
		{
			.name = "part-cache",
			.has_arg = required_argument,
			.flag = NULL,
			.val = '6',
		},
		{
			.name = "pc",
			.has_arg = required_argument,
			.flag = NULL,
			.val = '6',
		},
		{
			.name = "part-vendor",
			.has_arg = required_argument,
			.flag = NULL,
			.val = '7',
		},
		{
			.name = "pv",
			.has_arg = required_argument,
			.flag = NULL,
			.val = '7',
		},
		{
			0, 0, 0, 0
		},
	};

	println_green("Upgrade Program \"" __FILE__ "\" Build Date: " __DATE__  " "  __TIME__);

	if (argc == 1)
	{
		return auto_upgrade(NULL);
	}

	if (argc == 2 && argv[1][0] != '-')
	{
		if (file_test(argv[1], "f") == 0)
		{
			return auto_upgrade(argv[1]);
		}
	}

	dir_path[0] = 0;
	file_path[0] = 0;
	handle = NULL;

	while ((c = getopt_long(argc, argv, "vVcCuUxXSsl:L:f:F:d:D:hHt:T:2:4:5:6:7:", long_option, &option_index)) != EOF)
	{
		switch (c)
		{
		case 'l':
		case 'L':
			strcpy(swan_vfat_volume, optarg);
			println("new label is: %s", optarg);
			break;

		case SWAN_UPGRADE_OPTION_I200:
			swan_machine_type = SWAN_BOARD_I200;
			break;

		case SWAN_UPGRADE_OPTION_I600:
			swan_machine_type = SWAN_BOARD_I600;
			break;

		case SWAN_UPGRADE_OPTION_MODEM:
			swan_machine_type = SWAN_BOARD_I200;
			swan_package_flags |= SWAN_FLAGS_MODEM_UPGRADE;
			break;

		case SWAN_UPGRADE_OPTION_I700:
			swan_machine_type = SWAN_BOARD_I700;
			break;

		case SWAN_UPGRADE_OPTION_WIPE_VFAT:
			if (text_bool_value(optarg))
			{
				println_blue("Format VFAT partition");
				swan_mkfs_mask |= MKFS_MASK_VFAT;
			}
			else
			{
				println_blue("Don't format VFAT partition");
				swan_mkfs_mask &= ~MKFS_MASK_VFAT;
			}
			break;

		case SWAN_UPGRADE_OPTION_WIPE_DATA:
			if (text_bool_value(optarg))
			{
				println_blue("Format userdata partition");
				swan_mkfs_mask |= MKFS_MASK_USERDATA;
			}
			else
			{
				println_blue("Don't format userdata partition");
				swan_exclude_images[swan_exclude_image_count++] = SWAN_IMAGE_USERDATA;
				swan_mkfs_mask &= ~MKFS_MASK_USERDATA;
			}
			break;

		case SWAN_UPGRADE_OPTION_WIPE_CACHE:
			if (text_bool_value(optarg))
			{
				println_blue("Format cache partition");
				swan_mkfs_mask |= MKFS_MASK_CACHE;
			}
			else
			{
				println_blue("Don't format cache partition");
				swan_mkfs_mask &= ~MKFS_MASK_CACHE;
			}
			break;

		case SWAN_UPGRADE_OPTION_WIPE_VENDOR:
			if (text_bool_value(optarg))
			{
				println_blue("Format vendor partition");
				swan_mkfs_mask |= MKFS_MASK_VENDOR;
			}
			else
			{
				println_blue("Don't format vendor partition");
				swan_mkfs_mask &= ~MKFS_MASK_VENDOR;
			}
			break;

		case SWAN_UPGRADE_OPTION_CHECK_VERSION:
			if (text_bool_value(optarg))
			{
				println_blue("Open version check");
				swan_upgrade_flags |= UPGRADE_FLAG_CHECK_VERSION;
			}
			else
			{
				println_blue("Close version check");
				swan_upgrade_flags &= ~UPGRADE_FLAG_CHECK_VERSION;
			}
			break;

		case SWAN_UPGRADE_OPTION_EXCLUDE:
			swan_exclude_images[swan_exclude_image_count] = get_swan_image_type_by_name(optarg);
			if (swan_exclude_images[swan_exclude_image_count] == SWAN_IMAGE_UNKNOWN)
			{
				error_msg("Unknown image type \"%s\"", optarg);
				return -EINVAL;
			}
			else
			{
				swan_exclude_image_count++;
			}
			println_blue("Exclude image: \"%s\"", optarg);
			break;

		case SWAN_UPGRADE_OPTION_RESIZE:
			if (text_bool_value(optarg))
			{
				println_blue("Don't shrink images when package");
				swan_need_shrink = 1;
			}
			else
			{
				println_blue("Shrink images when package");
				swan_need_shrink = 0;
			}
			break;

		case 't':
		case 'T':
			if (text_rhcmp(optarg, "i200") == 0 || text_rhcmp(optarg, "I200") == 0)
			{
				swan_machine_type = SWAN_BOARD_I200;
			}
			else if (text_rhcmp(optarg, "i600") == 0 || text_rhcmp(optarg, "I600") == 0)
			{
				swan_machine_type = SWAN_BOARD_I600;
			}
			else if (text_rhcmp(optarg, "i700") == 0 || text_rhcmp(optarg, "I700") == 0)
			{
				swan_machine_type = SWAN_BOARD_I700;
			}
			else
			{
				swan_machine_type = SWAN_BOARD_UNKNOWN;
			}
			break;

		case 'f':
		case 'F':
			strcpy(file_path, optarg);
			break;

		case 'd':
		case 'D':
			strcpy(dir_path, optarg);
			break;

		case 'c':
		case 'C':
			handle = package;
			break;

		case 'u':
		case 'U':
			handle = upgrade;
			break;

		case 's':
		case 'S':
			handle = upgrade_simple;
			break;

		case 'x':
		case 'X':
			handle = unpack;
			break;

		case 'v':
		case 'V':
			println("Program Build Date: %s %s", __DATE__, __TIME__);
			return 0;

		case 'h':
		case 'H':
			show_usage();
			return 0;

		case '2':
			swan_emmc_part_table.system_size = text2size_mb(optarg);
			break;

		case '4':
			swan_emmc_part_table.recovery_size = text2size_mb(optarg);
			break;

		case '5':
			swan_emmc_part_table.userdata_size = text2size_mb(optarg);
			break;

		case '6':
			swan_emmc_part_table.cache_size = text2size_mb(optarg);
			break;

		case '7':
			swan_emmc_part_table.vendor_size = text2size_mb(optarg);
			break;

		default:
			error_msg("Unknown option %c", c);
			show_usage();
			return -EINVAL;
		}
	}

	if (handle == NULL)
	{
		error_msg("No function handle this action");
		show_usage();
		return -EINVAL;
	}

	if (file_path[0] == 0)
	{
		for (option_index = optind; option_index < argc; option_index++)
		{
			if (file_test(argv[option_index], "f") == 0)
			{
				strcpy(file_path, argv[option_index]);
			}
		}

		if (file_path[0] == 0)
		{
			strcpy(file_path, UPGRADE_FILE_NAME);
		}
	}

	if (dir_path[0] == 0)
	{
		for (option_index = optind; option_index < argc; option_index++)
		{
			if (file_test(argv[option_index], "d") == 0 || file_test(argv[option_index], "e") < 0)
			{
				strcpy(dir_path, argv[option_index]);
			}
		}

		if (dir_path[0] == 0)
		{
			strcpy(dir_path, ".");
		}
	}

	return handle(file_path, dir_path);
}
