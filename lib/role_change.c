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
#define ROLE_CHANGE_KEEPALIVE_DELAY 90
#define ROLE_CHANGE_KEEPALIVE_COUNT	3
#define ROLE_CHANGE_COMMAND_PING	"ping"
#define ROLE_CHANGE_COMMAND_PONG	"pong"

static ssize_t role_change_send_command(struct role_change_conn *conn, const char *fmt, ...)
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

	return network_client_send_packet(&conn->client, command, length);
}

static ssize_t role_change_send_ping(struct network_client *client)
{
	return network_client_send_packet(client, ROLE_CHANGE_COMMAND_PING, sizeof(ROLE_CHANGE_COMMAND_PING) - 1);
}

static ssize_t role_change_send_pong(struct network_client *client)
{
	return network_client_send_packet(client, ROLE_CHANGE_COMMAND_PONG, sizeof(ROLE_CHANGE_COMMAND_PONG) - 1);
}

static void role_change_parse_cmdline(struct role_change_conn *conn, int length)
{
	char *p = conn->command, *p_end;

	pd_info("recv_command[%d] = %s", length, p);

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
		conn->argc++;
	}
}

static int role_change_read_command(struct role_change_conn *conn)
{
	int length = network_client_recv_packet(&conn->client, conn->command, sizeof(conn->command) - 1);

	if (length > 0) {
		conn->command[length] = 0;
		role_change_parse_cmdline(conn, length);
	}

	return length;
}

// ================================================================================

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

	return network_service_accept(&role->service, conn, CAVAN_NET_FLAG_NODELAY);
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

static int role_change_service_add_client(struct cavan_dynamic_service *service, struct role_change_service *role, struct role_change_service_conn *conn)
{
	int ret;
	struct role_change_service_conn *node;

	ret = network_client_get_remote_ip(&conn->conn.client, &conn->addr);
	if (ret < 0) {
		pr_red_info("network_client_get_remote_ip");
		return ret;
	}

#if ROLE_CHANGE_SERVICE_DEBUG
	pd_info("add: addr = %s, name = %s", inet_ntoa(conn->addr), conn->name);
#endif

	conn->time = time(NULL);
	conn->keepalive = 0;

	role_change_service_lock(role);

	for (node = role->head; node; node = node->down) {
		if (node->addr.s_addr == conn->addr.s_addr && strcmp(node->name, conn->name) == 0) {
			struct role_change_service_conn *next = node->next;

			conn->next = next;
			conn->prev = node;
			node->next = conn;

			if (next) {
				next->prev = conn;
			}

			goto out_role_change_service_unlock;
		}
	}

	node = role->head;
	role->head = conn;

	conn->up = NULL;
	conn->down = node;
	conn->prev = NULL;
	conn->next = NULL;

	if (node) {
		node->up = conn;
	}

out_role_change_service_unlock:
	role_change_service_unlock(role);
	cavan_dynamic_service_epoll_add(service, conn->conn.client.sockfd, conn);
	return 0;
}

static bool role_change_service_remove_client_locked(struct cavan_dynamic_service *service, struct role_change_service *role, struct role_change_service_conn *conn)
{
	struct role_change_service_conn *prev, *next;

	if (conn->name == NULL) {
		return false;
	}

#if ROLE_CHANGE_SERVICE_DEBUG
	pd_info("remove: addr = %s, name = %s", inet_ntoa(conn->addr), conn->name);
#endif

	cavan_dynamic_service_epoll_remove(service, conn->conn.client.sockfd);

	prev = conn->prev;
	next = conn->next;

	if (prev) {
		conn->prev = NULL;
		prev->next = next;

		if (next) {
			conn->next = NULL;
			next->prev = prev;
		}

		conn->up = conn->down = NULL;
	} else {
		struct role_change_service_conn *up = conn->up;
		struct role_change_service_conn *down = conn->down;

		if (next) {
			conn->next = NULL;

			next->up = up;
			next->down = down;
			next->prev = NULL;

			if (conn == role->head) {
				role->head = next;
			}

			if (up) {
				conn->up = NULL;
				up->down = next;
			}

			if (down) {
				conn->down = NULL;
				down->up = next;
			}
		} else {
			if (conn == role->head) {
				role->head = down;
			}

			if (up) {
				conn->up = NULL;
				up->down = down;
			}

			if (down) {
				conn->down = NULL;
				down->up = up;
			}
		}
	}

	return true;
}

