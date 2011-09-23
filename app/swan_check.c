// Fuang.Cao: Tue Dec 21 17:04:41 CST 2010

#include <cavan.h>
#include <cavan/swan_dev.h>

int main(int argc, char *argv[])
{
	char dev_path[1024];

	assert (argc == 2);

	sprintf(dev_path, "/dev/mmcblk%c", argv[1][0]);

	return swan_check(dev_path, "245");
}
