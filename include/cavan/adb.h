#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Thu Jul 19 10:31:41 CST 2012
 */

#include <cavan.h>
#include <cavan/network.h>

#define ADB_PORT1				5037
#define ADB_PORT2				5038
#define ADB_SMS_TRANSLATOR_PORT	8888

int adb_read_status(int sockfd, char *buff, size_t size);
int adb_send_text(int sockfd, const char *text);
int adb_connect_service_base(const char *ip, u16 port);
int adb_connect_service(int sockfd, const char *service);
int adb_connect_service2(const char *ip, u16 port, const char * service);
int adb_create_tcp_link(const char *ip, u16 port, u16 tcp_port);
int frecv_text_and_write(int sockfd, int fd);
int recv_text_and_write(int sockfd, const char *filename);