static bool role_change_service_remove_client(struct cavan_dynamic_service *service, struct role_change_service *role, struct role_change_service_conn *conn)
{
	bool removed;

	role_change_service_lock(role);
	removed = role_change_service_remove_client_locked(service, role, conn);
	role_change_service_unlock(role);

	return removed;
}

static void role_change_service_free_client(struct role_change_service_conn *conn)
{
	network_client_close(&conn->conn.client);
	free(conn->name);
	conn->name = NULL;
	free(conn);
}

static int role_change_service_get_client_count(struct role_change_service_conn *conn)
{
	int count;

	for (count = 0; conn != NULL; conn = conn->next) {
		if (conn->keepalive > 0) {
			continue;
		}

		count++;
	}

	return count;
}

static char *role_change_service_list_clients(struct role_change_service *service, char *buff, size_t size)
{
	struct role_change_service_conn *conn;
	char *buff_end = buff + size;

	role_change_service_lock(service);

	for (conn = service->head; conn; conn = conn->down) {
		int count = role_change_service_get_client_count(conn);
		struct tm time;

		localtime_r(&conn->time, &time);

		buff += snprintf(buff, buff_end - buff, "%04d-%02d-%02d %02d:%02d:%02d %s %s %d\n",
			time.tm_year + 1900, time.tm_mon + 1, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec, inet_ntoa(conn->addr), conn->name, count);
	}

	role_change_service_unlock(service);

	return buff;
}

static struct role_change_service_conn *role_change_service_find_client(struct cavan_dynamic_service *service, struct role_change_service *role, const char *command, const char *name, const char *url, const struct sockaddr_in *peer_addr, int retry)
{
	struct role_change_service_conn *result = NULL;
	struct in_addr addr;

	if (cavan_inet_aton(name, &addr) < 0) {
		addr.s_addr = 0;
	}

	pd_info("addr = %s", inet_ntoa(addr));

	role_change_service_lock(role);

	while (1) {
		struct role_change_service_conn *column = role->head;

		while (column) {
			struct role_change_service_conn *line;

			line = column;
			column = column->down;

			if (addr.s_addr != 0) {
				if (line->addr.s_addr != addr.s_addr) {
					continue;
				}
			} else if (strcmp(line->name, name) != 0) {
				continue;
			}

			while (line) {
				struct role_change_service_conn *conn;
				int ret;

				conn = line;
				line = line->next;

#if ROLE_CHANGE_SERVICE_DEBUG
				pd_info("keepalive = %d", conn->keepalive);
#endif

				if (conn->keepalive > 0) {
					continue;
				}

				role_change_service_remove_client_locked(service, role, conn);

				if (peer_addr != NULL) {
					ret = role_change_send_command(&conn->conn, "%s %s %s:%d",
						command, url, inet_ntoa(peer_addr->sin_addr), ntohs(peer_addr->sin_port));
				} else {
					ret = role_change_send_command(&conn->conn, "%s %s", command, url);
				}

				if (ret < 0) {
					role_change_service_free_client(conn);
				} else {
					result = conn;
					goto out_role_change_service_unlock;
				}
			}
		}

		if (--retry < 0) {
			break;
		}

		role_change_service_unlock(role);

		pd_warn_info("wait client %d", retry);
		msleep(1000);

		role_change_service_lock(role);
	}

out_role_change_service_unlock:
	role_change_service_unlock(role);
	return result;
}

