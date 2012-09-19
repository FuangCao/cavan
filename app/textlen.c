// Fuang.Cao: Thu Dec 23 14:06:39 CST 2010

#include <cavan.h>

int main(int argc, char *argv[])
{
	assert(argc == 2);

#if __WORDSIZE == 64
	println("text length = %ld", strlen(argv[1]));
#else
	println("text length = %d", strlen(argv[1]));
#endif

	return 0;
}
