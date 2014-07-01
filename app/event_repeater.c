/*
 * File:		event_repeater.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2014-01-14 14:04:48
 *
 * Copyright (c) 2014 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/adb.h>
#include <cavan/input.h>
#include <cavan/network.h>
#include <cavan/command.h>

#define FILE_CREATE_DATE "2014-01-14 14:04:48"

struct event_repeater_data
{
	u16 port;
	const char *addr;
	int sockfd;
	int (*open_connect)(const char *ip, u16 port);
	void (*close_connect)(int sockfd);
};

static void show_usage(const char *command)
{
	println("Usage: %s [option]", command);
	println("--help, -h, -H\t\tshow this help");
	println("--version, -v, -V\tshow version");
	println("--ip, -i, -I\t\tserver ip address");
	println("--port, -p, -P\t\tserver port");
	println("--adb, -a, -A\t\tuse adb procotol instead of tcp");
}

static void event_repeater_input_handler(cavan_input_message_t *message, void *data)
{
	byte buff[1024], *p;
	struct event_repeater_data *repeater = data;

	p = mem_write8(buff, message->type);

	switch (message->type)
	{
	case CAVAN_INPUT_MESSAGE_KEY:
	case CAVAN_INPUT_MESSAGE_MOUSE_TOUCH:
		p = mem_write16(p, message->key.code);
		p = mem_write16(p, message->key.value);
		break;

	case CAVAN_INPUT_MESSAGE_MOUSE_MOVE:
		p = mem_write8(p, message->vector.x);
		p = mem_write8(p, message->vector.y);
		break;

	default:
		return;
	}

	ffile_write(repeater->sockfd, buff, p - buff);
}

static int event_repeater_run(struct event_repeater_data *data)
{
	int ret;
	int sockfd;
	struct cavan_input_service service;

	sockfd = data->open_connect(data->addr, data->port);
	if (sockfd < 0)
	{
		pr_red_info("open_connect");
		return sockfd;
	}

	cavan_input_service_init(&service, NULL);
	service.lcd_width = service.lcd_height = 100;
	service.handler = event_repeater_input_handler;
	data->sockfd = sockfd;
	ret = cavan_input_service_start(&service, data);
	if (ret < 0)
	{
		pr_red_info("cavan_input_service_start");
		goto out_close_connect;
	}

	cavan_input_service_join(&service);
	cavan_input_service_stop(&service);

	ret = 0;

out_close_connect:
	data->close_connect(sockfd);
	return ret;
}

int main(int argc, char *argv[])
{
	int c;
	int option_index;
	struct option long_option[] =
	{
		{
			.name = "help",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_HELP,
		},
		{
			.name = "version",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_VERSION,
		},
		{
			.name = "ip",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_IP,
		},
		{
			.name = "port",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_PORT,
		},
		{
			.name = "adb",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_ADB,
		},
		{
			0, 0, 0, 0
		},
	};
	struct event_repeater_data data =
	{
		.addr = "127.0.0.1",
		.port = 3333,
		.open_connect = inet_create_tcp_link2,
		.close_connect = inet_close_tcp_socket
	};

	while ((c = getopt_long(argc, argv, "vVhHIaA:i:I:p:P:", long_option, &option_index)) != EOF)
	{
		switch (c)
		{
		case 'v':
		case 'V':
		case CAVAN_COMMAND_OPTION_VERSION:
			show_author_info();
			println(FILE_CREATE_DATE);
			return 0;

		case 'h':
		case 'H':
		case CAVAN_COMMAND_OPTION_HELP:
			show_usage(argv[0]);
			return 0;

		case 'a':
		case 'A':
		case CAVAN_COMMAND_OPTION_ADB:
			data.open_connect = adb_create_tcp_link2;
			optarg = "127.0.0.1";
		case 'i':
		case 'I':
		case CAVAN_COMMAND_OPTION_IP:
			data.addr = optarg;
			break;

		case 'p':
		case 'P':
		case CAVAN_COMMAND_OPTION_PORT:
			data.port = text2value_unsigned(optarg, NULL, 10);
			break;

		default:
			show_usage(argv[0]);
			return -EINVAL;
		}
	}

	return event_repeater_run(&data);
}
