#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Sun Jan 27 21:46:02 CST 2013
 */

#include <cavan.h>

#define	SPRD_VBPIPE_FLAG_BYTE		0x7E
#define SPRD_VBPIPE_ESCAPE_BYTE		0x7D
#define SPRD_VBPIPE_COMPLEMENT_BYTE	0x20

#define SPRD_VBPIPE_MASK_IMEI1		(1 << 0)
#define SPRD_VBPIPE_MASK_IMEI2		(1 << 1)
#define SPRD_VBPIPE_MASK_BT_MAC		(1 << 2)
#define SPRD_VBPIPE_MASK_GPS_INFO	(1 << 3)
#define SPRD_VBPIPE_MASK_IMEI3		(1 << 4)
#define SPRD_VBPIPE_MASK_IMEI4		(1 << 5)
#define SPRD_VBPIPE_MASK_WIFI_MAC	(1 << 6)
#define SPRD_VBPIPE_READ_FLAG		(1 << 7)

#define SPRD_VBPIPE_OPER_SUCCESS_FLAG	0x01

#pragma pack(1)
struct sprd_vbpipe_message_desc
{
	u32 seq_num;
	u16 length;
	u8 type;
	u8 subtype;
};

struct sprd_vbpipe_imei_data
{
	u8 imei1[8];
	u8 imei2[8];
	u8 bt_mac[6];
	u8 gps_info[44];
	u8 wifi_mac[6];
	u8 reserved1[2];
	u8 imei3[8];
	u8 imei4[8];
	u8 reserved2[16];
	u16 crc16;
};
#pragma pack(0)

struct sprd_vbpipe_command_desc
{
	u32 seq_num;
	u8 cmd_type, cmd_subtype;
	u8 reply_type, reply_subtype;
	const void *command;
	size_t command_len;
	void *reply;
	size_t reply_len;
};

enum sprd_vbpipe_command_type
{
	SPRD_VBPIPE_SWVER_F = 0,
	SPRD_VBPIPE_SOFTKEY_F,
	SPRD_VBPIPE_LOG_F,
	SPRD_VBPIPE_AT_F,
	SPRD_VBPIPE_GET_SYS_INFO_F,
	SPRD_VBPIPE_SYSTEM_F,
	SPRD_VBPIPE_CHIP_TEST_F,
	SPRD_VBPIPE_POWER_F,
	SPRD_VBPIPE_SIM_F,
	SPRD_VBPIPE_PS_F,
	SPRD_VBPIPE_RF_F,
	SPRD_VBPIPE_MCU_F,
	SPRD_VBPIPE_CHANGE_MODE_F,
	SPRD_VBPIPE_NVITEM_F,
	SPRD_VBPIPE_AT_CHLSRV_F,
	SPRD_VBPIPE_ADC_F,
	SPRD_VBPIPE_PIN_TEST_F,
	SPRD_VBPIPE_CURRENT_TEST_F,
	SPRD_VBPIPE_SIM_CARD_TEST_F,
	SPRD_VBPIPE_AUDIO_TEST_F,
	SPRD_VBPIPE_KEYPAD_TEST_F,
	SPRD_VBPIPE_LCM_TEST_F,
	SPRD_VBPIPE_VIBRATOR_TEST_F,
	SPRD_VBPIPE_RPC_EFS_F,
	SPRD_VBPIPE_RPC_DC_F,
	SPRD_VBPIPE_PRODUCTION_TEST_EXTEND_F,
	SPRD_VBPIPE_FORCE_ON_CELLS_F,
	SPRD_VBPIPE_GPS_F = 35,
	SPRD_VBPIPE_TD_CAL_PARAM,
	SPRD_VBPIPE_TV_PARAM,
    SPRD_VBPIPE_DSP_DEBUG = 50,
	SPRD_VBPIPE_ZTE_ADJUST = 70,
	SPRD_VBPIPE_HANDSET_INFO = 93,
	SPRD_VBPIPE_DIRECT_NV,
    SPRD_VBPIPE_FASTWR_PRODUCTINFO,
    SPRD_VBPIPE_CHECK_NV,
    SPRD_VBPIPE_CHECK_IMEICOUNT,
	SPRD_VBPIPE_REQ_MAX_F
};

u16 sprd_vbpipe_crc16(u16 crc, const u8 *buff, size_t size);
void sprd_vbpipe_show_data(const char *prompt, const char *data, size_t size);
void sprd_vbpipe_show_imei(const struct sprd_vbpipe_imei_data *imei);

char *sprd_vbpipe_encode_data(const char *src, size_t srclen, char *dest, size_t destlen);
size_t sprd_vbpipe_encode_message(const struct sprd_vbpipe_message_desc *message, const char *src, size_t srclen, char *dest, size_t destlen);
char *sprd_vbpipe_decode_data(const char *src, size_t srclen, char *dest, size_t destlen, size_t *reslen);

ssize_t sprd_vbpipe_read_reply(int fd, struct sprd_vbpipe_command_desc *command);
int sprd_vbpipe_send_command(int fd, struct sprd_vbpipe_command_desc *command);

int sprd_vbpipe_read_imei(int fd, struct sprd_vbpipe_imei_data *imei, u8 mask);
int sprd_vbpipe_write_imei(int fd, struct sprd_vbpipe_imei_data *imei, u8 mask);
