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

#define X_AXIS_MAX				1024
#define X_AXIS_MIN				0
#define Y_AXIS_MAX				1024
#define Y_AXIS_MIN				0

#define SWAN_VK_UNLOCK_X0		10
#define SWAN_VK_UNLOCK_X1		90
#define SWAN_VK_UNLOCK_Y		75

#define SWAN_VK_AXIS_CAL(value, max) \
	((value) * (max) / 100)

enum swan_vk_link_type
{
	SWAN_VK_LINK_TCP,
	SWAN_VK_LINK_UDP,
	SWAN_VK_LINK_SERIAL,
	SWAN_VK_LINK_ADB,
	SWAN_VK_LINK_LOCAL,
	SWAN_VK_LINK_INVALID
};

struct swan_vk_command_option
{
	enum swan_vk_link_type link_type;
	u16 port;
	char ip[32];
	char devpath[1024];
	bool as_daemon;
};

struct swan_vk_descriptor
{
	char *name;
	unsigned int code;
};

struct swan_vk_clien_descriptor
{
	int fd;
	int pipefd[2];
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
int swan_vk_commandline(const char *data_path);

int swan_vk_server_main(int argc, char *argv[]);
int swan_vk_client_main(int argc, char *argv[]);
int swan_vk_cmdline_main(int argc, char *argv[]);
int swan_vk_line_main(int argc, char *argv[]);
int swan_vk_unlock_main(int argc, char *argv[]);
