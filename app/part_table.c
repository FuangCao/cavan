#define CAVAN_CMD_NAME part_table

// Fuang.Cao: Tue Dec 28 16:00:41 CST 2010

#include <cavan.h>
#include <cavan/dd.h>

int main(int argc, char *argv[])
{
	const char *src_file, *dest_file;

	assert(argc > 2 && argv[1][0] == '-');

	switch (argv[1][1]) {
	case 'b':
	case 'B':
	case 's':
	case 'S':
	case 'r':
	case 'R':
		assert(argc == 4);
		src_file = argv[2];
		dest_file = argv[3];
		break;
	case 'z':
	case 'Z':
		assert(argc == 3);
		src_file = "/dev/zero";
		dest_file = argv[2];
		break;
	default:
		pr_err_info("unknown option");
		return -1;
	}

	return cavan_dd(src_file, dest_file, 0, 0, 512);
}