static int role_change_service_process_command(struct cavan_dynamic_service *service, struct role_change_service *role, struct role_change_service_conn *conn)
{
	const char *command;
	int ret;

	if (conn->conn.argc < 1) {
		pr_red_info("invalid command");
		return -EINVAL;
	}

	command = conn->conn.argv[0];

	if (strcmp(command, ROLE_CHANGE_COMMAND_PING) == 0) {
		ret = role_change_send_pong(&conn->conn.client);
		if (ret < 0) {
			pr_red_info("role_change_send_pong");
			return ret;
		}
	} else if (strcmp(command, ROLE_CHANGE_COMMAND_PONG) == 0) {
		role_change_service_lock(role);
		conn->keepalive = 0;
		role_change_service_unlock(role);
		return 0;
	} else if (strcmp(command, "list") == 0) {
		char buff[4096], *text;
		int length;

		length = role_change_service_list_clients(role, buff, sizeof(buff)) - buff;
		if (length > 0) {
			text = buff;
			length--;
		} else {
			text = "none";
			length = 4;
		}

		ret = network_client_send_packet(&conn->conn.client, text, length);
		if (ret < 0) {
			pr_red_info("network_client_send_packet");
			return ret;
		}
	} else if (strcmp(command, "link") == 0) {
		if (conn->conn.argc > 2) {
			struct role_change_service_conn *remote;

			remote = role_change_service_find_client(service, role, command, conn->conn.argv[1], conn->conn.argv[2], NULL, 10);
			if (remote == NULL) {
				pr_red_info("role_change_service_find_client");
				return -EINVAL;
			}

			tcp_proxy_main_loop(&conn->conn.client, &remote->conn.client);
			role_change_service_free_client(remote);
			return 1;
		} else if (conn->conn.argc > 1) {
			struct network_client client;

			ret = network_client_open2(&client, conn->conn.argv[1], 0);
			if (ret < 0) {
				pr_red_info("network_client_open2");
				return -EINVAL;
			}

			tcp_proxy_main_loop(&conn->conn.client, &client);
			network_client_close(&client);
			return 1;
		} else {
			pr_red_info("too a few args");
			return -EINVAL;
		}
	} else if (strcmp(command, "burrow") == 0) {
		struct role_change_service_conn *remote;
		struct sockaddr_in addr;

		ret = network_client_get_remote_addr(&conn->conn.client, (struct sockaddr *) &addr, sizeof(addr));
		if (ret < 0) {
			pr_red_info("role_change_service_find_client");
			return ret;
		}

		remote = role_change_service_find_client(service, role, command, conn->conn.argv[1], conn->conn.argv[2], &addr, 10);
		if (remote == NULL) {
			pr_red_info("role_change_service_find_client");
			return -EINVAL;
		}

		ret = network_client_get_remote_addr(&remote->conn.client, (struct sockaddr *) &addr, sizeof(addr));
		if (ret < 0) {
			pr_red_info("role_change_service_find_client");
			goto out_role_change_service_free_client;
		}

		ret = role_change_send_command(&conn->conn, "%s:%d", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
		if (ret < 0) {
			pr_red_info("role_change_send_command");
			goto out_role_change_service_free_client;
		}

		pr_pos_info();
		msleep(20000);
		pr_pos_info();

out_role_change_service_free_client:
		role_change_service_free_client(remote);
		return ret;
	} else if (strcmp(command, "login") == 0) {
		if (conn->conn.argc > 1) {
			char *name = conn->conn.argv[1];

			if (conn->name != NULL) {
				if (strcmp(conn->name, name) != 0) {
					pr_red_info("conn->name not null: %s", conn->name);
					return -EINVAL;
				}

				return 0;
			}

			conn->name = strdup(name);
			if (conn->name == NULL) {
				pr_err_info("strdup");
				return -ENOMEM;
			}

			ret = role_change_service_add_client(service, role, conn);
			if (ret < 0) {
				free(conn->name);
				conn->name = NULL;
			}

			return 1;
		} else {
			pr_red_info("too a few args");
			return -EINVAL;
		}
	} else {
		pr_red_info("invalid command: %s", command);
		return -EINVAL;
	}

	return 0;
}

static int role_change_service_run_handler(struct cavan_dynamic_service *service, void *conn_data)
{
	struct role_change_service_conn *conn = (struct role_change_service_conn *) conn_data;
	struct role_change_service *role = cavan_dynamic_service_get_data(service);

	conn->name = NULL;
	conn->up = conn->down = NULL;
	conn->prev = conn->next = NULL;

	while (role_change_read_command(&conn->conn) > 0) {
		if (role_change_service_process_command(service, role, conn)) {
			break;
		}
	}

	return 0;
}

static int role_change_service_keepalive_handler(struct cavan_dynamic_service *service)
{
	struct role_change_service *role = cavan_dynamic_service_get_data(service);
	struct role_change_service_conn *column;

	role_change_service_lock(role);

	column = role->head;

	while (column) {
		struct role_change_service_conn *line;

		line = column;
		column = column->down;

		while (line) {
			struct role_change_service_conn *conn;

			conn = line;
			line = line->next;

#if ROLE_CHANGE_SERVICE_DEBUG
			pd_info("keepalive = %d", conn->keepalive);
#endif

			if (conn->keepalive > 0) {
				if (conn->keepalive < ROLE_CHANGE_KEEPALIVE_COUNT) {
					conn->keepalive++;
					continue;
				}
			} else if (role_change_send_ping(&conn->conn.client) > 0) {
				conn->keepalive = 1;
				continue;
			}

			role_change_service_remove_client_locked(service, role, conn);
			role_change_service_free_client(conn);
		}
	}

	role_change_service_unlock(role);

	return ROLE_CHANGE_KEEPALIVE_DELAY;
}

static void role_change_service_epoll_handler(struct cavan_dynamic_service *service, void *conn_data)
{
	struct role_change_service_conn *conn = (struct role_change_service_conn *) conn_data;
	struct role_change_service *role = cavan_dynamic_service_get_data(service);

	if (role_change_read_command(&conn->conn) < 0 || role_change_service_process_command(service, role, conn) < 0) {
		role_change_service_remove_client(service, role, conn);
		role_change_service_free_client(conn);
	}
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
	service->epoll = role_change_service_epoll_handler;
	service->keepalive = role_change_service_keepalive_handler;
	service->open_connect = role_change_service_open_connect;
	service->close_connect = role_change_service_close_connect;

	pthread_mutex_init(&role->lock, NULL);

	return cavan_dynamic_service_run(service);
}

// ================================================================================

static int role_change_client_start_handler(struct cavan_dynamic_service *service)
{
	return 0;
}

static void role_change_client_stop_handler(struct cavan_dynamic_service *service)
{
	pr_pos_info();
}

static int role_change_client_process_command(struct role_change_client *proxy, struct role_change_client_conn *conn)
{
	const char *command;

	if (conn->conn.argc < 1) {
		pr_red_info("invalid command");
		return -EINVAL;
	}

	command = conn->conn.argv[0];

	if (strcmp(command, ROLE_CHANGE_COMMAND_PING) == 0) {
		int ret = role_change_send_pong(&conn->conn.client);
		if (ret < 0) {
			pr_red_info("role_change_send_pong");
			return ret;
		}

		role_change_client_lock(proxy);
		conn->keepalive = 0;
		role_change_client_unlock(proxy);

		return 0;
	} else if (strcmp(command, "link") == 0) {
		conn->mode = ROLE_CHANGE_MODE_LINK;
	} else if (strcmp(command, "link2") == 0) {
		conn->mode = ROLE_CHANGE_MODE_LINK2;
	} else if (strcmp(command, "burrow") == 0) {
		conn->mode = ROLE_CHANGE_MODE_BURROW;
	} else {
		return -EINVAL;
	}

	if (conn->conn.argc > 1) {
		const char *url = conn->conn.argv[1];
		int ret;

		println("url = %s", url);

		ret = network_client_open2(&conn->client, url, 0);
		if (ret < 0) {
			pr_red_info("network_client_open2");

			if (conn->mode == ROLE_CHANGE_MODE_LINK2) {
				ret = role_change_send_command(&conn->conn, "linked %d", ret);
				if (ret < 0) {
					return ret;
				}

				return 0;
			}

			return ret;
		}

		if (conn->mode == ROLE_CHANGE_MODE_LINK2) {
			ret = role_change_send_command(&conn->conn, "linked");
			if (ret < 0) {
				return ret;
			}
		}

		return 1;
	}

	return -EINVAL;
}

static void role_change_client_add_conn(struct role_change_client *client, struct role_change_client_conn *conn)
{
	struct role_change_client_conn *head;

	conn->keepalive = 0;

	role_change_client_lock(client);

	head = client->head;
	client->head = conn;
	conn->next = head;
	conn->prev = NULL;

	if (head) {
		head->prev = conn;
	}

	role_change_client_unlock(client);
}

static void role_change_client_remove_conn(struct role_change_client *client, struct role_change_client_conn *conn)
{

	role_change_client_lock(client);

	if (conn == client->head) {
		client->head = conn->next;
		conn->prev = conn->next = NULL;
	} else {
		struct role_change_client_conn *prev = conn->prev;
		struct role_change_client_conn *next = conn->next;

		if (prev) {
			conn->prev = NULL;
			prev->next = next;
		}

		if (next) {
			conn->next = NULL;
			next->prev = prev;
		}
	}

	role_change_client_unlock(client);
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

		if (role_change_send_command(&conn->conn, "login %s", client->name) > 0) {
			int ret = 0;

			role_change_client_add_conn(client, conn);

			while (role_change_read_command(&conn->conn) > 0) {
				ret = role_change_client_process_command(client, conn);
				if (ret != 0) {
					break;
				}
			}

			role_change_client_remove_conn(client, conn);

			if (ret > 0) {
				return 0;
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

static int role_change_client_burrow(struct network_client *conn, struct network_client *client, const char *url_text)
{
	struct network_service service;
	struct network_url url;
	int sockfd;
	int ret;

	println("url_text = %s", url_text);

	sockfd = inet_bind_dup(conn->sockfd, SOCK_STREAM);
	if (sockfd < 0) {
		pr_err_info("inet_bind_dup");
		return sockfd;
	}

	service.sockfd = sockfd;

	sockfd = inet_bind_dup(conn->sockfd, SOCK_STREAM);
	if (sockfd < 0) {
		pr_err_info("inet_bind_dup");
		return sockfd;
	}

	client->sockfd = sockfd;

	ret = setsockopt_send_timeout(sockfd, 500);
	if (ret < 0) {
		pr_err_info("setsockopt_send_timeout");
	}

	ret = setsockopt_recv_timeout(sockfd, 500);
	if (ret < 0) {
		pr_err_info("setsockopt_recv_timeout");
	}

	network_url_parse(&url, url_text);

	ret = network_service_open(&service, &url, CAVAN_NET_FLAG_BOUND);
	if (ret < 0) {
		pr_red_info("network_service_open");
		return ret;
	}

	while (1) {
		int ret = network_client_open(client, &url, CAVAN_NET_FLAG_BOUND);
		if (ret < 0) {
			pr_err_info("network_client_open");

			if (network_service_readable(&service)) {
				ret = network_service_accept(&service, client, 0);
				if (ret < 0) {
					pr_err_info("network_service_accept");
				} else {
					close(sockfd);
					break;
				}
			}
		} else {
			break;
		}

		msleep(200);
	}

	network_service_close(&service);

	return ret;
}

static int role_change_client_run_handler(struct cavan_dynamic_service *service, void *conn_data)
{
	struct role_change_client_conn *conn = conn_data;

	if (conn->mode == ROLE_CHANGE_MODE_BURROW) {
		struct network_client client;
		const char *url;
		int ret;

		if (conn->conn.argc > 2) {
			url = conn->conn.argv[2];
		} else {
			return -EINVAL;
		}

		ret = role_change_client_burrow(&conn->conn.client, &client, url);
		if (ret < 0) {
			pr_red_info("role_change_client_burrow");
			return ret;
		}

		tcp_proxy_main_loop(&client, &conn->client);
		network_client_close(&client);
	} else {
		tcp_proxy_main_loop(&conn->client, &conn->conn.client);
	}

	return 0;
}

static int role_change_client_keepalive_handler(struct cavan_dynamic_service *service)
{
	struct role_change_client *client = cavan_dynamic_service_get_data(service);
	struct role_change_client_conn *conn;

	role_change_client_lock(client);

	for (conn = client->head; conn; conn = conn->next) {
#if ROLE_CHANGE_CLIENT_DEBUG
		pd_info("keepalive = %d", conn->keepalive);
#endif

		if (conn->keepalive < ROLE_CHANGE_KEEPALIVE_COUNT) {
			conn->keepalive++;
		} else {
			network_client_close(&conn->conn.client);
		}
	}

	role_change_client_unlock(client);

	return ROLE_CHANGE_KEEPALIVE_DELAY;
}

int role_change_client_run(struct cavan_dynamic_service *service)
{
	struct role_change_client *proxy = cavan_dynamic_service_get_data(service);

	pd_bold_info("URL = %s", network_url_tostring(&proxy->url, NULL, 0, NULL));
	pd_bold_info("NAME = %s", proxy->name);

	proxy->head = NULL;
	service->name = "ROLE_CHANGE_CLIENT";
	service->conn_size = sizeof(struct role_change_client_conn);
	service->start = role_change_client_start_handler;
	service->stop = role_change_client_stop_handler;
	service->run = role_change_client_run_handler;
	service->open_connect = role_change_client_open_connect;
	service->close_connect = role_change_client_close_connect;
	service->keepalive = role_change_client_keepalive_handler;

	pthread_mutex_init(&proxy->lock, NULL);

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

	return network_service_accept(&proxy->service, conn, CAVAN_NET_FLAG_NODELAY);
}

static bool role_change_proxy_close_connect(struct cavan_dynamic_service *service, void *conn)
{
	network_client_close(conn);
	return false;
}

static int role_change_proxy_run_handler(struct cavan_dynamic_service *service, void *conn)
{
	int ret;
	struct role_change_conn conn_proxy;
	struct role_change_proxy *proxy = cavan_dynamic_service_get_data(service);

	ret = network_client_open(&conn_proxy.client, &proxy->url_remote, 0);
	if (ret < 0) {
		pr_red_info("network_client_open");
		return ret;
	}

	if (proxy->burrow) {
		if (proxy->name == NULL) {
			pr_red_info("burrow name not set");
			return -EINVAL;
		}

		ret = role_change_send_command(&conn_proxy, "burrow %s %s", proxy->name, proxy->url);
		if (ret < 0) {
			pr_red_info("role_change_send_command");
			goto out_network_client_close;
		}

		ret = role_change_read_command(&conn_proxy);
		if (ret < 0) {
			pr_red_info("role_change_send_command");
			goto out_network_client_close;
		}

		ret = role_change_client_burrow(&conn_proxy.client, &conn_proxy.client, conn_proxy.argv[0]);
	} else if (proxy->name) {
		ret = role_change_send_command(&conn_proxy, "link %s %s", proxy->name, proxy->url);
	} else {
		ret = role_change_send_command(&conn_proxy, "link %s", proxy->url);
	}

	if (ret < 0) {
		pr_red_info("role_change_send_command");
		goto out_network_client_close;
	}

	ret = tcp_proxy_main_loop(conn, &conn_proxy.client);

out_network_client_close:
	network_client_close(&conn_proxy.client);
	return ret;
}

int role_change_proxy_run(struct cavan_dynamic_service *service)
{
	struct role_change_proxy *proxy = cavan_dynamic_service_get_data(service);

	pd_bold_info("LOCAL_URL = %s", network_url_tostring(&proxy->url_local, NULL, 0, NULL));
	pd_bold_info("REMOTE_URL = %s", network_url_tostring(&proxy->url_remote, NULL, 0, NULL));
	pd_bold_info("NAME = %s", proxy->name);
	pd_bold_info("URL = %s", proxy->url);

	service->name = "ROLE_CHANGE_PROXY";
	service->conn_size = sizeof(struct network_client);
	service->start = role_change_proxy_start_handler;
	service->stop = role_change_proxy_stop_handler;
	service->run = role_change_proxy_run_handler;
	service->open_connect = role_change_proxy_open_connect;
	service->close_connect = role_change_proxy_close_connect;

	return cavan_dynamic_service_run(service);
}
