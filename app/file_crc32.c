#include <cavan.h>
#include <cavan/file.h>
#include <cavan/text.h>

int main(int argc, char *argv[])
{
	int ret;
	u32 crc;

	assert(argc == 2 || argc == 3);

	crc = 0;

	if (argc == 2) {
		ret = file_crc32(argv[1], &crc);
		if (ret < 0) {
			error_msg("file_crc32");
			return -1;
		}
	} else {
		ret = file_ncrc32(argv[1], text2size(argv[2], NULL), &crc);
		if (ret < 0) {
			error_msg("file_ncrc32");
			return -1;
		}
	}

	printf("crc32 = 0x%08x\n", crc);

	return 0;
}
