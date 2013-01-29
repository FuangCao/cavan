#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Sun Jan 27 21:46:02 CST 2013
 */

#include <cavan.h>

#define	SPRD_DIAG_FLAG_BYTE			0x7E
#define SPRD_DIAG_ESCAPE_BYTE		0x7D
#define SPRD_DIAG_COMPLEMENT_BYTE	0x20

#define SPRD_DIAG_MASK_IMEI1		(1 << 0)
#define SPRD_DIAG_MASK_IMEI2		(1 << 1)
#define SPRD_DIAG_MASK_BT_MAC		(1 << 2)
#define SPRD_DIAG_MASK_GPS_INFO		(1 << 3)
#define SPRD_DIAG_MASK_IMEI3		(1 << 4)
#define SPRD_DIAG_MASK_IMEI4		(1 << 5)
#define SPRD_DIAG_MASK_WIFI_MAC		(1 << 6)
#define SPRD_DIAG_READ_FLAG			(1 << 7)

#define SPRD_DIAG_OPER_SUCCESS_FLAG	0x01

#define SPRD_DEFAULT_AT_DEVICE		"/dev/CHNPTY12"

#pragma pack(1)
struct sprd_diag_message_desc
{
	u32 seq_num;
	u16 length;
	u8 type;
	u8 subtype;
};

struct sprd_diag_imei_data
{
	byte imei1[8];
	byte imei2[8];
	byte bt_mac[6];
	byte gps_info[44];
	byte wifi_mac[6];
	u8 reserved1[2];
	byte imei3[8];
	byte imei4[8];
	u8 reserved2[16];
	u16 crc16;
};
#pragma pack(0)

struct sprd_diag_command_desc
{
	u32 seq_num;
	u8 cmd_type, cmd_subtype;
	u8 reply_type, reply_subtype;
	const void *command;
	size_t command_len;
	void *reply;
	size_t reply_len;
};

enum sprd_diag_command_type
{
	SPRD_DIAG_SWVER_F = 0,
	SPRD_DIAG_SOFTKEY_F,
	SPRD_DIAG_LOG_F,
	SPRD_DIAG_AT_F,
	SPRD_DIAG_GET_SYS_INFO_F,
	SPRD_DIAG_SYSTEM_F,
	SPRD_DIAG_CHIP_TEST_F,
	SPRD_DIAG_POWER_F,
	SPRD_DIAG_SIM_F,
	SPRD_DIAG_PS_F,
	SPRD_DIAG_RF_F,
	SPRD_DIAG_MCU_F,
	SPRD_DIAG_CHANGE_MODE_F,
	SPRD_DIAG_NVITEM_F,
	SPRD_DIAG_AT_CHLSRV_F,
	SPRD_DIAG_ADC_F,
	SPRD_DIAG_PIN_TEST_F,
	SPRD_DIAG_CURRENT_TEST_F,
	SPRD_DIAG_SIM_CARD_TEST_F,
	SPRD_DIAG_AUDIO_TEST_F,
	SPRD_DIAG_KEYPAD_TEST_F,
	SPRD_DIAG_LCM_TEST_F,
	SPRD_DIAG_VIBRATOR_TEST_F,
	SPRD_DIAG_RPC_EFS_F,
	SPRD_DIAG_RPC_DC_F,
	SPRD_DIAG_PRODUCTION_TEST_EXTEND_F,
	SPRD_DIAG_FORCE_ON_CELLS_F,
	SPRD_DIAG_GPS_F = 35,
	SPRD_DIAG_TD_CAL_PARAM,
	SPRD_DIAG_TV_PARAM,
    SPRD_DIAG_DSP_DEBUG = 50,
	SPRD_DIAG_ZTE_ADJUST = 70,
	SPRD_DIAG_HANDSET_INFO = 93,
	SPRD_DIAG_DIRECT_NV,
    SPRD_DIAG_FASTWR_PRODUCTINFO,
    SPRD_DIAG_CHECK_NV,
    SPRD_DIAG_CHECK_IMEICOUNT,
	SPRD_DIAG_REQ_MAX_F
};

u16 sprd_diag_crc16(u16 crc, const u8 *buff, size_t size);
void sprd_diag_show_data(const char *prompt, const char *data, size_t size);
void sprd_diag_show_imei(const struct sprd_diag_imei_data *imei);
size_t sprd_diag_text2imei(const char *text, byte *imei, size_t isize);
char *sprd_diag_imei_tostring(const byte *imei, size_t isize, char *buff, size_t size);
char *sprd_diag_text2mac(const char *text, byte *mac, size_t size);

char *sprd_diag_encode_data(const char *src, size_t srclen, char *dest, size_t destlen);
size_t sprd_diag_encode_message(const struct sprd_diag_message_desc *message, const char *src, size_t srclen, char *dest, size_t destlen);
char *sprd_diag_decode_data(const char *src, size_t srclen, char *dest, size_t destlen, size_t *reslen);

ssize_t sprd_diag_read_reply(int fd, struct sprd_diag_command_desc *command);
int sprd_diag_send_command(int fd, struct sprd_diag_command_desc *command, int retry);

int sprd_diag_read_imei(int fd, struct sprd_diag_imei_data *imei, u8 mask);
int sprd_diag_write_imei(int fd, struct sprd_diag_imei_data *imei, u8 mask);

ssize_t sprd_modem_send_at_command(int fd, char *reply, size_t size, const char *command, ...);
