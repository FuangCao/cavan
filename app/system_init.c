#define CAVAN_CMD_NAME system_init

// Fuang.Cao <cavan.cfa@gmail.com> Fri Apr 29 13:44:02 CST 2011

#include <cavan.h>
#include <cavan/device.h>

int main(int argc, char *argv[])
{
	int ret;

	assert(argc >= 2);

	ret = system_init(argv[1], argv + 2);
	if (ret < 0) {
		pr_err_info("system_init");
		return ret;
	}

	return 0;
}

