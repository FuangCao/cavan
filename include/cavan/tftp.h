#pragma once

#include <cavan/network.h>

#define TFTP_DATA_LEN		KB(63)
#define TFTP_PKG_MAX_LEN	(TFTP_DATA_LEN + 4)
#define TFTP_RETRY_COUNT	5
#define TFTP_TIMEOUT_VALUE	2
#define TFTP_LAST_ACK_TIMES	2
#define TFTP_MAX_LINK_COUNT	10

#define TFTP_RRQ 			1
#define TFTP_WRQ 			2
#define TFTP_DATA 			3
#define TFTP_ACK 			4
#define TFTP_ERROR			5
#define TFTP_DD_RRQ			6
#define TFTP_DD_WRQ			7
#define TFTP_MKDIR_REQ		8
#define TFTP_MKNODE_REQ		9
#define TFTP_SYMLINK_REQ	10
#define TFTP_COMMAND_REQ	11

#define TFTP_COMMAND_LOG_FILE	"/tmp/tftp.cmd.log"
#define TFTP_COMMAND_EXEC_TIME	(60 * 1000 * 5)
#define TFTP_DD_DEFAULT_PORT	8888

#pragma pack(1)
struct tftp_request_pkg
{
	u16 op_code;
	char filename[TFTP_PKG_MAX_LEN - 2];
};

struct tftp_dd_request_pkg
{
	u16 op_code;
	u32 offset;
	u32 size;
	char filename[TFTP_PKG_MAX_LEN - 10];
};

struct tftp_data_pkg
{
	u16 op_code;
	u16 blk_num;
	char data[TFTP_PKG_MAX_LEN - 4];
};

struct tftp_ack_pkg
{
	u16 op_code;
	u16 blk_num;
};

struct tftp_error_pkg
{
	u16 op_code;
	u16 err_code;
	char err_msg[TFTP_PKG_MAX_LEN - 4];
};

struct tftp_mkdir_pkg
{
	u16 op_code;
	mode_t mode;
	char pathname[TFTP_PKG_MAX_LEN - 6];
};

struct tftp_mknode_pkg
{
	u16 op_code;
	mode_t mode;
	dev_t dev;
	char pathname[TFTP_PKG_MAX_LEN - 10];
};

struct tftp_symlink_pkg
{
	u16 op_code;
	char pathname[TFTP_PKG_MAX_LEN - 2];
};

struct tftp_command_pkg
{
	u16 op_code;
	char command[TFTP_PKG_MAX_LEN - 2];
};

union tftp_pkg
{
	struct
	{
		u16 op_code;
		char buffer[TFTP_PKG_MAX_LEN - 2];
	};
	struct tftp_request_pkg req;
	struct tftp_dd_request_pkg dd_req;
	struct tftp_data_pkg data;
	struct tftp_ack_pkg ack;
	struct tftp_error_pkg err;
	struct tftp_mkdir_pkg mkdir;
	struct tftp_mknode_pkg mknode;
	struct tftp_symlink_pkg symlink;
	struct tftp_command_pkg command;
};
#pragma pack()

struct tftp_request
{
	struct sockaddr_in client_addr;
	char filename[512];
	char file_mode[512];
	u32 offset;
	u32 size;
};

int tftp_client_receive_file(const char *ip, u16 port, const char *file_in, const char *file_out, u32 offset_in, u32 offset_out, u32 size);
int tftp_client_receive_all(const char *ip, u16 port, const char *file_in, const char *file_out);
int tftp_client_send_file(const char *ip, u16 port, const char *file_in, const char *file_out, u32 offset_in, u32 offset_out, u32 size);
int tftp_client_send_directory(const char *ip, u16 port, const char *dir_in, const char *dir_out);
int tftp_client_send_all(const char *ip, u16 port, const char *file_in, const char *file_out);

int tftp_service_receive_data(const char *file_out, u32 offset_out, const char *file_mode, struct sockaddr_in *remote_addr);
int tftp_service_send_data(const char *file_in, u32 offset_in, u32 size, const char *file_mode, struct sockaddr_in *remote_addr);

int send_mkdir_request(const char *ip, u16 port, const char *pathname, mode_t mode);
int vsend_command_request(const char *ip, u16 port, const char *command, va_list ap);
int send_command_request(const char *ip, u16 port, const char *command, ...);
int send_command_request_show(const char *ip, u16 port, const char *command, ...);
int send_mknode_request(const char *ip, u16 port, const char *pathname, mode_t mode, dev_t dev);
int send_symlink_request(const char *ip, u16 port, const char *file_in, const char *file_out);

int tftp_mkdir(struct tftp_mkdir_pkg *mkdir_pkg_p, const struct sockaddr_in *remote_addr);
int tftp_command(struct tftp_command_pkg *command_pkg_p, const struct sockaddr_in *remote_addr);
int tftp_command_pipe(struct tftp_command_pkg *command_pkg_p, const struct sockaddr_in *remote_addr);
int tftp_mknode(struct tftp_mknode_pkg *mknode_pkg_p, const struct sockaddr_in *remote_addr);
int tftp_symlink(struct tftp_symlink_pkg *symlink_pkg_p, const struct sockaddr_in *remote_addr);

