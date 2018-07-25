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
#include <cavan/json.h>
#include <cavan/timer.h>
#include <cavan/thread.h>
#include <cavan/network.h>

#define HTTP_SENDER_AHEAD			10000
#define HTTP_SENDER_DELAY			(-100)
#define HTTP_SENDER_HOST			"game.weixin.qq.com"
#define HTTP_SENDER_SEND_COUNT		10
#define HTTP_SENDER_CONN_COUNT		50
#define HTTP_SENDER_GROUP_COUNT		20
#define HTTP_SENDER_SUCCESS_MAX		2
#define HTTP_SENDER_PACKAGES		200

struct cavan_http_sender;

struct cavan_http_client {
	struct network_client client;
	struct cavan_http_packet *packet;
	struct cavan_fifo fifo;
	int index;
	int send_times;
	int recv_times;
	bool opened;
	struct timespec time;
	struct cavan_http_client *prev;
	struct cavan_http_client *next;
};

struct cavan_http_group {
	struct cavan_http_sender *sender;
	struct cavan_http_client *head;
	struct cavan_http_group *next;
	pthread_cond_t cond_exit;
	bool completed;
	bool running;
	int success;
	int count;
};

struct cavan_http_sender {
	struct cavan_http_client clients[HTTP_SENDER_CONN_COUNT];
	struct cavan_http_group groups[HTTP_SENDER_GROUP_COUNT];
	struct cavan_http_group *head;
	struct cavan_http_group *tail;
	pthread_cond_t cond_write;
	struct network_url url;
	pthread_mutex_t lock;
	int group_count;
	int conn_count;
	int send_max;
	bool running;
	bool verbose;
	bool daemon;
	bool force;
	bool http;
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

static inline void cavan_http_sender_timedwait(struct cavan_http_sender *sender, pthread_cond_t *cond, const struct timespec *time)
{
	pthread_cond_timedwait(cond, &sender->lock, time);
}

static inline void cavan_http_sender_post(pthread_cond_t *cond)
{
	pthread_cond_signal(cond);
}

static void cavan_http_sender_enqueue(struct cavan_http_group *group)
{
	if (group->next == group) {
		struct cavan_http_sender *sender = group->sender;

		if (sender->head == NULL) {
			sender->head = group;
			cavan_http_sender_post(&sender->cond_write);
		} else {
			sender->tail->next = group;
		}

		sender->tail = group;
		group->next = NULL;
	}
}

static struct cavan_http_group *cavan_http_sender_dequeue(struct cavan_http_sender *sender)
{
	struct cavan_http_group *group = sender->head;

	if (group != NULL) {
		sender->head = group->next;
		group->next = group;
	}

