#pragma once

// Fuang.Cao <cavan.cfa@gmail.com> 2011-10-26 16:17:07

#include <cavan.h>
#include <cavan/network.h>
#include <cavan/service.h>

#define FTP_DATA_PORT		20
#define FTP_CTRL_PORT		21
#define FTP_DAEMON_COUNT	10

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

extern char ftp_root_path[];

char *ftp_file_stat_tostring(const char *filepath, char *text);
char *ftp_list_directory(const char *dirpath, char *text);

int ftp_service_run(struct cavan_service_description *service_desc, u16 port);

int ftp_client_read_response(int sockfd, char *response, size_t size);
int ftp_client_send_command(int sockfd, const char *command, size_t cmdsize, char *response, size_t repsize);
int ftp_client_send_command2(int sockfd, char *response, size_t repsize, const char *command, ...);
int ftp_client_login(int sockfd, const char *username, const char *password);
int ftp_client_run(const char *hostname, u16 port, const char *username, const char *password);
