#include <cavan.h>
#include <cavan/file.h>
#include <cavan/text.h>

int main(int argc, char *argv[])
{
	assert (argc > 1);

	if (argc < 3) {
		return file_show(argv[1], 0);
	} else {
		return file_show(argv[1], text2size(argv[2], NULL));
	}
}
