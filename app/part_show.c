#define CAVAN_CMD_NAME part_show

// Fuang.Cao <cavan.cfa@gmail.com> Thu Dec 30 20:25:01 CST 2010

#include <cavan.h>
#include <cavan/device.h>

int main(int argc, char *argv[])
{
	int ret;
	struct master_boot_sector mbs;

	assert(argc > 1);

	ret = read_master_boot_sector(argv[1], &mbs);
	if (ret < 0) {
		pr_err_info("read_master_boot_sector");
		return ret;
	}

	show_master_boot_sector(&mbs);

	return 0;
}
