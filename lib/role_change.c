/*
 * File:		role_change.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2017-12-15 19:07:42
 *
 * Copyright (c) 2017 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/tcp_proxy.h>
#include <cavan/role_change.h>

#define ROLE_CHANGE_SERVICE_DEBUG	1
#define ROLE_CHANGE_CLIENT_DEBUG	1

static ssize_t role_change_send_command(struct network_client *client, const char *fmt, ...)
{
	char command[1024];
	int length;
	va_list ap;

	va_start(ap, fmt);
	length = vsnprintf(command, sizeof(command), fmt, ap);
	va_end(ap);

#if ROLE_CHANGE_SERVICE_DEBUG || ROLE_CHANGE_CLIENT_DEBUG
	println("send_command[%d] = %s", length, command);
#endif

	return network_client_send_packet(client, command, length);
}

static void role_change_service_parse_cmdline(struct role_change_conn *conn, size_t length)
{
	char *p = conn->command, *p_end;

	conn->argv[0] = p;
	conn->argc = 0;

	for (p_end = p + length; p < p_end; p++) {
		switch (*p) {
		case ' ':
		case '\t':
		case '\r':
		case '\n':
		case '\f':
			*p = 0;
		case 0:
			if (p > conn->argv[conn->argc]) {
				conn->argc++;
			}

			conn->argv[conn->argc] = p + 1;
			break;
		}
	}

	if (p > conn->argv[conn->argc]) {
		if (p < conn->command + sizeof(conn->command)) {
			*p = 0;
		}

		conn->argc++;
	}

#if ROLE_CHANGE_SERVICE_DEBUG || ROLE_CHANGE_CLIENT_DEBUG
	{
		int i;

		for (i = 0; i < conn->argc; i++) {
			println("argv[%d] = %s", i, conn->argv[i]);
		}
	}
#endif
}

static int role_change_read_command(struct role_change_conn *conn)
{
	ssize_t rdlen = network_client_recv_packet(&conn->client, conn->command, sizeof(conn->command));
	if (rdlen > 0) {
		role_change_service_parse_cmdline(conn, rdlen);
	}

	return rdlen;
}

static int role_change_service_start_handler(struct cavan_dynamic_service *service)
{
	struct role_change_service *role = cavan_dynamic_service_get_data(service);

	return network_service_open(&role->service, &role->url, 0);
}

static void role_change_service_stop_handler(struct cavan_dynamic_service *service)
{
	struct role_change_service *role = cavan_dynamic_service_get_data(service);

	network_service_close(&role->service);
}

static int role_change_service_open_connect(struct cavan_dynamic_service *service, void *conn)
{
	struct role_change_service *role = cavan_dynamic_service_get_data(service);

	return network_service_accept(&role->service, conn);
}

static bool role_change_service_close_connect(struct cavan_dynamic_service *service, void *data)
{
	struct role_change_service_conn *conn = data;

	if (conn->name) {
		return true;
	}

	network_client_close(&conn->conn.client);

	return false;
}

static int role_change_service_add_client(struct role_change_service *service, struct role_change_service_conn *conn)
{
	int ret;
	struct role_change_service_conn *p;

	ret = network_client_get_remote_ip(&conn->conn.client, &conn->addr);
	if (ret < 0) {
		pr_red_info("network_client_get_remote_ip");
		return ret;
	}

#if ROLE_CHANGE_SERVICE_DEBUG
	pd_info("addr = %s, name = %s", inet_ntoa(conn->addr), conn->name);
#endif

	role_change_service_lock(service);

	for (p = service->head; p; p = p->down) {
		if (p->addr.s_addr == conn->addr.s_addr && strcmp(p->name, conn->name) == 0) {
			struct role_change_service_conn *right = p->right;

			conn->right = right;
			if (right != NULL) {
				right->left = conn;
			}

			conn->left = p;
			p->right = conn;
			goto out_role_change_service_unlock;
		}
	}

	p = service->head;
	service->head = conn;

	conn->down = p;
	if (p != NULL) {
		p->up = conn;
	}

	conn->up = NULL;
	conn->left = NULL;
	conn->right = NULL;

out_role_change_service_unlock:
	role_change_service_unlock(service);
	return 0;
}

static void role_change_service_remove_client_locked(struct role_change_service *service, struct role_change_service_conn *conn)
{
	struct role_change_service_conn *left, *right;

	left = conn->left;
	right = conn->right;

#if ROLE_CHANGE_SERVICE_DEBUG
	println("left = %p", left);
	println("right = %p", right);
#endif

	if (left == NULL) {
		struct role_change_service_conn *up = conn->up;
		struct role_change_service_conn *down = conn->down;

#if ROLE_CHANGE_SERVICE_DEBUG
		println("up = %p", up);
		println("down = %p", down);
#endif

		if (right != NULL) {
			struct role_change_service_conn *up = conn->up;
			struct role_change_service_conn *down = conn->down;

			right->up = up;
			right->down = down;
			right->left = NULL;

			if (up != NULL) {
				up->down = right;
			} else {
				service->head = right;
			}

			if (down != NULL) {
				down->up = right;
			}
		} else {
			if (up != NULL) {
				up->down = down;
			} else {
				service->head = down;
			}

			if (down != NULL) {
				down->up = up;
			}
		}
	} else {
		left->right = right;

		if (right != NULL) {
			right->left = left;
		}
	}
}

static void role_change_service_free_client(struct role_change_service_conn *conn)
{
	network_client_close(&conn->conn.client);
	free(conn->name);
	free(conn);
}

static char *role_change_service_list_clients(struct role_change_service *service, char *buff, size_t size)
{
	struct role_change_service_conn *conn;
	char *buff_end = buff + size;

	role_change_service_lock(service);

	for (conn = service->head; conn; conn = conn->down) {
		buff += snprintf(buff, buff_end - buff, "%s %s\n", inet_ntoa(conn->addr), conn->name);
	}

	role_change_service_unlock(service);

	return buff;
}

static struct role_change_service_conn *role_change_service_find_client(struct role_change_service *service, const char *name, const char *url)
{
	struct role_change_service_conn *conn;

#if ROLE_CHANGE_SERVICE_DEBUG
	println("name = %s", name);
	println("url = %s", url);
#endif

	role_change_service_lock(service);

	for (conn = service->head; conn; conn = conn->down) {
		if (strcmp(conn->name, name) == 0) {
			role_change_service_remove_client_locked(service, conn);

			if (role_change_send_command(&conn->conn.client, "link %s", url) < 0) {
				role_change_service_free_client(conn);
			} else {
				break;
			}
		}
	}

	role_change_service_unlock(service);

	return conn;
}

static bool role_change_service_process_cmdline(struct role_change_service *service, struct role_change_service_conn *conn)
{
	const char *command = conn->conn.argv[0];

#if ROLE_CHANGE_SERVICE_DEBUG
	pd_info("command = %s", command);
#endif

	if (strcmp(command, "list") == 0) {
		char buff[4096], *text;
		int length;

		length = role_change_service_list_clients(service, buff, sizeof(buff)) - buff;
		if (length > 0) {
			text = buff;
		} else {
			text = "empty";
			length = 5;
		}

		if (network_client_send_packet(&conn->conn.client, text, length) < 0) {
			pr_red_info("network_client_send_packet");
			return true;
		}
	} else if (strcmp(command, "link") == 0) {
		if (conn->conn.argc > 2) {
			struct role_change_service_conn *remote;

			remote = role_change_service_find_client(service, conn->conn.argv[1], conn->conn.argv[2]);
			if (remote == NULL) {
				pr_red_info("role_change_service_find_client");
				return true;
			}

			tcp_proxy_main_loop(&conn->conn.client, &remote->conn.client);
			role_change_service_free_client(remote);
		}

		return true;
	} else if (strcmp(command, "login") == 0) {
		if (conn->conn.argc > 1) {
			char *name = conn->conn.argv[1];

			if (conn->name != NULL) {
				pr_red_info("conn->name not null: %s", conn->name);
				return strcmp(conn->name, name) != 0;
			}

			conn->name = strdup(name);
			if (conn->name && role_change_service_add_client(service, conn) < 0) {
				free(conn->name);
				conn->name = NULL;
			}
		}

		return true;
	} else {
		return true;
	}

	return false;
}

static int role_change_service_run_handler(struct cavan_dynamic_service *service, void *conn_data)
{
	struct role_change_service_conn *conn = (struct role_change_service_conn *) conn_data;
	struct role_change_service *role = cavan_dynamic_service_get_data(service);

	conn->name = NULL;
	conn->up = conn->down = NULL;
	conn->left = conn->right = NULL;

	while (role_change_read_command(&conn->conn) > 0) {
		if (conn->conn.argc > 0 && role_change_service_process_cmdline(role, conn)) {
			break;
		}
	}

	return 0;
}

static int role_change_service_keepalive_handler(struct cavan_dynamic_service *service)
{
	struct role_change_service *role = cavan_dynamic_service_get_data(service);
	struct role_change_service_conn *column = role->head;

	role_change_service_lock(role);

	while (column) {
		struct role_change_service_conn *line;

		line = column;
		column = column->down;

		while (line) {
			struct role_change_service_conn *conn;

			conn = line;
			line = line->right;

			if (network_client_send_packet(&conn->conn.client, "keepalive", 9) < 0) {
				role_change_service_remove_client_locked(role, conn);
				role_change_service_free_client(conn);
			}
		}
	}

	role_change_service_unlock(role);

	return 60000;
}

int role_change_service_run(struct cavan_dynamic_service *service)
{
	struct role_change_service *role = cavan_dynamic_service_get_data(service);

	pd_bold_info("URL = %s", network_url_tostring(&role->url, NULL, 0, NULL));

	service->name = "ROLE_CHANGE_SERVICE";
	service->conn_size = sizeof(struct role_change_service_conn);
	service->start = role_change_service_start_handler;
	service->stop = role_change_service_stop_handler;
	service->run = role_change_service_run_handler;
	service->keepalive = role_change_service_keepalive_handler;
	service->open_connect = role_change_service_open_connect;
	service->close_connect = role_change_service_close_connect;

	pthread_mutex_init(&role->lock, NULL);

	return cavan_dynamic_service_run(service);
}

// ================================================================================

static int role_change_client_start_handler(struct cavan_dynamic_service *service)
{
	pr_pos_info();
	return 0;
}

static void role_change_client_stop_handler(struct cavan_dynamic_service *service)
{
	pr_pos_info();
}

static int role_change_client_process_command(struct role_change_client *proxy, struct role_change_client_conn *conn)
{
	const char *command = conn->conn.argv[0];

#if ROLE_CHANGE_CLIENT_DEBUG
	println("command = %s", command);
#endif

	if (strcmp(command, "keepalive") == 0) {
		return 0;
	}

	if (strcmp(command, "link") == 0) {
		if (conn->conn.argc > 1) {
			const char *url = conn->conn.argv[1];
			int ret;

			println("url = %s", url);

			ret = network_client_open2(&conn->client, url, 0);
			if (ret < 0) {
				pr_red_info("network_client_open2");
				return ret;
			}

			return 1;
		}
	}

	return -EINVAL;
}

static int role_change_client_open_connect(struct cavan_dynamic_service *service, void *conn_data)
{
	struct role_change_client *client = cavan_dynamic_service_get_data(service);
	struct role_change_client_conn *conn = conn_data;

	while (1) {
		int ret = network_client_open(&conn->conn.client, &client->url, 0);
		if (ret < 0) {
			pr_red_info("network_client_open");
			msleep(2000);
			continue;
		}

		if (role_change_send_command(&conn->conn.client, "login %s", client->name) > 0) {
			while (role_change_read_command(&conn->conn) > 0) {
				if (conn->conn.argc > 0) {
					int ret = role_change_client_process_command(client, conn);
					if (ret < 0) {
						break;
					}

					if (ret > 0) {
						return 0;
					}
				}
			}
		}

		network_client_close(&conn->conn.client);
	}

	return 0;
}

static bool role_change_client_close_connect(struct cavan_dynamic_service *service, void *conn_data)
{
	struct role_change_client_conn *conn = conn_data;
	network_client_close(&conn->conn.client);
	network_client_close(&conn->client);
	return false;
}

static int role_change_client_run_handler(struct cavan_dynamic_service *service, void *conn_data)
{
	struct role_change_client_conn *conn = conn_data;
	tcp_proxy_main_loop(&conn->client, &conn->conn.client);
	return 0;
}

int role_change_client_run(struct cavan_dynamic_service *service)
{
	struct role_change_client *proxy = cavan_dynamic_service_get_data(service);

	pd_bold_info("URL = %s", network_url_tostring(&proxy->url, NULL, 0, NULL));
	pd_bold_info("NAME = %s", proxy->name);

	service->name = "ROLE_CHANGE_CLIENT";
	service->conn_size = sizeof(struct role_change_client_conn);
	service->start = role_change_client_start_handler;
	service->stop = role_change_client_stop_handler;
	service->run = role_change_client_run_handler;
	service->open_connect = role_change_client_open_connect;
	service->close_connect = role_change_client_close_connect;

	return cavan_dynamic_service_run(service);
}

// ================================================================================

static int role_change_proxy_start_handler(struct cavan_dynamic_service *service)
{
	struct role_change_proxy *proxy = cavan_dynamic_service_get_data(service);

	return network_service_open(&proxy->service, &proxy->url_local, 0);
}

static void role_change_proxy_stop_handler(struct cavan_dynamic_service *service)
{
	struct role_change_proxy *proxy = cavan_dynamic_service_get_data(service);

	network_service_close(&proxy->service);
}

static int role_change_proxy_open_connect(struct cavan_dynamic_service *service, void *conn)
{
	struct role_change_proxy *proxy = cavan_dynamic_service_get_data(service);

	return network_service_accept(&proxy->service, conn);
}

static bool role_change_proxy_close_connect(struct cavan_dynamic_service *service, void *conn)
{
	network_client_close(conn);
	return false;
}

static int role_change_proxy_run_handler(struct cavan_dynamic_service *service, void *conn)
{
	int ret;
	struct network_client client_proxy;
	struct role_change_proxy *proxy = cavan_dynamic_service_get_data(service);

	ret = network_client_open(&client_proxy, &proxy->url_remote, 0);
	if (ret < 0) {
		pr_red_info("network_client_open");
		return ret;
	}

	ret = role_change_send_command(&client_proxy, "link %s %s", proxy->name, proxy->url);
	if (ret < 0) {
		pr_red_info("role_change_send_command");
		goto out_network_client_close;
	}

	ret = tcp_proxy_main_loop(conn, &client_proxy);

out_network_client_close:
	network_client_close(&client_proxy);
	return ret;
}

int role_change_proxy_run(struct cavan_dynamic_service *service)
{
	struct role_change_proxy *proxy = cavan_dynamic_service_get_data(service);

	pd_bold_info("LOCAL_URL = %s", network_url_tostring(&proxy->url_local, NULL, 0, NULL));
	pd_bold_info("REMOTE_URL = %s", network_url_tostring(&proxy->url_remote, NULL, 0, NULL));

	service->name = "ROLE_CHANGE_PROXY";
	service->conn_size = sizeof(struct network_client);
	service->start = role_change_proxy_start_handler;
	service->stop = role_change_proxy_stop_handler;
	service->run = role_change_proxy_run_handler;
	service->open_connect = role_change_proxy_open_connect;
	service->close_connect = role_change_proxy_close_connect;

	return cavan_dynamic_service_run(service);
}
