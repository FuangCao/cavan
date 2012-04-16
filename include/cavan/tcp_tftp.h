#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Sat Jan 14 12:59:51 CST 2012
 */

#include <cavan.h>
#include <cavan/network.h>

#define TCP_TFTP_SERVER_PORT		8888
#define TCP_TFTP_DAEMON_COUNT		10
#define TCP_TFTP_TIMEOUT			5000

enum tcp_tftp_package_type
{
	TCP_TFTP_WRITE,
	TCP_TFTP_READ,
	TCP_TFTP_RESPONSE,
	TCP_TFTP_PACKAGE_COUNT
};

struct tcp_tftp_response_package
{
	int code;
	char message[1024];
};

struct tcp_tftp_file_request
{
	off_t offset;
	off_t size;
	mode_t mode;
	char filename[512];
};

union tcp_tftp_body
{
	struct tcp_tftp_response_package res_pkg;
	struct tcp_tftp_file_request file_req;
};

struct tcp_tftp_package
{
	u32 type;
	union tcp_tftp_body body;
};

int tcp_tftp_service_run(u16 port);
int tcp_tftp_send_file(const char *ip, u16 port, const char *src_file, off_t src_offset, const char *dest_file, off_t dest_offset, off_t size);
int tcp_tftp_receive_file(const char *ip, u16 port, const char *src_file, off_t src_offset, const char *dest_file, off_t dest_offset, off_t size);
