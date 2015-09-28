// Fuang.Cao <cavan.cfa@gmail.com> Tue Jan 18 13:22:09 CST 2011

#include <cavan.h>

int main(int argc, char *argv[])
{
	assert(argc == 3);

	if (argv[2][0] == '0') {
		return switch2graph_mode(argv[1]);
	} else {
		return switch2text_mode(argv[1]);
	}
}
