#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Sat Jan 14 12:59:51 CST 2012
 */

#include <cavan.h>
#include <cavan/network.h>

#define TCP_TFTP_SERVER_PORT	8888
#define TCP_TFTP_MAX_SEND_SIZE	1024
#define TCP_TFTP_DAEMON_COUNT	10
#define TCP_TFTP_TIMEOUT		5000

enum tcp_tftp_package_type
{
	TCP_TFTP_REQ_READ,
	TCP_TFTP_REQ_WRITE,
	TCP_TFTP_EVENT_ACK,
	TCP_TFTP_EVENT_ERROR,
};

#pragma pack(1)
struct tcp_tftp_write_request
{
	u16 pkg_type;
	struct stat st;
	char pathname[1024];
};

struct tcp_tftp_read_request
{
	u16 pkg_type;
	char pathname[1024];
};

struct tcp_tftp_ack
{
	u16 pkg_type;
	struct stat st;
};

struct tcp_tftp_error_message
{
	u16 pkg_type;
	u32 err_code;
	char message[1024];
};

union tcp_tftp_package
{
	u16 pkg_type;
	struct tcp_tftp_read_request read_req;
	struct tcp_tftp_write_request write_req;
	struct tcp_tftp_ack ack;
	struct tcp_tftp_error_message err_msg;
};
#pragma pack()

int tcp_tftp_service_run(u16 port);
int tcp_tftp_send_file(const char *inpath, const char *outpath, const char *ip, u16 port);
