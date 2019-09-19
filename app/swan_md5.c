#define CAVAN_CMD_NAME swan_md5

// Fuang.Cao: Fri Dec 24 15:37:21 CST 2010

#include <cavan.h>
#include <cavan/swan_pkg.h>

int main(int argc, char *argv[])
{
	assert(argc == 2);

	return swan_check_md5sum(argv[1]);
}
