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

#define HTTP_SENDER_AHEAD			10000
#define HTTP_SENDER_REPEAT			60
#define HTTP_SENDER_HOST			"game.weixin.qq.com"
#define HTTP_SENDER_SEND_MIN		3
#define HTTP_SENDER_CONN_COUNT		4
#define HTTP_SENDER_QUEUE_SIZE		4
#define HTTP_SENDER_PACKAGES		200

struct cavan_http_sender;

struct cavan_http_client {
	struct network_client client;
	struct cavan_http_sender *sender;
	pthread_cond_t cond_exit;
	bool running;

	u8 packets[HTTP_SENDER_QUEUE_SIZE];
	u8 head;
	u8 tail;
};

struct cavan_http_sender {
	struct cavan_http_client clients[HTTP_SENDER_CONN_COUNT];
	bool status[HTTP_SENDER_PACKAGES];
	int counts[HTTP_SENDER_PACKAGES];
	pthread_cond_t cond_write;
	struct network_url url;
	pthread_mutex_t lock;
	int conn_count;
	bool running;
	bool verbose;
	bool daemon;
	bool http;
	u32 repeat;
	u64 time;
};

static void *cavan_http_sender_receive_thread(void *data);

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

static bool cavan_http_sender_url_init(struct cavan_http_sender *sender, char *host)
{
	struct network_url *url = &sender->url;
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

static void cavan_http_client_init(struct cavan_http_client *client)
{
	pthread_cond_init(&client->cond_exit, NULL);
	client->running = false;
}

static int cavan_http_client_open(struct cavan_http_client *client, struct cavan_http_sender *sender)
{
	int count = 0;

	while (1) {
		int ret = network_client_open(&client->client, &sender->url, CAVAN_NET_FLAG_NODELAY);
		if (ret < 0) {
			pr_red_info("network_client_open");

			if (++count > 3) {
				return ret;
			}
		} else {
			break;
		}

		msleep(100);
	}

	client->sender = sender;
	client->head = client->tail = 0;

	if (!client->running) {
		client->running = true;
		cavan_pthread_run(cavan_http_sender_receive_thread, client);
	}

	return 0;
}

static void cavan_http_client_close(struct cavan_http_client *client)
{
	int i;

	for (i = 20; i > 0 && client->running; i--) {
		if (client->head == client->tail) {
			break;
		}

		cavan_http_sender_unlock(client->sender);
		println("wait recv complete: %d", i);
		msleep(100);
		cavan_http_sender_lock(client->sender);
	}

	if (client->running) {
		client->running = false;
		network_client_shutdown(&client->client);
		cavan_http_sender_wait(client->sender, &client->cond_exit);
	}

	network_client_close(&client->client);
}

static inline bool cavan_http_client_writeable(struct cavan_http_client *client)
{
	return client->running && (((client->tail + 1) % HTTP_SENDER_QUEUE_SIZE) != client->head);
}

static void cavan_http_client_enqueue(struct cavan_http_client *client, u8 value)
{
	client->packets[client->tail] = value;
	client->tail = (client->tail + 1) % HTTP_SENDER_QUEUE_SIZE;
}

static int cavan_http_client_dequeue(struct cavan_http_client *client)
{
	int value;

	if (client->head == client->tail) {
		return -1;
	}

	value = client->packets[client->head];
	client->head = (client->head + 1) % HTTP_SENDER_QUEUE_SIZE;

	if (((client->tail + 2) % HTTP_SENDER_QUEUE_SIZE) == client->head) {
		cavan_http_sender_post(&client->sender->cond_write);
	}

	return value;
}

static void cavan_http_sender_init(struct cavan_http_sender *sender)
{
	int i;

	memset(sender, 0x00, sizeof(*sender));
	pthread_mutex_init(&sender->lock, NULL);
	pthread_cond_init(&sender->cond_write, NULL);

	for (i = 0; i < HTTP_SENDER_CONN_COUNT; i++) {
		cavan_http_client_init(sender->clients + i);
	}
}

static int cavan_http_sender_open(struct cavan_http_sender *sender, int count)
{
	int i;

	if (count > HTTP_SENDER_CONN_COUNT) {
		count = HTTP_SENDER_CONN_COUNT;
	}

	for (i = 0; i < count; i++) {
		struct cavan_http_client *client = sender->clients + i;
		int ret;

		ret = cavan_http_client_open(client, sender);
		if (ret < 0) {
			pr_red_info("cavan_http_client_open");
			break;
		}
	}

	return i;
}

static void cavan_http_sender_close(struct cavan_http_sender *sender, int count)
{
	int i;

	for (i = 0; i < count; i++) {
		cavan_http_client_close(sender->clients + i);
	}
}

static int cavan_http_sender_get_client(struct cavan_http_sender *sender, int index, int count)
{
	int i;

	for (i = index; i < count; i++) {
		if (cavan_http_client_writeable(sender->clients + i)) {
			return i;
		}
	}

	for (i = 0; i < index; i++) {
		if (cavan_http_client_writeable(sender->clients + i)) {
			return i;
		}
	}

	return -1;
}

#if CONFIG_CAVAN_C99
static int cavan_http_sender_find_errdesc(cavan_string_t *body, char *buff, int size)
{
	const char *errdesc, *end;
	int length;

	errdesc = strstr(body->text, "\"errdesc\":");
	if (errdesc == NULL) {
		return -ENOENT;
	}

	errdesc += 10;

	if (*errdesc != '"') {
		return -EINVAL;
	}

	errdesc++;

	end = strchr(errdesc, '"');
	if (end == NULL) {
		return -EINVAL;
	}

	length = end - errdesc;
	if (length >= size) {
		length = size - 1;
	}

	memcpy(buff, errdesc, length);
	buff[length] = 0;

	return length;
}

static bool cavan_http_sender_is_completed(cavan_string_t *body, int count)
{
	int i;
	int length;
	char errdesc[1024];
	const char *texts[] = {
		"\u4e0a\u9650", // 上限
		"\u7528\u5b8c", // 用完
		"\u592a\u5feb", // 太快
		"\u9891\u7e41", // 频繁
		"\u9886\u53d6\u8fc7", // 领取过
	};

	length = cavan_http_sender_find_errdesc(body, errdesc, sizeof(errdesc));
	if (length < 0) {
		return false;
	}

	for (i = 0; i < NELEM(texts); i++) {
		if (strstr(errdesc, texts[i])) {
			println("Found: %s", texts[i]);
			return true;
		}
	}

	if (strstr(errdesc, "\u5b8c")) { // 完
		return (count > HTTP_SENDER_SEND_MIN);
	}

	return false;
}
#endif

static void *cavan_http_sender_receive_thread(void *data)
{
	struct cavan_http_client *client = data;
	struct cavan_http_sender *sender = client->sender;
	struct cavan_http_packet *rsp;
	struct cavan_fifo fifo;
	cavan_string_t *body;
	int ret;

	ret = cavan_fifo_init(&fifo, 4096, &client->client);
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

	body = &rsp->body;

	cavan_http_sender_lock(sender);

	while (client->running) {
		int packet;
		int count;

		cavan_http_sender_unlock(sender);
		ret = cavan_http_packet_read(rsp, &fifo);
		cavan_http_sender_lock(sender);

		if (ret < 0) {
			pr_red_info("cavan_http_packet_read");
			break;
		}

		while ((packet = cavan_http_client_dequeue(client)) < 0 && client->running) {
			cavan_http_sender_unlock(sender);
			pr_red_info("wait packet enqueue");
			msleep(100);
			cavan_http_sender_lock(sender);
		}

		count = ++sender->counts[packet];

		if (sender->verbose) {
			println("received: packet[%d] = %d", count, packet);
			cavan_stdout_write_line(body->text, body->length);
		}

#if CONFIG_CAVAN_C99
		if (cavan_http_sender_is_completed(body, count)) {
			sender->status[packet] = false;
		}
#endif
	}

	cavan_http_sender_unlock(sender);

	cavan_http_packet_free(rsp);
out_fifo_deinit:
	cavan_fifo_deinit(&fifo);
out_pthread_cond_signal:
	cavan_http_sender_lock(sender);
	client->running = false;
	sender->conn_count--;
	cavan_http_sender_post(&client->cond_exit);
	cavan_http_sender_post(&sender->cond_write);
	cavan_http_sender_unlock(sender);
	return NULL;
}

static int cavan_http_sender_get_packet(struct cavan_http_sender *sender, int index, int count)
{
	int i;

	for (i = index; i < count; i++) {
		if (sender->status[i]) {
			return i;
		}
	}

	for (i = 0; i < index; i++) {
		if (sender->status[i]) {
			return i;
		}
	}

	return -ENOENT;
}

static int cavan_http_sender_main_loop(struct cavan_http_sender *sender, struct cavan_http_packet *packets[], int count)
{
	u64 overtime = sender->time + sender->repeat * 1000;
	bool pending = true;
	int i;

	for (i = 0; i < count; i++) {
		sender->status[i] = true;
		sender->counts[i] = 0;
	}

	while (1) {
		struct timespec ts;
		struct tm tm;
		u64 mseconds;

		clock_gettime_real(&ts);
		localtime_r(&ts.tv_sec, &tm);

		println("time  = %02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);

		mseconds = cavan_timespec_mseconds(&ts);
		if (mseconds + HTTP_SENDER_AHEAD < sender->time) {
			char buff[1024];
			int delay;

			delay = sender->time - mseconds;
			time2text_msec(delay, buff, sizeof(buff));
			println("delay = %s", buff);
			msleep(1000 - (mseconds % 1000));
		} else {
			break;
		}
	}

	while (pending && clock_gettime_real_ms() < overtime) {
		int packet_index;
		int conn_index;
		int conn_count;
		int index;
		int ret;

		while (1) {
			conn_count = cavan_http_sender_open(sender, count);
			if (conn_count > 0) {
				break;
			}
		}

		sender->conn_count = conn_count;

		println("open %d connects successfull", conn_count);

		while (1) {
			u64 time = clock_gettime_real_ms();

			if (time < sender->time) {
				int delay = sender->time - time;

				if (delay > 1000) {
					println("delay = %d", delay);
					msleep(1000);
				} else {
					msleep(delay);
					break;
				}
			} else {
				break;
			}
		}

		conn_index = 0;
		packet_index = 0;

		cavan_http_sender_lock(sender);

		while (sender->conn_count > 0 && clock_gettime_real_ms() < overtime) {
			struct cavan_http_client *client;
			cavan_string_t *header;

			while ((index = cavan_http_sender_get_client(sender, conn_index, conn_count)) < 0) {
				if (sender->conn_count <= 0) {
					goto out_cavan_http_sender_close;
				}

				if (sender->verbose) {
					println("wait for write");
				}

				cavan_http_sender_wait(sender, &sender->cond_write);
			}

			packet_index = cavan_http_sender_get_packet(sender, packet_index, count);
			if (packet_index < 0) {
				pending = false;
				break;
			}

			client = sender->clients + index;
			conn_index = (index + 1) % conn_count;
			header = &packets[packet_index]->header;
			cavan_http_client_enqueue(client, packet_index);

			if (sender->verbose) {
				println("send: conn = %d, packet = %d", index, packet_index);
			}

			cavan_http_sender_unlock(sender);
			ret = network_client_send(&client->client, header->text, header->length);
			cavan_http_sender_lock(sender);

			if (ret < 0) {
				pr_red_info("cavan_http_sender_send_packet");
				cavan_http_client_close(client);
				continue;
			}

			packet_index = (packet_index + 1) % count;
		}

out_cavan_http_sender_close:
		cavan_http_sender_close(sender, conn_count);
		cavan_http_sender_unlock(sender);
	}

	return 0;
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
	println("-R, -r, --repeat\trepeat seconds");
	println("--daemon\t\t%s", cavan_help_message_daemon);
}

int main(int argc, char *argv[])
{
	int i;
	int c;
	int ret;
	int count = 0;
	int delay = -200;
	int option_index;
	char *test = NULL;
	struct cavan_http_packet *packets[HTTP_SENDER_PACKAGES];
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
			.name = "repeat",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_REPEAT,
		}, {
			.name = "daemon",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_DAEMON,
		}, {
			0, 0, 0, 0
		},
	};

	cavan_http_sender_init(&sender);

	sender.repeat = HTTP_SENDER_REPEAT;
	sender.time = ((clock_gettime_real_ms() + 3600000 - 1) / 3600000) * 3600000;

	while ((c = getopt_long(argc, argv, "vVhHd:D:tr:R:TnN", long_option, &option_index)) != EOF) {
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

		case 'r':
		case 'R':
		case CAVAN_COMMAND_OPTION_REPEAT:
			sender.repeat = text2value_unsigned(optarg, NULL, 10);
			break;

		case CAVAN_COMMAND_OPTION_DAEMON:
			sender.daemon = true;
			break;

		default:
			cavan_http_sender_show_usage(argv[0]);
			return -EINVAL;
		}
	}

	println("delay = %d", delay);
	println("repeat = %d", sender.repeat);

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

		cavan_http_sender_url_init(&sender, test);

		ret = network_client_open(&client, &sender.url, 0);
		if (ret < 0) {
			pr_red_info("network_client_open2");
			return ret;
		}

		while (1) {
			cavan_string_t *header = &req.header;
			char date[32];
			u64 time;

			time = clock_gettime_mono_ms();

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

			println("delay = %d", (int) (clock_gettime_mono_ms() - time));

			if (cavan_http_packet_get_header(&rsp, HTTP_HEADER_DATE, date, sizeof(date)) > 0) {
				char buff[1024];

				cavan_http_time_tostring3(buff, sizeof(buff));

				println("remote = %s", date);
				println("local  = %s", buff);
			}

			msleep(100);
		}

		return 0;
	} else if (count > 0) {
		char host0[128];

		if (cavan_http_packet_get_header(packets[0], HTTP_HEADER_HOST, host0, sizeof(host0)) < 0) {
			pr_red_info("host not found");
			return -EINVAL;
		}

		println("host = %s", host0);

		for (i = 1; i < count; i++) {
			char host1[128];

			if (cavan_http_packet_get_header(packets[i], HTTP_HEADER_HOST, host1, sizeof(host1)) < 0) {
				pr_red_info("host[%d] not found", i);
				return -EINVAL;
			}

			if (strcmp(host0, host1) != 0) {
				pr_red_info("host[%d] not mach: %s", i, host1);
				return -EINVAL;
			}
		}

		if (!cavan_http_sender_url_init(&sender, host0)) {
			pr_red_info("cavan_http_sender_url_init");
			return -EFAULT;
		}

		if (sender.daemon) {
			pr_bold_info("run as daemon now");

			ret = daemon(1, 0);
			if (ret < 0) {
				pr_err_info("daemon: %d", ret);
			}
		}

		return cavan_http_sender_main_loop(&sender, packets, count);
	} else {
		return -EINVAL;
	}
}
