// Fuang.Cao <cavan.cfa@gmail.com> Wed Jun 22 14:31:43 CST 2011

#include <cavan.h>
#include <cavan/text.h>

int main(int argc, char *argv[])
{
	char buff[1024];

	assert(argc == 2);

	if (to_abs_path_base(argv[1], buff, sizeof(buff)) == NULL) {
		return -EFAULT;
	}

	print_string(buff);

	return 0;
}
