#include <cavan.h>
#include <cavan/bmp.h>

int main(int argc, char *argv[])
{
	const char *fb_dev;

	assert(argc > 1);

	if (argc == 2) {
		fb_dev = "/dev/fb0";
	} else {
		fb_dev = argv[2];
	}

	return bmp_view(argv[1], fb_dev);
}
