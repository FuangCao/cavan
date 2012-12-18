#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Sat Jan 14 12:59:51 CST 2012
 */

#include <cavan.h>
#include <cavan/network.h>
#include <cavan/service.h>

#define TCP_DD_DEFAULT_PORT		8888
#define TCP_DD_DAEMON_COUNT		10
#define TCP_DD_TIMEOUT			5000

enum tcp_dd_package_type
{
	TCP_DD_WRITE,
	TCP_DD_READ,
	TCP_DD_EXEC,
	TCP_DD_RESPONSE,
	TCP_DD_PACKAGE_COUNT
};

struct tcp_dd_response_package
{
	u32 code;
	u32 number;
	char message[1024];
};

struct tcp_dd_file_request
{
	u32 offset;
	u32 size;
	u32 mode;
	char filename[1024];
};

struct tcp_dd_exec_request
{
	char command[1024];
};

struct tcp_dd_package
{
	u32 type;

	union
	{
		struct tcp_dd_response_package res_pkg;
		struct tcp_dd_file_request file_req;
		struct tcp_dd_exec_request exec_req;
	};
};

int tcp_dd_service_run(struct cavan_service_description *desc, u16 port);
int tcp_dd_send_file(struct inet_file_request *file_req);
int tcp_dd_receive_file(struct inet_file_request *file_req);
int tcp_dd_exec_command(struct inet_file_request *file_req);
