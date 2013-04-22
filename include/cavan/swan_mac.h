#pragma once

#include <cavan.h>

#define MAX_SN_LEN				64
#define WIFI_MAC_LEN			18
#define BT_MAC_LEN				27

#define I200_WIFI_MAC_FILE		"wifi/softmac"
#define I600_WIFI_MAC_FILE		"lib/firmware/ath6k/AR6102/softmac"

#define I600_BT_MAC_FILE		"etc/bluetooth/BC6888.psr"
#define I200_BT_MAC_FILE		"etc/bluetooth/BC6QFN.psr"

struct oem_info
{
	u8 flags;
	char sn[MAX_SN_LEN];
	char wifi_mac[WIFI_MAC_LEN];
	char bt_mac[BT_MAC_LEN];
};

int write_mac_address(const char *emmc_dev, const char *system_mnt_point);
