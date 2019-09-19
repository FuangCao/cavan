#define CAVAN_CMD_NAME bmp_view

#include <cavan.h>
#include <cavan/bmp.h>

int main(int argc, char *argv[])
{
	assert(argc > 1);

	return cavan_fb_bmp_view2(argv[1], argc > 2 ? argv[2] : NULL);
}
