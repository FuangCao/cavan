// Fuang.Cao <cavan.cfa@gmail.com> Thu Dec 30 11:34:28 CST 2010

#include <cavan.h>
#include <cavan/device.h>
#include <cavan/text.h>

int main(int argc, char *argv[])
{
	int ret;
	int i, j;
	int dev_fd;
	struct master_boot_sector mbs;
	struct hd_geometry geo;
	u32 total_secs;
	u64 total_bytes;
	u32 part_secs;
	u32 sec_size;
	u32 start_sec;

	assert(argc > 2);

	dev_fd = open(argv[1], O_RDWR | O_BINARY);
	if (dev_fd < 0) {
		print_error("open device \"%s\"", argv[1]);
		return -1;
	}

	ret = fread_master_boot_sector(dev_fd, &mbs);
	if (ret < 0) {
		error_msg("fread_master_boot_sector");
		goto out_close_dev;
	}

	ret = fget_device_geometry(dev_fd, &geo);
	if (ret < 0) {
		error_msg("fget_device_geometry");
		goto out_close_dev;
	}

	ret = get_sector_size(dev_fd, &sec_size);
	if (ret < 0) {
		error_msg("get_sector_size");
		goto out_close_dev;
	}

	ret = fget_device_size(dev_fd, &total_bytes);
	if (ret < 0) {
		error_msg("fget_device_size");
		goto out_close_dev;
	}

	total_secs = total_bytes / sec_size;

	println("device size = %s = %d(sector)", size2text(total_bytes), total_secs);

	show_geometry(&geo);

	memset(mbs.disk_part_tables, 0, 64);

	start_sec = sector_cylinder_lalignment(text2size(argv[2], NULL) / sec_size, &geo);

	println("start_sec = %d(sector)", start_sec);

	if (start_sec == 0) {
		start_sec = 1;
	}

	for (i = 0, j = 3; i < 4 && j < argc; i++, j++) {
		part_secs = sector_cylinder_alignment_auto(text2size(argv[j], NULL) / sec_size, &geo);

		println("partition start address = %d(sector), size = %d(sector)", start_sec, part_secs);

		if (start_sec + part_secs > total_secs) {
			ret = -1;
			error_msg("partition size is too large");
			goto out_close_dev;
		}

		set_part_address(mbs.disk_part_tables + i, &geo, start_sec, calculate_partition_size(start_sec, part_secs, &geo));
		mbs.disk_part_tables[i].file_system_mark = 0x83;

		start_sec += part_secs;
	}

	if (i < 4 && start_sec < total_secs) {
		part_secs = sector_cylinder_lalignment(total_secs - start_sec, &geo);

		set_part_address(mbs.disk_part_tables + i, &geo, start_sec, calculate_partition_size(start_sec, part_secs, &geo));
		mbs.disk_part_tables[i].file_system_mark = 0x83;
	}

	mbs.magic_number = 0xAA55;

	ret = fwrite_master_boot_sector(dev_fd, &mbs);
	if (ret < 0) {
		error_msg("fwrite_master_boot_sector");
		goto out_close_dev;
	}

	ret = freread_part_table(dev_fd);
	if (ret < 0) {
		error_msg("freread_part_table_retry");
		goto out_close_dev;
	}

out_close_dev:
	close(dev_fd);

	return ret;
}
