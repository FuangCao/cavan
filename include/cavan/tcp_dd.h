#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Sat Jan 14 12:59:51 CST 2012
 */

#include <cavan.h>
#include <cavan/network.h>

#define TCP_DD_SERVER_PORT		8888
#define TCP_DD_DAEMON_COUNT		10
#define TCP_DD_TIMEOUT			5000

enum tcp_dd_package_type
{
	TCP_DD_WRITE,
	TCP_DD_READ,
	TCP_DD_RESPONSE,
	TCP_DD_PACKAGE_COUNT
};

struct tcp_dd_response_package
{
	int code;
	int number;
	char message[1024];
};

struct tcp_dd_file_request
{
	off_t offset;
	off_t size;
	mode_t mode;
	char filename[1024];
};

union tcp_dd_body
{
	struct tcp_dd_response_package res_pkg;
	struct tcp_dd_file_request file_req;
};

struct tcp_dd_package
{
	u32 type;
	union tcp_dd_body body;
};

int tcp_dd_service_run(u16 port);
int tcp_dd_send_file(const char *ip, u16 port, const char *src_file, off_t src_offset, const char *dest_file, off_t dest_offset, off_t size);
int tcp_dd_receive_file(const char *ip, u16 port, const char *src_file, off_t src_offset, const char *dest_file, off_t dest_offset, off_t size);
