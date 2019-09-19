#define CAVAN_CMD_NAME part_probe

// Fuang.Cao: Tue Dec 28 15:54:22 CST 2010

#include <cavan.h>
#include <cavan/device.h>

int main(int argc, char *argv[])
{
	int ret;

	assert(argc > 1);

	ret = reread_part_table(argv[1]);
	if (ret < 0) {
		pr_err_info("reread_part_table");
		return ret;
	}

	return 0;
}
