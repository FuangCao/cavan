#include <cavan.h>
#include <cavan/swan_dev.h>
#include <cavan/text.h>
#include <cavan/swan_upgrade.h>

int main(int argc, char *argv[])
{
	struct partition_desc emmc_desc = {
		.major = 179,
		.minor = FIRST_MINOR,
		.path = EMMC_DEVICE,
	};

	assert(argc == 2);

	emmc_desc.path[text_len(emmc_desc.path) - 1] = argv[1][0];

	return swan_sfdisk(&emmc_desc);
}
