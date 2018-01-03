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
#include <cavan/thread.h>
#include <cavan/network.h>

static void *http_sender_thread_handler(void *pathname)
{
	char *text;
	size_t size;

	text = file_read_all_text(pathname, &size);
	if (text != NULL) {
		char *p, *p_end;
		char *name = text;
		char *value = NULL;
		int namelen = 0;

		name = text;
		value = NULL;

		for (p = text, p_end = p + size; p < p_end; p++) {
			switch (*p) {
			case ':':
				if (value == NULL) {
					namelen = p - name;
					value = p + 1;
				}
				break;

			case ' ':
			case '\t':
				if (name == p) {
					name++;
				} else if (value == p) {
					value++;
				}
				break;

			case '\r':
			case '\n':
				if (namelen == 4 && strncasecmp(name, "host", 4) == 0) {
					struct network_url url;
					char hostname[1024];
					u16 port;

					strncpy(hostname, value, p - value);
					p = strchr(hostname, ':');
					if (p != NULL) {
						port = text2value_unsigned(p + 1, NULL, 10);
						*p = 0;
					} else {
						port = 443;
					}

					network_url_init(&url, "ssl", hostname, port, NULL);

					while (1) {
						struct network_client client;
						char buff[4096];
						ssize_t rdlen;
						int ret;

						ret = network_client_open(&client, &url, 0);
						if (ret < 0) {
							pr_red_info("network_client_open");
							break;
						}

						network_client_send(&client, text, size);

						while (1) {
							rdlen = network_client_timed_recv(&client, buff, sizeof(buff), 2000);
							if (rdlen > 0) {
								write(1, buff, rdlen);
								fsync(1);
							} else {
								break;
							}
						}

						network_client_close(&client);

						msleep(1000);
					}

					return NULL;
				}

				name = p + 1;
				namelen = 0;
				value = NULL;
				break;
			}
		}
	}

	return NULL;
}

int main(int argc, char *argv[])
{
	int i;

	for (i = 1; i < argc; i++) {
		cavan_pthread_run(http_sender_thread_handler, argv[i]);
	}

	while (1) {
		sleep(60);
	}

	return 0;
}
