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

	while (1) {
		char buff[1024];
		int length;

		length = inet_recvfrom(sockfd, buff, sizeof(buff) - 1, &addr);
		if (length <= 0) {
			break;
		}

		length = inet_sendto(sockfd, buff, length, &addr);
		if (length < 0) {
			break;
		}

		buff[length] = 0;
		time_println(buff);
	}

out_close_sockfd:
	close(sockfd);
	return ret;
}

static int app_network_test_udp_service_main(int argc, char *argv[])
{
	struct sockaddr_in addr;
	char buff[1024];
	int sockfd;
	int pkg_count;
	int err_count;

	assert(argc > 1);

	sockfd = inet_create_udp_service(atoi(argv[1]));
	if (sockfd < 0) {
		pr_err_info("inet_create_udp_service");
		return sockfd;
	}

	err_count = 0;
	pkg_count = 0;

	while (1) {
		int length = inet_recvfrom_timeout(sockfd, buff, sizeof(buff), &addr, 2000);

		if (length > 0) {
			pkg_count++;
		} else if (pkg_count > 0) {
			err_count++;
		}

		if (pkg_count > 0) {
			length = snprintf(buff, sizeof(buff), "pkg = %d, err = %d", pkg_count, err_count);

			int wrlen = inet_sendto(sockfd, buff, length, &addr);
			if (wrlen < 0) {
				pr_err_info("inet_sendto");
				break;
			}
		}

		time_println("pkg = %d, err = %d", pkg_count, err_count);
		msleep(2000);
	}

	close(sockfd);

	return 0;
}

static void *app_network_ime_receive_handler(void *data)
{
	struct network_client_lock *lock = (struct network_client_lock *) data;
	char line[1024];

	while (1)
	{
		struct network_client *client;
		int length;

		client = network_client_lock_read_acquire(lock);
		length = network_client_recv(client, line, sizeof(line) - 1);
		network_client_lock_read_release(lock);

		if (length <= 0) {
			break;
		}

		line[length] = 0;

		if (strcasecmp(line, "PING") == 0) {
			client = network_client_lock_write_acquire(lock);
			length = network_client_send_text(client, "PONG");
			network_client_lock_write_release(lock);

			if (length < 0) {
				break;
			}
		} else {
			puts(line);
		}
	}

	pr_red_info("ime receive thread exit!");

	return NULL;
}

static int app_network_ime_login(struct network_client *client)
{
	char hostname[1024];

	network_get_hostname(hostname, sizeof(hostname));

	return network_client_printf(client, "USER %s", hostname);
}

static int app_network_ime_main(int argc, char *argv[])
{
	struct network_client_lock lock;
	struct network_client client;
	int ret;

	assert(argc > 1);

	network_client_lock_init(&lock, &client);

	ret = network_client_open2(&client, argv[1], CAVAN_NET_FLAG_PACK);
	if (ret < 0) {
		pr_err_info("network_client_open2: %d", ret);
		return ret;
	}

	ret = app_network_ime_login(&client);
	if (ret < 0) {
		pr_err_info("app_network_ime_login: %d", ret);
		goto out_network_client_close;
	}

	ret = cavan_pthread_create(NULL, app_network_ime_receive_handler, &lock, false);
	if (ret < 0) {
		pr_err_info("cavan_pthread_create: %d", ret);
		goto out_network_client_close;
	}

	if (argc > 2) {
		const char *command = argv[2];

		if (strcmp(command, "sign") == 0) {
			struct timespec ts;
			int ahead;
			u64 time;

			command = "signin";

			clock_gettime_real(&ts);
			time = ((cavan_timespec_mseconds(&ts) + 3600000 - 1) / 3600000) * 3600000;

			if (argc > 3) {
				ahead = atoi(argv[3]);
			} else {
				ahead = -1000;
			}

			println("ahead = %d", ahead);

			time += ahead;

			while (1) {
				struct tm tm;
				u64 mseconds;

				clock_gettime_real(&ts);
				localtime_r(&ts.tv_sec, &tm);

				println("time  = %02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);

				mseconds = cavan_timespec_mseconds(&ts);
				if (mseconds < time) {
					int delay = time - mseconds;

					if (delay < 1000) {
						msleep(delay);
					} else {
						char buff[1024];

						time2text_msec(delay, buff, sizeof(buff));
						println("delay = %s", buff);
						msleep(1000 - (mseconds % 1000));
					}
				} else {
					break;
				}
			}
		}

		network_client_lock_write_acquire(&lock);
		network_client_printf(&client, "broadcast %s", command);
		network_client_lock_write_release(&lock);
	} else {
		while (true) {
			char buff[1024];
			char *line;
			int length;

			print("> ");

			if (!fgets(buff, sizeof(buff) - 1, stdin)) {
				break;
			}

			length = strlen(buff);
			line = cavan_string_trim(buff, &length);

			if (length > 0) {
				network_client_lock_write_acquire(&lock);
				network_client_send(&client, line, length);
				network_client_lock_write_release(&lock);
			}
		}
	}

	msleep(200);

out_network_client_close:
	network_client_close(&client);
	network_client_lock_deinit(&lock);
	return ret;
}

CAVAN_COMMAND_MAP_START {
	{ "ime", app_network_ime_main },
	{ "test", app_network_test_main },
	{ "dump", app_network_dump_main },
	{ "client", app_network_client_main },
	{ "service", app_network_service_main },
	{ "test-max-links", app_network_test_max_links_main },
	{ "test-relink", app_network_test_relink_main },
	{ "test-dns", app_network_test_dns_main },
	{ "test-udp-client", app_network_test_udp_client_main },
	{ "test-udp-service", app_network_test_udp_service_main },
} CAVAN_COMMAND_MAP_END;
