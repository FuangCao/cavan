#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Sat Jan 14 12:59:51 CST 2012
 */

#include <cavan.h>
#include <cavan/network.h>
#include <cavan/service.h>
#include <cavan/alarm.h>
#include <cavan/block.h>

#define TCP_DD_DEFAULT_PORT			8888
#define TCP_DD_DAEMON_COUNT			10
#define TCP_DD_TIMEOUT				5000
#define TCP_DD_VERSION				0x20151223
#define TCP_KEYPAD_DEVICE			"/dev/tcp_keypad"

#define TCP_KEYPADF_EXIT_ACK		(1 << 0)
#define TCP_KEYPADF_CMDLINE			(1 << 1)

#define TCP_DDF_BREAKPOINT_RESUME	(1 << 0)
#define TCP_DDF_SKIP_EXIST			(1 << 1)

#define TCP_DD_PKG_BODY_OFFSET	offsetof(struct tcp_dd_package, body)

typedef int (*tcp_dd_handler_t)(struct network_url *url, struct network_file_request *req, u32 flags);

enum tcp_dd_package_type {
	TCP_DD_RESPONSE,
	TCP_DD_WRITE,
	TCP_DD_READ,
	TCP_DD_EXEC,
	TCP_ALARM_ADD,
	TCP_ALARM_REMOVE,
	TCP_ALARM_LIST,
	TCP_KEYPAD_EVENT,
	TCP_DD_MKDIR,
	TCP_DD_RDDIR,
	TCP_DD_FILE_STAT,
	TCP_DD_BREAKPOINT,
	TCP_DD_PACKAGE_COUNT
};

#pragma pack(1)

struct tcp_dd_response_package {
	u32 code;
	u32 number;
	char message[1024];
};

struct tcp_dd_file_request {
	u64 offset;
	u64 size;
	u32 mode;
	char filename[1024];
};

struct tcp_dd_mkdir_request {
	u32 mode;
	char pathname[1024];
};

struct tcp_dd_rddir_request {
	char pathname[1024];
};

struct tcp_dd_exec_request {
	u16 lines;
	u16 columns;
	char command[1024];
};

struct tcp_alarm_add_request {
	u32 time;
	u32 repeat;
	char command[1024];
};

struct tcp_alarm_query_request {
	u32 index;
};

struct tcp_dd_package {
	u32 version;
	u16 type;
	u16 type_inverse;
	u32 flags;

	union {
		u8 body[0];
		struct tcp_dd_response_package res_pkg;
		struct tcp_dd_file_request file_req;
		struct tcp_dd_exec_request exec_req;
		struct tcp_alarm_add_request alarm_add;
		struct tcp_alarm_query_request alarm_query;
		struct tcp_dd_mkdir_request mkdir_pkg;
		struct tcp_dd_rddir_request rddir_pkg;
	};
};

#pragma pack()

struct cavan_tcp_dd_service {
	struct network_service service;
	struct cavan_alarm_thread alarm;
	struct network_url url;
	char *filename;
	char pathname[1024];
	int brightness;
	char backlight[1024];
	int mouse_fd;
	int keypad_fd;
	int keypad_use_count;
	bool keypad_uinput;
	bool keypad_insmod;
	const char *keypad_ko;
	struct cavan_part_table *part_table;
	struct network_discovery_service discovery;
};

void tcp_dd_set_package_type(struct tcp_dd_package *pkg, u16 type);
bool tcp_dd_package_is_valid(const struct tcp_dd_package *pkg);
bool tcp_dd_package_is_invalid(const struct tcp_dd_package *pkg);
ssize_t tcp_dd_package_recv(struct network_client *client, struct tcp_dd_package *pkg);
ssize_t tcp_dd_package_send(struct network_client *client, struct tcp_dd_package *pkg, u16 type, size_t length, u32 flags);
int tcp_dd_send_request(struct network_client *client, struct tcp_dd_package *pkg, struct tcp_dd_package *response, u16 type, size_t length, u32 flags);
int tcp_dd_send_request2(struct network_client *client, struct tcp_dd_package *pkg, u16 type, size_t length, u32 flags);
int tcp_dd_send_request3(struct network_url *url, struct tcp_dd_package *pkg, struct tcp_dd_package *response, u16 type, size_t length, u32 flags);
int tcp_dd_send_request4(struct network_url *url, struct tcp_dd_package *pkg, u16 type, size_t length, u32 flags);

int tcp_dd_get_partition_filename(const char *name, char *buff, size_t size);
const char *tcp_dd_get_partition_pathname(struct cavan_tcp_dd_service *service, const char *name);

int tcp_dd_service_run(struct cavan_dynamic_service *service);
int tcp_dd_send_file(struct network_url *url, struct network_file_request *file_req, u32 flags);
int tcp_dd_receive_file(struct network_url *url, struct network_file_request *file_req, u32 flags);
int tcp_dd_exec_command(struct network_url *url, const char *command);
int tcp_dd_keypad_client_run(struct network_url *url, int flags);

int tcp_alarm_add(struct network_url *url, const char *command, time_t time, time_t repeat);
int tcp_alarm_remove(struct network_url *url, int index);
int tcp_alarm_list(struct network_url *url, int index);

int tcp_dd_mkdir(struct network_url *url, const char *pathname, mode_t mode);
