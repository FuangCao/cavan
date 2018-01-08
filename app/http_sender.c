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
#include <cavan/timer.h>
#include <cavan/thread.h>
#include <cavan/network.h>

#define HTTP_SENDER_AHEAD	15000

struct cavan_http_packet {
	char *header;
	char *body;
	size_t size;
	size_t used;
	size_t length;
	int lines;
	char *host;
	char *content_length;
};

struct cavan_http_sender {
	u64 time;
	bool http;
	bool verbose;
};

static struct cavan_http_sender g_http_sender;

static struct cavan_http_packet *cavan_http_packet_alloc(void)
{
	struct cavan_http_packet *packet = malloc(sizeof(struct cavan_http_packet));
	if (packet != NULL) {
		memset(packet, 0x00, sizeof(struct cavan_http_packet));
	}

	return packet;
}

static void cavan_http_packet_clear(struct cavan_http_packet *packet)
{
	if (packet->host) {
		free(packet->host);
		packet->host = NULL;
	}

	if (packet->content_length) {
		free(packet->content_length);
		packet->content_length = NULL;
	}

	if (packet->body) {
		free(packet->body);
		packet->body = NULL;
	}

	if (packet->header) {
		free(packet->header);
		packet->header = NULL;
	}

	packet->used = 0;
	packet->size = 0;
	packet->lines = 0;
	packet->length = 0;
}

static void cavan_http_packet_free(struct cavan_http_packet *packet)
{
	cavan_http_packet_clear(packet);
	free(packet);
}

static int cavan_http_packet_add_text(struct cavan_http_packet *packet, const char *text, size_t size)
{
	size_t total = packet->used + size;

	if (total >= packet->size) {
		size_t new_size = total << 1;
		char *header = malloc(new_size);
		if (header == NULL) {
			return -ENOMEM;
		}

		if (packet->size && packet->header) {
			memcpy(header, packet->header, packet->used);
			free(packet->header);
		}

		packet->header = header;
		packet->size = new_size;
	}

	memcpy(packet->header + packet->used, text, size);
	packet->used += size;

	return size;
}

static int cavan_http_packet_add_line_end(struct cavan_http_packet *packet)
{
	return cavan_http_packet_add_text(packet, "\r\n", 2);
}

static int cavan_http_packet_add_line(struct cavan_http_packet *packet, const char *line, size_t size)
{
	const char *line_end;
	const char *name, *value;
	int namelen;
	int ret;

	ret = cavan_http_packet_add_text(packet, line, size);
	if (ret < 0) {
		pr_red_info("cavan_http_packet_add_text");
		return ret;
	}

	ret = cavan_http_packet_add_line_end(packet);
	if (ret < 0) {
		pr_red_info("cavan_http_packet_add_line_end");
		return ret;
	}

	packet->lines++;

	if (packet->lines == 1) {
		return 0;
	}

	line_end = line + size;
	name = line;
	namelen = 0;
	value = NULL;

	while (line < line_end) {
		switch (*line) {
		case ':':
			namelen = line - name;
			value = line + 1;
			break;

		case ' ':
		case '\t':
			if (name == line) {
				name++;
			} else if (value == line) {
				value++;
			}
			break;

		default:
			if (value != NULL) {
				goto out_parse_complete;
			}
		}

		line++;
	}

out_parse_complete:
	if (value != NULL) {
		if (namelen == 4 && strncasecmp(name, "host", 4) == 0) {
			packet->host = strndup(value, line_end - value);
		} else if (namelen == 14 && strncasecmp(name, "content-length", 14) == 0) {
			packet->content_length = strndup(value, line_end - value);
		}
	}

	return 0;
}

static int cavan_http_packet_read_body(struct cavan_http_packet *packet, struct cavan_fifo *fifo)
{
	char *content_length;
	int length;

	content_length = packet->content_length;
	if (content_length == NULL) {
		return 0;
	}

	length = text2value_unsigned(packet->content_length, NULL, 10);
	if (length > 0) {
		packet->body = malloc(length);
		if (packet->body == NULL) {
			pr_err_info("malloc");
			return -ENOMEM;
		}

		if (cavan_fifo_fill(fifo, packet->body, length) < length) {
			pr_red_info("cavan_fifo_fill");
			return -EFAULT;
		}

		packet->length = length;

		if (g_http_sender.verbose) {
			cavan_stdout_write_line(packet->body, length);
		}
	}

	return 0;
}

