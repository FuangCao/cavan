#define CAVAN_CMD_NAME shrink

#include <cavan.h>
#include <cavan/image.h>

int main(int argc, char *argv[])
{
	assert(argc == 2);

	return image_shrink(argv[1]);
}
