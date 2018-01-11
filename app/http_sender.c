/*
 * File:		http_sender.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-01-03 21:58:42
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
#include <cavan/file.h>
#include <cavan/http.h>
#include <cavan/timer.h>
#include <cavan/thread.h>
#include <cavan/network.h>

#define HTTP_SENDER_AHEAD			15000
#define HTTP_SENDER_HOST			"game.weixin.qq.com"

struct cavan_http_sender {
	struct network_client client;
	pthread_cond_t cond_write;
	pthread_cond_t cond_read;
	pthread_cond_t cond_exit;
	pthread_mutex_t lock;
	int write_count;
	int read_count;
	bool running;
	bool verbose;
	bool http;
	u64 time;
};

static inline void cavan_http_sender_lock(struct cavan_http_sender *sender)
{
	pthread_mutex_lock(&sender->lock);
}

static inline void cavan_http_sender_unlock(struct cavan_http_sender *sender)
{
	pthread_mutex_unlock(&sender->lock);
}

static inline void cavan_http_sender_wait(struct cavan_http_sender *sender, pthread_cond_t *cond)
{
	pthread_cond_wait(cond, &sender->lock);
}

static inline void cavan_http_sender_post(pthread_cond_t *cond)
{
	pthread_cond_signal(cond);
}

static bool cavan_http_sender_url_init(struct cavan_http_sender *sender, struct network_url *url, char *host)
{
	char buff[1024];
	u16 port;
	char *p;

	p = strchr(host, ':');
	if (p != NULL) {
		port = text2value_unsigned(p + 1, NULL, 10);
		*p = 0;
	} else if (sender->http) {
		port = 80;
	} else {
		port = 443;
	}

	network_url_init(url, sender->http ? "tcp" : "ssl", host, port, NULL);
	println("url = %s", network_url_tostring(url, buff, sizeof(buff), NULL));

	return true;
}

static void *cavan_http_sender_receive_thread(void *data)
{
	struct cavan_http_sender *sender = data;
	struct cavan_http_packet *rsp;
	struct cavan_fifo fifo;
	int ret;

	ret = cavan_fifo_init(&fifo, 4096, &sender->client);
	if (ret < 0) {
		pr_red_info("cavan_fifo_init");
		goto out_pthread_cond_signal;
	}

	fifo.read = network_client_fifo_read;

	rsp = cavan_http_packet_alloc();
	if (rsp == NULL) {
		pr_red_info("cavan_http_packet_alloc");
		goto out_fifo_deinit;
	}

	cavan_http_sender_lock(sender);

	while (sender->running) {
		while (sender->running) {
			if (sender->read_count > 0) {
				sender->read_count--;
				break;
			}

			if (sender->verbose) {
				println("wait for read");
			}

			cavan_http_sender_wait(sender, &sender->cond_read);
		}

		if (!sender->running) {
			break;
		}

		cavan_http_sender_unlock(sender);
		ret = cavan_http_packet_read(rsp, &fifo);
		cavan_http_sender_lock(sender);

		if (ret < 0) {
			pr_red_info("cavan_http_packet_read");
			break;
		}

		if (++sender->write_count == 1) {
			cavan_http_sender_post(&sender->cond_write);
		}

		if (sender->verbose) {
			cavan_string_t *body = &rsp->body;
			cavan_stdout_write_line(body->text, body->length);
		}
	}

	cavan_http_sender_unlock(sender);

	cavan_http_packet_free(rsp);
out_fifo_deinit:
	cavan_fifo_deinit(&fifo);
out_pthread_cond_signal:
	cavan_http_sender_lock(sender);
	sender->running = false;
	pthread_cond_signal(&sender->cond_exit);
	cavan_http_sender_unlock(sender);
	return NULL;
}

static int cavan_http_sender_main_loop(struct cavan_http_sender *sender, struct cavan_http_packet *packets[], int count, char *host)
{
	struct network_client *client = &sender->client;
	struct network_url url;

	if (!cavan_http_sender_url_init(sender, &url, host)) {
		pr_red_info("cavan_http_sender_url_init");
		return -EFAULT;
	}

	while (1) {
		u64 time = clock_gettime_real_ms();
		if (time + HTTP_SENDER_AHEAD < sender->time) {
			char buff[1024];
			int delay;

			delay = sender->time - time;
			time2text_msec(delay, buff, sizeof(buff));
			println("delay = %s", buff);
			msleep(1000);
		} else {
			break;
		}
	}

	while (1) {
		int ret;
		int i;

		while (1) {
			ret = network_client_open(client, &url, 0);
			if (ret < 0) {
				pr_red_info("network_client_open");
				msleep(200);
			} else {
				break;
			}
		}

		println("connect successfull");

		cavan_http_sender_lock(sender);

		sender->write_count = count;
		sender->read_count = 0;

		if (!sender->running) {
			sender->running = true;
			cavan_pthread_run(cavan_http_sender_receive_thread, sender);
		}

		cavan_http_sender_unlock(sender);

		while (1) {
			u64 time = clock_gettime_real_ms();

			if (time < sender->time) {
				int delay = sender->time - time;

				println("delay = %d", delay);

				if (delay > 1000) {
					usleep(1000000);
				} else {
					usleep(delay * 1000);
					break;
				}
			} else {
				break;
			}
		}

		cavan_http_sender_lock(sender);

		i = 0;

		while (sender->running) {
			cavan_string_t *header;

			while (sender->running) {
				if (sender->write_count > 0) {
					sender->write_count--;
					break;
				}

				if (sender->verbose) {
					println("wait for write");
				}

				cavan_http_sender_wait(sender, &sender->cond_write);
			}

			if (!sender->running) {
				break;
			}

			cavan_http_sender_unlock(sender);
			header = &packets[i]->header;
			ret = network_client_send(client, header->text, header->length);
			cavan_http_sender_lock(sender);

			if (ret < 0) {
				pr_red_info("network_client_send");
				break;
			}

			i = (i + 1) % count;

			if (++sender->read_count == 1) {
				cavan_http_sender_post(&sender->cond_read);
			}
		}

		if (sender->running) {
			sender->running = false;
			close(client->sockfd);
			sender->read_count = 1;
			cavan_http_sender_post(&sender->cond_read);
			cavan_http_sender_wait(sender, &sender->cond_exit);
		}

		cavan_http_sender_unlock(sender);

		network_client_close(client);
	}
}

static void cavan_http_sender_show_usage(const char *command)
{
	println("Usage:");
	println("%s [option] URL", command);
	println("-H, -h, --help\t\t%s", cavan_help_message_help);
	println("-V, -v, --verbose\t%s", cavan_help_message_verbose);
	println("-T, -t, --test\t\ttest only");
	println("-D, -d, --delay\t\t%s", cavan_help_message_delay_time);
	println("-N, -n, --now\t\t%s", cavan_help_message_current_time);
}

int main(int argc, char *argv[])
{
	int i;
	int c;
	int ret;
	char *test;
	int count = 0;
	int delay = 0;
	int option_index;
	struct cavan_http_packet *packets[200];
	struct cavan_http_sender sender;
	struct option long_option[] = {
		{
			.name = "help",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_HELP,
		}, {
			.name = "verbose",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_VERBOSE,
		}, {
			.name = "http",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_HTTP,
		}, {
			.name = "time",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_TIME,
		}, {
			.name = "delay",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_DELAY,
		}, {
			.name = "now",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_NOW,
		}, {
			.name = "test",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_TEST,
		}, {
			0, 0, 0, 0
		},
	};

	memset(&sender, 0x00, sizeof(sender));

	pthread_cond_init(&sender.cond_write, NULL);
	pthread_cond_init(&sender.cond_read, NULL);
	pthread_cond_init(&sender.cond_exit, NULL);
	pthread_mutex_init(&sender.lock, NULL);

	sender.time = ((clock_gettime_real_ms() + 3600000 - 1) / 3600000) * 3600000;

	while ((c = getopt_long(argc, argv, "vVhHd:D:tTnN", long_option, &option_index)) != EOF) {
		switch (c) {
		case 'v':
		case 'V':
		case CAVAN_COMMAND_OPTION_VERSION:
			sender.verbose = true;
			break;

		case 'h':
		case 'H':
		case CAVAN_COMMAND_OPTION_HELP:
			cavan_http_sender_show_usage(argv[0]);
			return 0;

		case 'd':
		case 'D':
		case CAVAN_COMMAND_OPTION_DELAY:
			delay = text2value(optarg, NULL, 10);
			break;

		case 'n':
		case 'N':
		case CAVAN_COMMAND_OPTION_NOW:
			sender.time = clock_gettime_real_ms();
			break;

		case CAVAN_COMMAND_OPTION_HTTP:
			sender.http = true;
			break;

		case 't':
		case 'T':
			test = HTTP_SENDER_HOST;
			break;

		case CAVAN_COMMAND_OPTION_TEST:
			test = optarg;
			break;

		default:
			cavan_http_sender_show_usage(argv[0]);
			return -EINVAL;
		}
	}

	println("delay = %d", delay);

	sender.time += delay;

	for (i = optind; i < argc; i++) {
		ret = cavan_http_packet_parse_file(argv[i], packets + count, NELEM(packets) - count);
		if (ret < 0) {
			pr_red_info("cavan_http_sender_load_file");
			return ret;
		}

		count += ret;
	}

	println("count = %d", count);

	if (test != NULL) {
		struct cavan_http_packet req;
		struct cavan_http_packet rsp;
		struct network_client client;
		struct network_url url;
		struct cavan_fifo fifo;

		sender.verbose = true;

		ret = cavan_fifo_init(&fifo, 4096, &client);
		if (ret < 0) {
			pr_red_info("cavan_fifo_init");
			return ret;
		}

		fifo.read = network_client_fifo_read;

		cavan_http_packet_init(&req);
		cavan_http_packet_init(&rsp);

		CAVAN_HTTP_PACKET_ADD_LINE(&req, "GET / HTTP/1.1");
		cavan_http_packet_add_linef(&req, "Host: %s", test);
		CAVAN_HTTP_PACKET_ADD_LINE(&req, "Connection: keep-alive");
		cavan_http_packet_add_line_end(&req);

		cavan_http_sender_url_init(&sender, &url, test);

		ret = network_client_open(&client, &url, 0);
		if (ret < 0) {
			pr_red_info("network_client_open2");
			return ret;
		}

		while (1) {
			cavan_string_t *header = &req.header;
			const char *date;

			ret = network_client_send(&client, header->text, header->length);
			if (ret < 0) {
				pr_red_info("network_client_send");
				break;
			}

			ret = cavan_http_packet_read(&rsp, &fifo);
			if (ret < 0) {
				pr_red_info("cavan_http_packet_read");
				break;
			}

			date = rsp.headers[HTTP_HEADER_DATE];
			if (date != NULL) {
				char buff[1024];

				cavan_http_time_tostring3(buff, sizeof(buff));

				println("remote = %s", date);
				println("local  = %s", buff);
			}

			msleep(100);
		}

		return 0;
	} else if (count > 0) {
		char *host = packets[0]->headers[HTTP_HEADER_HOST];
		if (host == NULL) {
			pr_red_info("host not found");
			return -EINVAL;
		}

		println("host = %s", host);

		for (i = 1; i < count; i++) {
			char *p = packets[i]->headers[HTTP_HEADER_HOST];
			if (p == NULL) {
				pr_red_info("host[%d] not found", i);
				return -EINVAL;
			}

			if (strcmp(host, p) != 0) {
				pr_red_info("host[%d] not mach: %s", i, p);
				return -EINVAL;
			}
		}

		return cavan_http_sender_main_loop(&sender, packets, count, host);
	} else {
		return -EINVAL;
	}
}