static int cavan_http_packet_read_response(struct cavan_http_packet *packet, struct cavan_fifo *fifo)
{
	cavan_http_packet_clear(packet);

	while (1) {
		char line[4096];
		char *p;
		int ret;

		p = cavan_fifo_read_line_strip(fifo, line, sizeof(line));
		if (p > line) {
			int ret = cavan_http_packet_add_line(packet, line, p - line);
			if (ret < 0) {
				pr_red_info("cavan_http_packet_add_line");
				return ret;
			}
		} else {
			ret = cavan_http_packet_read_body(packet, fifo);
			if (ret < 0) {
				pr_red_info("cavan_http_packet_read_body");
				return ret;
			}

			break;
		}
	}

	return 0;
}

static int cavan_http_sender_load_file(const char *pathname, struct cavan_http_packet *packets[])
{
	int ret;
	int count;
	char *text;
	size_t size;
	char *p, *p_end;
	char *line, *line_end;
	struct cavan_http_packet *packet;

	text = file_read_all(pathname, 0, &size);
	if (text == NULL) {
		pr_red_info("file_read_all");
		return -EFAULT;
	}

	packet = cavan_http_packet_alloc();
	if (packet == NULL) {
		pr_red_info("cavan_http_packet_alloc");
		ret = -ENOMEM;
		goto out_free_text;
	}

	count = 0;
	line = text;
	line_end = NULL;

	for (p = text, p_end = p + size; p < p_end; p++) {
		switch (*p) {
		case '\r':
			if (line_end == NULL) {
				line_end = p;
			}
			break;

		case '\n':
			if (line_end == NULL) {
				line_end = p;
			}

			if (line_end > line) {
				ret = cavan_http_packet_add_line(packet, line, line_end - line);
				if (ret < 0) {
					pr_red_info("cavan_http_packet_add_line");
					goto out_free_packets;
				}
			} else if (packet->lines > 0) {
				ret = cavan_http_packet_add_line_end(packet);
				if (ret < 0) {
					pr_red_info("cavan_http_packet_add_line_end");
					goto out_free_packets;
				}

				packets[count++] = packet;
				packet = cavan_http_packet_alloc();
				if (packet == NULL) {
					pr_red_info("cavan_http_packet_alloc");
					ret = -ENOMEM;
					goto out_free_packets;
				}
			}

			line = p + 1;
			line_end = NULL;
			break;
		}
	}

	if (packet->lines > 0) {
		ret = cavan_http_packet_add_line_end(packet);
		if (ret < 0) {
			pr_red_info("cavan_http_packet_add_line_end");
			goto out_free_packets;
		}

		packets[count++] = packet;
	} else {
		cavan_http_packet_free(packet);
	}

	free(text);
	return count;

out_free_packets:
	if (packet != NULL) {
		cavan_http_packet_free(packet);
	}

	while (count > 0) {
		cavan_http_packet_free(packets[--count]);
	}
out_free_text:
	free(text);
	return ret;
}

