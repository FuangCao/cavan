#define CAVAN_CMD_NAME file_join

// Fuang.Cao: Tue Dec 28 23:27:38 CST 2010

#include <cavan.h>
#include <cavan/file.h>

int main(int argc, char *argv[])
{
	assert(argc > 3);

	return file_join(argv[1], argv + 2, argc - 2);
}
