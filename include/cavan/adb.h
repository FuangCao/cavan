#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Thu Jul 19 10:31:41 CST 2012
 */

#include <cavan.h>
#include <cavan/network.h>

#define ADB_DEBUG				1

#define ADB_PORT1				5037
#define ADB_PORT2				5038
#define ADB_SMS_TRANSLATOR_PORT	8888

enum
{
	SMS_TYPE_END = 0x00,
	SMS_TYPE_TEST,
	SMS_TYPE_ACK,
	SMS_TYPE_OKAY,
	SMS_TYPE_FAILED,
	SMS_TYPE_DATE,
	SMS_TYPE_ADDRESS,
	SMS_TYPE_BODY
};

struct eavoo_short_message
{
	u32 date;
	char address[32];
	char body[1024];
};

int adb_read_status(int sockfd, char *buff, size_t size);
int adb_send_text(int sockfd, const char *text);
int adb_connect_service_base(const char *ip, u16 port);
int adb_connect_service(const char *ip, u16 port, const char *service);
int adb_create_tcp_link(const char *ip, u16 port, u16 tcp_port);
int frecv_text_and_write(int sockfd, int fd);
int recv_text_and_write(int sockfd, const char *filename);

ssize_t sms_receive_value(int sockfd, void *value, size_t size);
ssize_t sms_receive_text(int sockfd, char *buff);
int sms_receive_message(int sockfd, struct eavoo_short_message *message);
void show_eavoo_short_message(struct eavoo_short_message *message);
int fsms_receive_and_write(int sockfd, int fd);
int sms_receive_and_write(int sockfd, const char *filename);

char *adb_parse_sms_single(const char *buff, const char *end, char *segments[], size_t size);
char *adb_parse_sms_multi(const char *buff, const char *end);
char *adb_parse_sms_main(char *buff, char *end);
ssize_t sms_send_response(int sockfd, u8 type);

static inline int adb_create_tcp_link2(const char *ip, u16 port)
{
	return adb_create_tcp_link(ip, 0, port);
}

static inline int adb_send_command(int sockfd, const char *command)
{
	return adb_send_text(sockfd, command);
}
