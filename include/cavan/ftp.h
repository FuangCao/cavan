#pragma once

// Fuang.Cao <cavan.cfa@gmail.com> 2011-10-26 16:17:07

#include <cavan/network.h>

#define FTP_DATA_PORT		20
#define FTP_COLTROL_PORT	21
#define CAVAN_FTP_ROOT_DIR	"/"

enum cavan_ftp_state
{
	FTP_STATE_READY,
	FTP_STATE_USER_RECVED,
	FTP_STATE_PASS_FAULT,
	FTP_STATE_LOGINED,
	FTP_STATE_PORT_RECVED,
};

struct cavan_ftp_descriptor
{
	int ctrl_sockfd;
	int data_sockfd;
};

int ftp_service_run(u16 port, int count);
int ftp_client_run(const char *ip_address, u16 port);

