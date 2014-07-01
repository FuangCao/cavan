// Fuang.Cao <cavan.cfa@gmail.com> 2011-12-23 16:52:20

#include <cavan.h>
#include <cavan/ecc.h>
#include <cavan/command.h>

#define FILE_CREATE_DATE "2011-12-23 16:52:20"

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
			.val = CAVAN_COMMAND_OPTION_HELP,
		},
		{
			.name = "version",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_VERSION,
		},
		{0, 0, 0, 0},
	};
	u8 ecc_table[256];

	while ((c = getopt_long(argc, argv, "vVhH", long_option, &option_index)) != EOF)
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

		default:
			show_usage();
			return -EINVAL;
		}
	}

	cavan_ecc_show_ecc_table(cavan_ecc_build_ecc_table(ecc_table, NELEM(ecc_table)), NELEM(ecc_table));

	return 0;
}
