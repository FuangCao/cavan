// Fuang.Cao <cavan.cfa@gmail.com> Mon Jul  4 10:55:46 CST 2011

#include <cavan.h>
#include <cavan/calculator.h>

int main(int argc, char *argv[])
{
	int ret;
	double result;
	char buff[1024];

	assert(argc >= 2);

	text_cat2(buff, argv + 1, argc - 1);

	ret = complete_calculation(buff, &result);
	if (ret < 0)
	{
		error_msg("simple_calculator");
		return ret;
	}

	println("%lf", result);

	return 0;
}
