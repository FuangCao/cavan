// Fuang.Cao: Thu Dec 23 14:06:39 CST 2010

#include <cavan.h>

int main(int argc, char *argv[])
{
	assert(argc == 2);

	println("text length = %d", strlen(argv[1]));

	return 0;
}