static void *cavan_http_sender_thread_handler(void *data)
{
	struct cavan_http_packet *req = data;
	struct cavan_http_packet *rsp;
	struct network_client client;
	struct network_url url;
	struct cavan_fifo fifo;
	int count = 0;
	u16 port;
	char *p;
	int ret;

	ret = cavan_fifo_init(&fifo, 4096, &client);
	if (ret < 0) {
		pr_red_info("cavan_fifo_init");
		return NULL;
	}

	fifo.read = network_client_fifo_read;

	rsp = cavan_http_packet_alloc();
	if (rsp == NULL) {
		pr_red_info("cavan_http_packet_alloc");
		return NULL;
	}

	p = strchr(req->host, ':');
	if (p != NULL) {
		port = text2value_unsigned(p + 1, NULL, 10);
		*p = 0;
	} else if (g_http_sender.http) {
		port = 80;
	} else {
		port = 443;
	}

	network_url_init(&url, g_http_sender.http ? "tcp" : "ssl", req->host, port, NULL);
	println("url = %s", network_url_tostring(&url, NULL, 0, NULL));

	while (1) {
		u64 time;
		int ret;

		while (1) {
			ret = network_client_open(&client, &url, 0);
			if (ret < 0) {
				pr_red_info("network_client_open");
				msleep(200);
			} else {
				break;
			}
		}

		println("connect successfull");

		while (1) {
			time = clock_gettime_real_ms();
			if (time < g_http_sender.time) {
				int delay = g_http_sender.time - time;
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

		while (1) {
			cavan_fifo_reset(&fifo);

			ret = network_client_send(&client, req->header, req->used);
			if (ret < 0) {
				pr_red_info("network_client_send");
				break;
			}

			ret = cavan_http_packet_read_response(rsp, &fifo);
			if (ret < 0) {
				pr_red_info("network_client_send");
				break;
			}

			count++;
			msleep(count * 50);
		}

		network_client_close(&client);
	}

	return NULL;
}

static void cavan_http_sender_show_usage(const char *command)
{
	println("Usage:");
	println("%s [option] URL", command);
	println("-H, -h, --help\t\t%s", cavan_help_message_help);
	println("-V, -v, --verbose\t%s", cavan_help_message_verbose);
	println("-T, -t, --time\t\t%s", cavan_help_message_start_time);
	println("-D, -d, --delay\t\t%s", cavan_help_message_delay_time);
	println("-N, -n, --now\t\t%s", cavan_help_message_current_time);
}

int main(int argc, char *argv[])
{
	int i;
	int c;
	int ret;
	int count = 0;
	int delay = 0;
	int option_index;
	struct cavan_http_packet *packets[100];
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
			0, 0, 0, 0
		},
	};

	g_http_sender.time = ((clock_gettime_real_ms() + 3600000 - 1) / 3600000) * 3600000;

	while ((c = getopt_long(argc, argv, "vVhHd:D:t:T:nN", long_option, &option_index)) != EOF) {
		switch (c) {
		case 'v':
		case 'V':
		case CAVAN_COMMAND_OPTION_VERSION:
			g_http_sender.verbose = true;
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

		case 't':
		case 'T':
		case CAVAN_COMMAND_OPTION_TIME:
			break;

		case 'n':
		case 'N':
		case CAVAN_COMMAND_OPTION_NOW:
			g_http_sender.time = clock_gettime_real_ms();
			break;

		case CAVAN_COMMAND_OPTION_HTTP:
			g_http_sender.http = true;
			break;

		default:
			cavan_http_sender_show_usage(argv[0]);
			return -EINVAL;
		}
	}

	println("delay = %d", delay);

	g_http_sender.time += delay;

	for (i = optind; i < argc; i++) {
		ret = cavan_http_sender_load_file(argv[i], packets + count);
		if (ret < 0) {
			pr_red_info("cavan_http_sender_load_file");
			return ret;
		}

		count += ret;
	}

	println("count = %d", count);

	if (count > 0) {
		while (1) {
			u64 time = clock_gettime_real_ms();
			if (time + HTTP_SENDER_AHEAD < g_http_sender.time) {
				char buff[1024];

				delay = g_http_sender.time - time;
				time2text_msec(delay, buff, sizeof(buff));
				println("delay = %s", buff);
				msleep(1000);
			} else {
				break;
			}
		}

		for (i = 1; i < count; i++) {
			struct cavan_http_packet *packet = packets[i];
			if (packet->host) {
				cavan_pthread_run(cavan_http_sender_thread_handler, packet);
			}
		}

		struct cavan_http_packet *packet = packets[0];
		if (packet->host) {
			cavan_http_sender_thread_handler(packet);
		}

		while (1) {
			sleep(60);
		}
	}

	return 0;
}
