/*
 * File:		penetrator.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-07-31 10:32:06
 *
 * Copyright (c) 2018 Fuang.Cao <cavan.cfa@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <cavan.h>
#include <cavan/penetrator.h>

static int cavan_penetrate_send_command(int sockfd, const struct sockaddr_in *addr, const char *format, ...)
{
	char command[1024];
	va_list ap;
	int length;

	va_start(ap, format);
	length = vsnprintf(command, sizeof(command), format, ap);
	va_end(ap);

	return inet_sendto(sockfd, command, length, addr);
}

static struct cavan_penetrate_conn *cavan_penetrate_conn_alloc(const struct sockaddr_in *addr, const char *name)
{
	struct cavan_penetrate_conn *conn;

	conn = cavan_malloc_type(struct cavan_penetrate_conn);
	if (conn == NULL) {
		pr_err_info("cavan_malloc_type");
		return NULL;
	}

	conn->name = strdup(name);
	if (conn->name == NULL) {
		pr_err_info("strdup");
		goto out_free_conn;
	}

	memcpy(&conn->addr, addr, sizeof(*addr));

	return conn;

out_free_conn:
	free(conn);
	return NULL;
}

#if 0
static void cavan_penetrate_conn_free(struct cavan_penetrate_conn *conn)
{
	free(conn->name);
	free(conn);
}
#endif

static void cavan_penetrate_service_add(struct cavan_penetrate_service *service, struct cavan_penetrate_conn *conn)
{
	if (service->head == NULL) {
		service->head = conn;
		conn->prev = conn->next = conn;
	} else {
		struct cavan_penetrate_conn *head = service->head;
		struct cavan_penetrate_conn *tail = head->prev;

		head->prev = conn;
		tail->next = conn;
		conn->next = head;
		conn->prev = tail;
	}
}

#if 0
static void cavan_penetrate_service_remove(struct cavan_penetrate_service *service, struct cavan_penetrate_conn *conn)
{
	struct cavan_penetrate_conn *next = conn->next;

	if (next == conn) {
		service->head = NULL;
	} else {
		struct cavan_penetrate_conn *prev = conn->prev;

		prev->next = next;
		next->prev = prev;
		service->head = next;
		conn->next = conn->prev = conn;
	}
}
#endif

static struct cavan_penetrate_conn *cavan_penetrate_service_find(struct cavan_penetrate_service *service, const struct sockaddr_in *addr)
{
	struct cavan_penetrate_conn *head = service->head;
	struct cavan_penetrate_conn *conn;
	in_addr_t host;
	u16 port;

	if (head == NULL) {
		return NULL;
	}

	host = addr->sin_addr.s_addr;
	port = addr->sin_port;
	conn = head;

	while (1) {
		if (conn->addr.sin_addr.s_addr == host && conn->addr.sin_port == port) {
			return conn;
		}

		conn = conn->next;
		if (conn == head) {
			break;
		}
	}

	return NULL;
}

static struct cavan_penetrate_conn *cavan_penetrate_service_find_by_name(struct cavan_penetrate_service *service, const char *name)
{
	struct cavan_penetrate_conn *head = service->head;
	struct cavan_penetrate_conn *conn;

	if (head == NULL) {
		return NULL;
	}

	conn = head;

	while (1) {
		if (strcmp(conn->name, name) == 0) {
			return conn;
		}

		conn = conn->next;
		if (conn == head) {
			break;
		}
	}

	return NULL;
}

int cavan_penetrate_service_init(struct cavan_penetrate_service *service, u16 port)
{
	struct sockaddr_in addr;
	int sockfd;
	int ret;

	sockfd = inet_socket(SOCK_DGRAM);
	if (sockfd < 0) {
		pr_err_info("inet_socket");
		return sockfd;
	}

	inet_sockaddr_init(&addr, NULL, port);

	ret = inet_bind(sockfd, &addr);
	if (ret < 0) {
		pr_err_info("inet_bind");
		close(sockfd);
		return ret;
	}

	service->sockfd = sockfd;
	service->head = NULL;

	return 0;
}

void cavan_penetrate_service_deinit(struct cavan_penetrate_service *service)
{
	close(service->sockfd);
}

void cavan_penetrate_service_run(struct cavan_penetrate_service *service)
{
	struct sockaddr_in addr;
	char cmdline[1024];
	char *argv[100];
	int rdlen;
	int argc;
	int i;

	while (1) {
		const char *command;

		rdlen = inet_recvfrom(service->sockfd, cmdline, sizeof(cmdline), &addr);
		if (rdlen < 0) {
			pr_err_info("inet_recvfrom");
			break;
		}

		inet_show_sockaddr(&addr);

		cmdline[rdlen] = 0;
		println("cmdline[%d] = %s", rdlen, cmdline);

		argc = cavan_cmdline_parse(cmdline, argv, NELEM(argv));

		for (i = 0; i < argc; i++) {
			println("argv[%d] = %s", i, argv[i]);
		}

		command = argv[0];

		if (strcmp(command, "ping") == 0) {
			cavan_penetrate_send_command(service->sockfd, &addr, "pong");
		} else if (strcmp(command, "pong") == 0) {
			struct cavan_penetrate_conn *conn = cavan_penetrate_service_find(service, &addr);
			if (conn != NULL) {
				conn->time = time(NULL);
			}
		} else if (strcmp(command, "link") == 0) {
			if (argc > 1) {
				struct cavan_penetrate_conn *conn = cavan_penetrate_service_find_by_name(service, argv[1]);
				if (conn != NULL) {
					cavan_penetrate_send_command(service->sockfd, &conn->addr, "link %s:%d", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
					cavan_penetrate_send_command(service->sockfd, &addr, "%s:%d", inet_ntoa(conn->addr.sin_addr), ntohs(conn->addr.sin_port));
				}
			}
		} else if (strcmp(command, "login") == 0) {
			if (argc > 1) {
				struct cavan_penetrate_conn *conn = cavan_penetrate_service_find(service, &addr);
				if (conn == NULL) {
					conn = cavan_penetrate_conn_alloc(&addr, argv[1]);
					if (conn != NULL) {
						cavan_penetrate_service_add(service, conn);
					}
				}
			}
		} else if (strcmp(command, "list") == 0) {
			struct cavan_penetrate_conn *head = service->head;
			if (head != NULL) {
				struct cavan_penetrate_conn *conn = head;
				char buff[1024];
				char *p = buff;
				char *p_end = p + sizeof(buff);

				while (1) {
					p += snprintf(p, p_end - p, "%s - %s:%d\n", conn->name, inet_ntoa(conn->addr.sin_addr), ntohs(conn->addr.sin_port));

					conn = conn->next;
					if (conn == head) {
						break;
					}
				}

				inet_sendto(service->sockfd, buff, p - buff, &addr);
			}
		}
	}
}

int cavan_penetrate_client_init(struct cavan_penetrate_client *client)
{
	return 0;
}

void cavan_penetrate_client_deinit(struct cavan_penetrate_client *client)
{
}

int cavan_penetrate_proxy_init(struct cavan_penetrate_proxy *proxy)
{
	return 0;
}

void cavan_penetrate_proxy_deinit(struct cavan_penetrate_proxy *proxy)
{
}
