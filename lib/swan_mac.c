#include <cavan.h>
#include <cavan/swan_mac.h>
#include <cavan/text.h>
#include <cavan/file.h>
#include <cavan/device.h>
#include <cavan/image.h>

#define MAX_BUFF_LEN			KB(4)

static void show_oem_info(struct oem_info *oem)
{
	println("flags = 0x%02x", oem->flags);
	println("sn = %s", text_header(oem->sn, MAX_SN_LEN));
	println("wifi_mac = %s", text_header(oem->wifi_mac, WIFI_MAC_LEN));
	println("bt_mac = %s", get_text_region(oem->bt_mac, 8, BT_MAC_LEN));
}

static void check_bt_mac(char *bt_mac)
{
	ssize_t readlen;
	u32 bt_mac_value[4];

	readlen = file_read_urandom(bt_mac_value, sizeof(bt_mac_value));
	if (readlen < 0)
	{
		unsigned int i;

		for (i = 0; i < ARRAY_SIZE(bt_mac_value); i++)
		{
			bt_mac_value[i] = random();
		}
	}

	sscanf(bt_mac, "&0001 = %x %x %x %x", bt_mac_value, bt_mac_value + 1, bt_mac_value + 2, bt_mac_value + 3);
	sprintf(bt_mac, "&0001 = %04X %04X %04X %04X",
		bt_mac_value[0] & 0xFFFF, bt_mac_value[1] & 0xFFFF, bt_mac_value[2] & 0xFFFF, bt_mac_value[3] & 0xFFFF);
}

static void gen_wifi_mac(u8 *wifi_mac_value)
{
	int i;
	ssize_t readlen;

	do {
		readlen = file_read_urandom(wifi_mac_value + 3, 3);
		if (readlen < 0)
		{
			for (i = 3; i < 6; i++)
			{
				wifi_mac_value[i] = random();
			}
		}

		for (i = 3; i < 6 && wifi_mac_value[i] != 0xFF; i++);
	} while (i < 6);

	wifi_mac_value[5] &= ~1;
	wifi_mac_value[2] = 0x08;
	wifi_mac_value[1] = 0x11;
	wifi_mac_value[0] = 0xFC;
}

static void check_wifi_mac(char *wifi_mac)
{
	int i;
	int ret;
	u8 wifi_mac_value[6];
	u32 wifi_mac_value32[6];

	ret = sscanf(wifi_mac, "%x:%x:%x:%x:%x:%x",
		wifi_mac_value32 + 5, wifi_mac_value32 + 4, wifi_mac_value32 + 3,
		wifi_mac_value32 + 2, wifi_mac_value32 + 1, wifi_mac_value32);
	if (ret == 6)
	{
		goto out_copy_mac;
	}

	ret = sscanf(wifi_mac, "%x-%x-%x-%x-%x-%x",
		wifi_mac_value32 + 5, wifi_mac_value32 + 4, wifi_mac_value32 + 3,
		wifi_mac_value32 + 2, wifi_mac_value32 + 1, wifi_mac_value32);
	if (ret == 6)
	{
		goto out_copy_mac;
	}

	gen_wifi_mac(wifi_mac_value);
	goto out_set_mac;

out_copy_mac:
	for (i = 0; i < 6; i++)
	{
		wifi_mac_value[i] = wifi_mac_value32[i] & 0xFF;
	}
out_set_mac:
	sprintf(wifi_mac, "%02X:%02X:%02X:%02X:%02X:%02X",
		wifi_mac_value[5], wifi_mac_value[4], wifi_mac_value[3],
		wifi_mac_value[2], wifi_mac_value[1], wifi_mac_value[0]);
}

static int read_oem_info(const char *emmc_dev, struct oem_info *oem_info)
{
	int ret;

	ret = file_readfrom(emmc_dev, oem_info, sizeof(*oem_info), SN_OFFSET, 0);
	if (ret < 0)
	{
		error_msg("read oem info");
		return ret;
	}

	check_bt_mac(oem_info->bt_mac);
	check_wifi_mac(oem_info->wifi_mac);
	show_oem_info(oem_info);

	return 0;
}

static char *get_bt_mac_file(const char *system_mnt_point, char *pathname, size_t size)
{
	unsigned int i;
	const char *bt_mac_files[] = { I600_BT_MAC_FILE, I200_BT_MAC_FILE };
	char *name_p;

	name_p = text_path_cat(pathname, size, system_mnt_point, NULL);

	for (i = 0; i < ARRAY_SIZE(bt_mac_files); i++)
	{
		text_copy(name_p, bt_mac_files[i]);

		if (access(pathname, W_OK) == 0)
		{
			return pathname;
		}
	}

	return NULL;
}

static ssize_t write_bt_mac(const char *system_mnt_point, char *mac, size_t size)
{
	char bt_mac_file[1024];

	if (get_bt_mac_file(system_mnt_point, bt_mac_file, sizeof(bt_mac_file)) == NULL)
	{
		ERROR_RETURN(ENOENT);
	}

	println("BT-MAC file is: %s", bt_mac_file);

	if (file_replace_line_simple(bt_mac_file, "&0001", 5, mac, size) < 0)
	{
		char buff[size + 1];

		buff[0] = '\n';

		memcpy(buff + 1, mac, size);

		return file_writeto(bt_mac_file, buff, sizeof(buff), 0, O_APPEND);
	}

	return 0;
}

static ssize_t write_wifi_mac(const char *system_mnt_point, char *mac, size_t size)
{
	int ret;
	char i600_wifi_mac[1024], i200_wifi_mac[1024];

	text_path_cat(i200_wifi_mac, sizeof(i200_wifi_mac), system_mnt_point, I200_WIFI_MAC_FILE);

	ret = file_writeto(i200_wifi_mac, mac, size, 0, O_TRUNC);
	if (ret < 0)
	{
		print_error("file_writeto");
		return ret;
	}

	text_path_cat(i600_wifi_mac, sizeof(i600_wifi_mac), system_mnt_point, I600_WIFI_MAC_FILE);

	return file_hardlink(i600_wifi_mac, i200_wifi_mac);
}

int write_mac_address(const char *emmc_dev, const char *system_mnt_point)
{
	int ret;
	struct oem_info oem_info;

	ret = read_oem_info(emmc_dev, &oem_info);
	if (ret < 0)
	{
		error_msg("read oem info");
		return ret;
	}

	ret = write_wifi_mac(system_mnt_point, oem_info.wifi_mac, WIFI_MAC_LEN - 1);
	if (ret < 0)
	{
		print_error("write wifi mac address");
	}

	ret = write_bt_mac(system_mnt_point, oem_info.bt_mac, BT_MAC_LEN);
	if (ret < 0)
	{
		print_error("write bluetooth mac address");
	}

	return 0;
}
