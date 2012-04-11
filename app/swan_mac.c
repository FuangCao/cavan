#include <cavan.h>
#include <cavan/swan_mac.h>
#include <cavan/swan_upgrade.h>

int main(int argc, char *argv[])
{
	int ret;
	const char *system_mnt_point = SYSTEM_MNT_POINT;

	ret = mount_to(EMMC_DEVICE "p2", system_mnt_point, "ext4", NULL);
	if (ret < 0)
	{
		error_msg("mount system device");
		return ret;
	}

	ret = write_mac_address(EMMC_DEVICE, system_mnt_point);
	if (ret < 0)
	{
		error_msg("write mac address failed");
	}

	umount_directory2(system_mnt_point, MNT_DETACH);

	return ret;
}