	return group;
}

static void cavan_http_group_add(struct cavan_http_group *group, struct cavan_http_client *client)
{
	if (group->head == NULL) {
		client->next = client->prev = client;
		group->head = client;
	} else {
		struct cavan_http_client *head = group->head;
		struct cavan_http_client *tail = head->prev;

		tail->next = head->prev = client;
		client->prev = tail;
		client->next = head;
	}

	group->count++;
}

static void cavan_http_group_remove(struct cavan_http_group *group, struct cavan_http_client *client)
{
	struct cavan_http_client *next = client->next;
	struct cavan_http_client *prev = client->prev;

	prev->next = next;
	next->prev = prev;

	if (group->head == client) {
		if (next == client) {
			group->head = NULL;
		} else {
			group->head = next;
		}
	}

	group->count--;
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

static int cavan_http_group_open(struct cavan_http_group *group)
{
	return cavan_pthread_run(cavan_http_sender_receive_thread, group);
}

static void cavan_http_group_close(struct cavan_http_group *group)
{
	if (group->running) {
		struct cavan_http_client *client = group->head;

		group->running = false;

		if (client != NULL) {
			network_client_close(&client->client);
		}

		cavan_http_sender_wait(group->sender, &group->cond_exit);
	}

	group->head = NULL;
}

static void cavan_http_group_init(struct cavan_http_group *group)
{
	pthread_cond_init(&group->cond_exit, NULL);
	group->running = false;
	group->head = NULL;
	group->count = 0;
}

static void cavan_http_sender_init(struct cavan_http_sender *sender)
{
	memset(sender, 0x00, sizeof(*sender));
	pthread_mutex_init(&sender->lock, NULL);
	pthread_cond_init(&sender->cond_write, NULL);
}

static void cavan_http_sender_open(struct cavan_http_sender *sender, struct cavan_http_packet *packets[], int count)
{
	struct cavan_http_group *group = sender->groups;
	int index = 0;
	int i;

	sender->head = sender->tail = NULL;

	for (i = 0; i < HTTP_SENDER_GROUP_COUNT; i++) {
		cavan_http_group_init(sender->groups + i);
	}

	for (i = 0; i < count; i++) {
		struct cavan_http_client *client = sender->clients + i;
		struct cavan_http_packet *packet = packets[i];
		int ret;

		if (packet->group > index) {
			index = packet->group;

			if (group->count > 0) {
				group++;
			}
		}

		client->packet = packet;
		client->index = i;
		client->send_times = 0;
		client->recv_times = 0;

		ret = network_client_open(&client->client, &sender->url, CAVAN_NET_FLAG_NODELAY);
		if (ret < 0) {
			pr_red_info("network_client_open");
			client->opened = false;
		} else {
			client->opened = true;
		}

		cavan_fifo_init(&client->fifo, 4096, client);
		client->fifo.read = network_client_fifo_read;
		cavan_http_group_add(group, client);
	}

	sender->group_count = group - sender->groups;
	if (group->count > 0) {
		sender->group_count++;
	} else {
		group--;
	}

	while (group >= sender->groups) {
		group->sender = sender;
		group->next = group;

		if (cavan_http_group_open(group) < 0) {
			pr_red_info("cavan_http_group_open");
			break;
		}

		group--;
	}
}

static void cavan_http_sender_close(struct cavan_http_sender *sender, int count)
{
	int i;

	for (i = 0; i < HTTP_SENDER_GROUP_COUNT; i++) {
		cavan_http_group_close(sender->groups + i);
	}

	for (i = 0; i < sender->conn_count; i++) {
		network_client_close(&sender->clients[i].client);
	}
}

#if CONFIG_CAVAN_C99
static int cavan_http_sender_find_errdesc(cavan_string_t *body, char *buff, int size)
{
	const struct cavan_json_node *node;
	struct cavan_json_document *doc;
	int length;

	doc = cavan_json_document_parse(body->text, body->length);
	if (doc == NULL) {
		pr_err_info("cavan_json_document_parse");
		return -EINVAL;
	}

	node = cavan_json_document_find(doc, "ret", NULL);
	if (node == NULL || node->value == NULL) {
		length = -EINVAL;
		goto out_cavan_json_document_free;
	}

	if (strcmp(node->value, "0") == 0) {
		length = 0;
		goto out_cavan_json_document_free;
	}

	node = cavan_json_document_find(doc, "data", "errdesc", NULL);
	if (node == NULL || node->value == NULL) {
		length = -EINVAL;
		goto out_cavan_json_document_free;
	}

	length = text_ncopy(buff, node->value, size) - buff;

out_cavan_json_document_free:
	cavan_json_document_free(doc);
	return length;
}

static bool cavan_http_sender_is_completed(struct cavan_http_group *group, struct cavan_http_client *client, cavan_string_t *body)
{
	int i;
	int length;
	char errdesc[1024];
	const char *texts0[] = {
		"\u4e0a\u9650", // 上限
		"\u592a\u5feb", // 太快
		"\u9891\u7e41", // 频繁
	};
	const char *texts1[] = {
		"\u5143", // 元
		"\u7528\u5b8c", // 用完
		"\u73b0\u91d1", // 现金
		"\u53d1\u653e", // 发放
		"\u9519\u8bef", // 错误
		"\u9886\u53d6\u8fc7", // 领取过
	};
	const char *texts2[] = {
		"\u9886\u5b8c", // 领完
		"\u7b7e\u5230\u8fc7", // 签到过
	};
	const char *texts3[] = {
		"\u4e0d\u8db3", // 不足
		"\u4e0d\u6ee1\u8db3", // 不满足
		"\u672a\u767b\u5f55", // 未登录
	};

	if (group->sender->force) {
		return false;
	}

	length = cavan_http_sender_find_errdesc(body, errdesc, sizeof(errdesc));
	if (length < 0) {
		return false;
	}

	if (length == 0) {
		u32 delay;

		group->success++;
		println("Successfull: %d", group->success);

		cavan_http_sender_unlock(group->sender);

		if (group->success < HTTP_SENDER_SUCCESS_MAX) {
			delay = 200;
		} else {
			delay = 500;
		}

		println("Delay: %d", delay);
		msleep(delay);

		cavan_http_sender_lock(group->sender);

		return true;
	}

	if (group->sender->verbose) {
		println("errdesc = %s", errdesc);
	}

	for (i = 0; i < NELEM(texts0); i++) {
		if (strstr(errdesc, texts0[i])) {
			println("Found: %s", texts0[i]);
			group->completed = true;
			return true;
		}
	}

	for (i = 0; i < NELEM(texts1); i++) {
		if (strstr(errdesc, texts1[i])) {
			println("Found: %s", texts1[i]);
			return true;
		}
	}

	for (i = 0; i < NELEM(texts2); i++) {
		if (strstr(errdesc, texts2[i])) {
			struct tm *time = localtime(&client->time.tv_sec);

			if (time->tm_min < 59 && time->tm_sec > 0) {
				println("Found: %s", texts2[i]);
				return true;
			}

			return false;
		}
	}

	for (i = 0; i < NELEM(texts3); i++) {
		if (strstr(errdesc, texts3[i])) {
			cavan_http_sender_unlock(group->sender);
			println("Wait: %s", texts3[i]);
			msleep(500);
			cavan_http_sender_lock(group->sender);
			return false;
		}
	}

	return false;
}
#endif

static void *cavan_http_sender_receive_thread(void *data)
{
	struct cavan_http_group *group = data;
	struct cavan_http_sender *sender = group->sender;
	struct cavan_http_packet *rsp;
	cavan_string_t *body;
	int ret;

	group->completed = false;
	group->running = true;
	group->success = 0;

	cavan_http_sender_lock(sender);

	rsp = cavan_http_packet_alloc();
	if (rsp == NULL) {
		pr_red_info("cavan_http_packet_alloc");
		goto out_exit;
	}

	body = &rsp->body;

	while (1) {
		struct cavan_http_client *client;
		int count;

		while (1) {
			client = group->head;
			if (client == NULL) {
				goto out_unlock;
			}

			if (client->send_times < sender->send_max) {
				if (client->opened) {
					break;
				}

				int ret = network_client_open(&client->client, &sender->url, CAVAN_NET_FLAG_NODELAY);
				if (ret < 0) {
					pr_red_info("network_client_open");
					msleep(200);
				} else {
					client->opened = true;
				}
			} else {
				cavan_http_group_remove(group, client);
			}
		}

		cavan_fifo_reset(&client->fifo);
		cavan_http_sender_enqueue(group);

		cavan_http_sender_unlock(sender);
		ret = cavan_http_packet_read(rsp, &client->fifo);
		cavan_http_sender_lock(sender);

		if (ret < 0) {
			pr_red_info("cavan_http_packet_read");
			network_client_close(&client->client);
			client->opened = false;
		}

		if (group->head != NULL) {
			group->head = client->next;
		}

		count = ++client->recv_times;

		if (sender->verbose) {
			println("received[%d]: %d/%d", client->index, count, client->send_times);
			cavan_stdout_write_line(body->text, body->length);
		}

#if CONFIG_CAVAN_C99
		if (cavan_http_sender_is_completed(group, client, body)) {
			cavan_http_group_remove(group, client);
			network_client_close(&client->client);
		}
#endif

		if (group->completed) {
			println("completed");
			break;
		}
	}

out_unlock:
	cavan_http_packet_free(rsp);
out_exit:
	sender->group_count--;
	group->running = false;
	cavan_http_sender_post(&group->cond_exit);
	cavan_http_sender_post(&sender->cond_write);
	cavan_http_sender_unlock(sender);
	return NULL;
}

static int cavan_http_sender_main_loop(struct cavan_http_sender *sender, struct cavan_http_packet *packets[], int count)
{
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

	cavan_http_sender_open(sender, packets, count);
	sender->conn_count = count;

	println("conn_count = %d", sender->conn_count);
	println("group_count = %d", sender->group_count);

	while (sender->group_count > 0) {
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

	cavan_http_sender_lock(sender);

	while (sender->group_count > 0) {
		struct cavan_http_client *client;
		struct cavan_http_group *group;
		cavan_string_t *header;

		group = cavan_http_sender_dequeue(sender);
		if (group == NULL) {
			cavan_http_sender_wait(sender, &sender->cond_write);
			continue;
		}

		client = group->head;
		if (client == NULL) {
			continue;
		}

		header = &client->packet->header;
		client->send_times++;

		if (sender->verbose) {
			println("send[%d]: %d", client->index, client->send_times);
		}

		cavan_http_sender_unlock(sender);
		clock_gettime_real(&client->time);
		network_client_send(&client->client, header->text, header->length);
		cavan_http_sender_lock(sender);
	}

	cavan_http_sender_close(sender, count);
	cavan_http_sender_unlock(sender);
	return 0;
}

static int cavan_http_sender_check_file_time(const char *pathname)
{
	char buff[1024];
	struct stat sb;
	int overtime;
	int ret;

	ret = stat(pathname, &sb);
	if (ret < 0) {
		pr_err_info("stat: %s", pathname);
		return ret;
	}

	overtime = time(NULL) - sb.st_atime;
	time2text_sec(overtime, buff, sizeof(buff));

	println("overtime = %s", buff);

	if (overtime > 36000) {
		return -EINVAL;
	}

	return 0;
}

static void cavan_http_sender_show_usage(const char *command)
{
	println("Usage:");
	println("%s [option] FILES", command);
	println("-H, -h, --help\t\t%s", cavan_help_message_help);
	println("-V, -v, --verbose\t%s", cavan_help_message_verbose);
	println("-T, -t, --test\t\ttest only");
	println("-D, -d, --delay\t\t%s", cavan_help_message_delay_time);
	println("-N, -n, --now\t\t%s", cavan_help_message_current_time);
	println("-C, -c, --count\t\tsend count");
	println("--daemon\t\t%s", cavan_help_message_daemon);
	println("--force\t\tforce send");
}

int main(int argc, char *argv[])
{
	int i;
	int c;
	int ret;
	int count = 0;
	int delay = HTTP_SENDER_DELAY;
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
			.name = "count",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_COUNT,
		}, {
			.name = "daemon",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_DAEMON,
		}, {
			.name = "force",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_FORCE,
		}, {
			0, 0, 0, 0
		},
	};

	cavan_http_sender_init(&sender);

	sender.send_max = HTTP_SENDER_SEND_COUNT;
	sender.time = ((clock_gettime_real_ms() + 3600000 - 1) / 3600000) * 3600000;

	while ((c = getopt_long(argc, argv, "vVhHd:D:tc:C:TnN", long_option, &option_index)) != EOF) {
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

		case 'c':
		case 'C':
		case CAVAN_COMMAND_OPTION_COUNT:
			sender.send_max = text2value_unsigned(optarg, NULL, 10);
			break;

		case CAVAN_COMMAND_OPTION_DAEMON:
			sender.daemon = true;
			break;

		case CAVAN_COMMAND_OPTION_FORCE:
			sender.force = true;
			break;

		default:
			cavan_http_sender_show_usage(argv[0]);
			return -EINVAL;
		}
	}

#if CONFIG_CAVAN_SSL == 0
	if (!sender.http) {
		pr_warn_info("ssl not enabled");
		sender.http = true;
	}
#endif

	println("delay = %d", delay);
	println("send_max = %d", sender.send_max);

	sender.time += delay;

	for (i = optind; i < argc; i++) {
		const char *pathname = argv[i];

		ret = cavan_http_sender_check_file_time(pathname);
		if (ret < 0) {
			pr_warn_info("cavan_http_sender_check_file_time");
		}

		ret = cavan_http_packet_parse_file(pathname, packets + count, NELEM(packets) - count);
		if (ret < 0) {
			pr_red_info("cavan_http_sender_load_file");
		} else if (ret > 0) {
			count += ret;
		} else {
			pr_warn_info("No packet found!");
		}
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

		while (1) {
			cavan_http_sender_main_loop(&sender, packets, count);
			println("Press Enter to continue");
			while (getchar() != '\n');
		}
	} else {
		return -EINVAL;
	}

	return 0;
}
