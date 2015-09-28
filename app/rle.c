// Fuang.Cao <cavan.cfa@gmail.com> Tue Jan 18 16:17:02 CST 2011

#include <cavan.h>
#include <cavan/rle.h>
#include <cavan/file.h>

int main(int argc, char *argv[])
{
	int ret;

	assert(argc == 4 && argv[1][0] == '-');

	switch (argv[1][1]) {
	case 'x':
		ret = unrle_to(argv[2], argv[3]);
		if (ret < 0) {
			error_msg("inrle_to");
			return ret;
		}
		break;
	case 'c':
		ret = rle_to(argv[2], argv[3]);
		if (ret < 0) {
			error_msg("rle_to");
			return ret;
		}
		break;
	default:
		error_msg("argument error");
		return -1;
	}


	return 0;
}
