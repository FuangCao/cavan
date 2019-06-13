/*
 * File:		network.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2016-09-18 17:43:29
 *
 * Copyright (c) 2016 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/timer.h>
#include <cavan/network.h>
#include <cavan/service.h>

struct app_network_service {
	struct network_service service;
	struct cavan_simple_cmdline cmdline;
	const char *url;
};

static void *app_network_receive_thread(void *data)
{
	struct network_client *client = data;

	while (1) {
		char buff[40960];
		int rdlen, wrlen;

		rdlen = client->recv(client, buff, sizeof(buff));
		if (rdlen <= 0) {
			pr_red_info("recv: %d", rdlen);
			break;
		}

		wrlen = write(stdout_fd, buff, rdlen);
		if (wrlen < rdlen) {
			pr_red_info("write: %d", wrlen);
			break;
		}

		fsync(stdout_fd);
	}

	network_client_close_socket(client);

	return NULL;
}

static int app_network_cmdline(struct cavan_simple_cmdline *cmdline, struct network_client *client)
{
	int ret;
	pthread_t thread;

	ret = cavan_pthread_create(&thread, app_network_receive_thread, client, true);
	if (ret < 0) {
		pr_err_info("cavan_pthread_create: %d", ret);
		return ret;
	}

	while (1) {
		cavan_string_t *text;
		int wrlen;

		text = cavan_simple_cmdline_readline(cmdline);
		if (text == NULL) {
			break;
		}

		println("send[%d]: %s", text->length, text->text);

		wrlen = client->send(client, text->text, text->length);
		if (wrlen < text->length) {
			break;
		}

		fsync(client->sockfd);
	}

	cavan_pthread_join(thread);

	return 0;
}

static int app_network_dump_main(int argc, char *argv[])
{
	network_protocol_dump();
	return 0;
}

static int app_network_client_main(int argc, char *argv[])
{
	struct cavan_simple_cmdline cmdline;
	struct network_client client;
	int ret;

	assert(argc > 1);

	ret = network_client_open2(&client, argv[1], CAVAN_NET_FLAG_WAIT);
	if (ret < 0) {
		pr_red_info("network_client_open2");
		return ret;
	}

	ret = cavan_simple_cmdline_init(&cmdline);
	if (ret >= 0) {
		ret = app_network_cmdline(&cmdline, &client);
		cavan_simple_cmdline_deinit(&cmdline);
	}

	network_client_close(&client);

	return ret;
}

static int app_network_test_main(int argc, char *argv[])
{
	struct network_client client;
	pthread_t thread;
	int count;
	int ret;
	int i;

	assert(argc > 1);

	ret = network_client_open2(&client, argv[1], CAVAN_NET_FLAG_WAIT);
	if (ret < 0) {
		pr_red_info("network_client_open2");
		return ret;
	}

	if (argc > 2) {
		count = atoi(argv[2]);
	} else {
		count = 10000;
	}

	println("count = %d", count);

	ret = cavan_pthread_create(&thread, app_network_receive_thread, &client, false);
	if (ret < 0) {
		pr_err_info("cavan_pthread_create: %d", ret);
		return ret;
	}

	for (i = 0; i < count; i++) {
		char buff[1024];
		int length;

		length = snprintf(buff, sizeof(buff), "text-%08d\n", i);
		if (network_client_send(&client, buff, length) < length) {
			break;
		}
	}

	while (1) {
		println("send completed");
		msleep(2000);
	}

	network_client_close(&client);

	return ret;
}

static int app_network_start_handler(struct cavan_dynamic_service *service)
{
	struct app_network_service *app_service = cavan_dynamic_service_get_data(service);

	return network_service_open2(&app_service->service, app_service->url, 0);
}

static void app_network_stop_handler(struct cavan_dynamic_service *service)
{
	struct app_network_service *app_service = cavan_dynamic_service_get_data(service);

	network_service_close(&app_service->service);
}

static int app_network_open_connect(struct cavan_dynamic_service *service, void *conn)
{
	struct app_network_service *app_service = cavan_dynamic_service_get_data(service);

	return network_service_accept(&app_service->service, conn, CAVAN_NET_FLAG_NODELAY);
}

static bool app_network_close_connect(struct cavan_dynamic_service *service, void *conn)
{
	network_client_close(conn);
	return false;
}

static int app_network_run_handler(struct cavan_dynamic_service *service, void *conn_data)
{
	struct app_network_service *app_service = cavan_dynamic_service_get_data(service);

	return app_network_cmdline(&app_service->cmdline, conn_data);
}

static int app_network_service_main(int argc, char *argv[])
{
	int ret;
	struct cavan_dynamic_service *service;
	struct app_network_service *app_service;

	assert(argc > 1);

	service = cavan_dynamic_service_create(sizeof(struct app_network_service));
	if (service == NULL) {
		pr_red_info("cavan_dynamic_service_create");
		return -ENOMEM;
	}

	app_service = cavan_dynamic_service_get_data(service);
	app_service->url = argv[1];

	ret = cavan_simple_cmdline_init(&app_service->cmdline);
	if (ret < 0) {
		pr_red_info("cavan_simple_cmdline_init");
		goto out_cavan_dynamic_service_destroy;
	}

	service->min = 20;
	service->max = 1000;
	service->verbose = true;
	service->name = "APP_NETWORK";
	service->conn_size = sizeof(struct network_client);
	service->start = app_network_start_handler;
	service->stop = app_network_stop_handler;
	service->run = app_network_run_handler;
	service->open_connect = app_network_open_connect;
	service->close_connect = app_network_close_connect;

	ret = cavan_dynamic_service_run(service);

out_cavan_dynamic_service_destroy:
	cavan_dynamic_service_destroy(service);
	return ret;
}

static int app_network_test_max_links_main(int argc, char *argv[])
{
	struct network_url url;
	int count = 0;

	assert(argc > 1);

	if (network_url_parse(&url, argv[1]) == NULL) {
		return -EINVAL;
	}

	while (1) {
		struct network_client *client;

		client = malloc(sizeof(struct network_client));
		if (client == NULL) {
			pr_err_info("malloc");
			break;
		}

		while (network_client_open(client, &url, 0) < 0) {
			pr_err_info("network_client_open");
			msleep(500);
		}

		count++;
		println("opened links: %d", count);
	}

	return 0;
}

static void *app_network_test_relink_thread(void *data)
{
	static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
	static u32 delay_min = 0xFFFFFFFF;
	static u32 delay_max;
	static u64 delay_sum;
	static int invalid;
	static int count;

	struct network_url *url = (struct network_url *) data;
	struct network_client client;

	while (1) {
		u64 time_start = clock_gettime_mono_ms();
		u32 delay;

		int ret = network_client_open(&client, url, 0);
		if (ret < 0) {
			pr_err_info("network_client_open");
			msleep(500);
			continue;
		}

		delay = clock_gettime_mono_ms() - time_start;

		pthread_mutex_lock(&lock);

		println("====================");

		println("delay = %d", delay);

		count++;
		println("count = %d", count);

		delay_sum += delay;

		if (delay < delay_min) {
			delay_min = delay;
		}

		if (delay > delay_max) {
			delay_max = delay;
		}

		println("delay_min = %d", delay_min);
		println("delay_max = %d", delay_max);
		println("delay_avg = %d", (u32) (delay_sum / count));

		if (delay > 3000) {
			invalid++;
		}

		println("invalid = %d", invalid);

		pthread_mutex_unlock(&lock);

		network_client_close(&client);
	}

	return NULL;
}

static int app_network_test_relink_main(int argc, char *argv[])
{
	struct network_url url;
	int i;

	assert(argc > 1);

	if (network_url_parse(&url, argv[1]) == NULL) {
		return -EINVAL;
	}

	for (i = 0; i < 100; i++) {
		cavan_pthread_run(app_network_test_relink_thread, &url);
	}

	while (1) {
		msleep(2000);
	}

	return 0;
}

static int app_network_test_dns_main(int argc, char *argv[])
{
	int i;

	for (i = 1; i < argc; i++) {
		const char *hostname = argv[i];
		struct addrinfo *info;
		struct addrinfo hints;
		int ret;

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = 0;

		ret = getaddrinfo(hostname, NULL, &hints, &info);
		if (ret < 0 || info == NULL) {
			pr_error_info("getaddrinfo: %s", gai_strerror(ret));
		} else {
			struct addrinfo *p = info;
			int i;

			for (i = 0; p != NULL; i++) {
				char buff[1024];

				network_sockaddr_tostring(p->ai_addr, buff, sizeof(buff));
				println("%d. %s", i, buff);
				p = p->ai_next;
			}

			freeaddrinfo(info);
		}
	}

	return 0;
}

static int app_network_test_udp_client_main(int argc, char *argv[])
{
	struct sockaddr_in addr;
	int sockfd;
	int count;
	int ret;

	assert(argc > 1);

	if (argc > 2) {
		ret = cavan_stdio_redirect3(argv[2], 0x06);
		if (ret < 0) {
			pr_err_info("cavan_stdio_redirect3");
			return ret;
		}
	}

	sockfd = udp_socket();
	if (sockfd < 0) {
		pr_err_info("udp_socket");
		return sockfd;
	}

	ret = inet_sockaddr_init_url(&addr, argv[1]);
	if (ret < 0) {
		pr_err_info("inet_sockaddr_init_url");
		goto out_close_sockfd;
	}

	ret = inet_sendto(sockfd, "0123456789", 10, &addr);
	if (ret < 0) {
		pr_err_info("inet_sendto");
		goto out_close_sockfd;
	}

	count = 0;

	while (1) {
		char buff[1024];
		int length;

		length = inet_recvfrom(sockfd, buff, sizeof(buff) - 1, &addr);
		if (length <= 0) {
			break;
		}

		buff[length] = 0;
		count++;

		time_println("count = %d", count);
	}

out_close_sockfd:
	close(sockfd);
	return ret;
}

static int app_network_test_udp_service_main(int argc, char *argv[])
{
	struct sockaddr_in addr;
	char buff[1024];
	int length;
	int sockfd;
	int count;

	assert(argc > 1);

	sockfd = inet_create_udp_service(atoi(argv[1]));
	if (sockfd < 0) {
		pr_err_info("inet_create_udp_service");
		return sockfd;
	}

	length = inet_recvfrom(sockfd, buff, sizeof(buff) - 1, &addr);
	if (length < 0) {
		pr_err_info("inet_recvfrom");
		goto out_close_sockfd;
	}

	buff[length] = 0;
	println("buff[%d] = %s", length, buff);

	count = 0;

	while (1) {
		int wrlen = inet_sendto(sockfd, buff, length, &addr);
		if (wrlen < 0) {
			pr_err_info("inet_sendto");
			break;
		}

		count++;
		time_println("count = %d", count);

		msleep(2000);
	}

out_close_sockfd:
	close(sockfd);
	return 0;
}

CAVAN_COMMAND_MAP_START {
	{ "dump", app_network_dump_main },
	{ "client", app_network_client_main },
	{ "service", app_network_service_main },
	{ "test", app_network_test_main },
	{ "test-max-links", app_network_test_max_links_main },
	{ "test-relink", app_network_test_relink_main },
	{ "test-dns", app_network_test_dns_main },
	{ "test-udp-client", app_network_test_udp_client_main },
	{ "test-udp-service", app_network_test_udp_service_main },
} CAVAN_COMMAND_MAP_END;
