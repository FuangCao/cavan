#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Tue Sep 18 11:33:57 CST 2012
 */

#include <cavan.h>
#include <cavan/service.h>

#define SWAN_VK_PROMPT		"SWAN-VK"
#define DEVICE_SWAN_VK_DATA "/dev/swan_vk"
#define DEVICE_SWAN_VK_VALUE "/sys/devices/platform/swan_vk.0/value"
#define DEVICE_SWAN_TTY		"/dev/ttymxc0"
#define SWAN_VK_STOP_VALUE	0x504F5453
#define SWAN_VK_TCP_PORT	8888

#define SWAN_VKEY_SEARCH		217
#define SWAN_VKEY_BACK			158
#define SWAN_VKEY_MENU			127
#define SWAN_VKEY_MENU2			139
#define SWAN_VKEY_HOME			172
#define SWAN_VKEY_VOLUME_UP		115
#define SWAN_VKEY_VOLUME_DOWN	114
#define SWAN_VKEY_CAMERA		212
#define SWAN_VKEY_CALL			169
#define SWAN_VKEY_ENDCALL		397
#define SWAN_VKEY_POWER			116
#define SWAN_VKEY_UP			103
#define SWAN_VKEY_DOWN			108
#define SWAN_VKEY_LEFT			105
#define SWAN_VKEY_RIGHT			106

struct swan_vk_descriptor
{
	char *name;
	unsigned int code;
};

struct swan_vk_service_descriptor
{
	int sockfd;
	int datafd;
	pthread_mutex_t mutex_lock;
};

int swan_vk_serial_client(const char *tty_path);
int swan_vk_serial_server(const char *tty_path, const char *data_path);
int swan_vk_adb_client(const char *ip, u16 port);
int swan_vk_adb_server(struct cavan_service_description *desc, const char *data_path, u16 port);
int swan_vk_commadline(const char *data_path);
