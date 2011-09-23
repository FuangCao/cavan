#include <cavan.h>
#include <cavan/file.h>
#include <cavan/text.h>

int main(int argc, char *argv[])
{
	assert(argc == 3);

	file_show(argv[1], text2size(argv[2]));

	return 0;
}
