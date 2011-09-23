#include <cavan.h>
#include <cavan/swan_dev.h>
#include <cavan/text.h>
#include <cavan/swan_upgrade.h>

int main(int argc, char *argv[])
{
	int ret;
	struct partition_desc emmc_desc = {
		.major = 179,
		.minor = FIRST_MINOR,
		.path = EMMC_DEVICE,
	};
	struct swan_emmc_partition_table part_table;

	assert(argc > 1);

	emmc_desc.path[text_len(emmc_desc.path) - 1] = argv[1][0];

	if (argc > 2)
	{
		ret = sscanf(argv[2], "%d,%d,%d,%d,%d", &part_table.system_size, &part_table.recovery_size, &part_table.userdata_size, &part_table.cache_size, &part_table.vendor_size);
		if (ret < 4 || ret > 5)
		{
			error_msg("argument fault");
			return -EINVAL;
		}

		if (ret == 4)
		{
			part_table.vendor_size = 0;
		}
	}
	else
	{
		get_default_emmc_partition_table(&part_table);
	}

	return swan_sfdisk(&emmc_desc, &part_table);
}
