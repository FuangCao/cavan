// Fuang.Cao <cavan.cfa@gmail.com> Wed May 25 16:36:32 CST 2011

#include <cavan.h>
#include <cavan/swan_dev.h>

int main(int argc, char *argv[])
{
	if (argc > 1)
	{
		return display_config(argv[1][0] - '0');
	}
	else
	{
		return display_config(0);
	}
}

