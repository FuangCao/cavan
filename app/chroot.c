// Fuang.Cao <cavan.cfa@gmail.com> Fri Apr 29 13:58:03 CST 2011

#include <cavan.h>
#include <cavan/device.h>

int main(int argc, char *argv[])
{
	int ret;

	assert(argc >= 2);

	ret = chroot(argv[1]);
	if (ret < 0) {
		pr_err_info("chroot to %s", argv[1]);
		return ret;
	}

	if (argc == 2) {
		system_init(NULL, argv + 2);
	} else {
		system_init(argv[2], argv + 3);
	}

	return 0;
}
