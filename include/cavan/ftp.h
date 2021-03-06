#pragma once

// Fuang.Cao <cavan.cfa@gmail.com> 2011-10-26 16:17:07

#include <cavan.h>
#include <cavan/network.h>
#include <cavan/service.h>

#define FTP_DATA_PORT		20
#define FTP_CTRL_PORT		21
#define FTP_DAEMON_COUNT	10
#define FTP_TIMEOUT_MS		(1000 * 60 * 20)

#define FTP_BUILD_CMD(a, b, c, d) \
	BYTES_DWORD(d, c, b, a)

#define FTP_BUILD_CMD2(a) \
	FTP_BUILD_CMD((a)[0], (a)[1], (a)[2], (a)[3])

enum cavan_ftp_state {
	FTP_STATE_READY,
	FTP_STATE_USER_RECVED,
	FTP_STATE_LOGINED,
	FTP_STATE_PORT_RECVED,
};

struct cavan_ftp_service {
	struct network_service service;
	struct network_url url;
	char home[1024];
};

struct cavan_ftp_client {
	struct network_client client;
	struct network_url data_url;
	struct network_client data_client;
	struct network_service data_service;

	char file_type;
	bool port_received;
	bool pasv_received;
	enum cavan_ftp_state state;
};

struct ftp_command_package {
	const char *command;
	size_t cmdsize;
	char *response;
	size_t rspsize;
	struct network_service *service;
	struct network_client *data_client;
};

char *ftp_file_stat_tostring(const char *filepath, char *buff, char *buff_end);
char *ftp_list_directory(const char *dirpath, char *text);

int ftp_service_run(struct cavan_dynamic_service *service);

int ftp_client_read_response(struct network_client *client, char *response, size_t size);
int ftp_client_send_command_package(struct network_client *client, struct ftp_command_package *pkg);
int ftp_client_send_command(struct network_client *client, const char *command, size_t cmdsize, char *response, size_t rspsize);
int ftp_client_send_command2(struct network_client *client, char *response, size_t rspsize, const char *command, ...);
int ftp_client_send_pasv_command(struct network_client *client, struct network_url *url);
int ftp_client_create_pasv_link(struct network_client *client_ctrl, struct network_client *client_data);
int ftp_client_login(struct network_client *client, const char *username, const char *password);
int ftp_client_run(struct network_url *url, const char *username, const char *password);
