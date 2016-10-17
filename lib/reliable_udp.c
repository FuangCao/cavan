/*
 * File:		reliable_udp.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2016-10-17 11:14:59
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
#include <cavan/reliable_udp.h>

static struct reliable_udp_package *reliable_udp_package_alloc(size_t size, bool need_ack)
{
	struct reliable_udp_package *package = malloc(sizeof(struct reliable_udp_package) + size);

	if (package == NULL) {
		return NULL;
	}

	package->need_ack = need_ack;
	package->size = size;

	return package;
}

static void reliable_udp_add_package_locked(struct reliable_udp_client *client, struct reliable_udp_package *package, u64 time)
{
	struct reliable_udp_package **pp = &client->send_head;

	package->time = time;

	while (1) {
		struct reliable_udp_package *p = *pp;

		if (p == NULL || p->time > time) {
			break;
		}

		pp = &p->next;
	}

	package->next = *pp;
	*pp = package;

	if (package == client->send_head) {
		client->send_wakeup(client);
	}
}

static void reliable_udp_send_package(struct reliable_udp_client *client, struct reliable_udp_package *package)
{
	package->send_count = 0;

	client->send_lock(client);

	package->package->index = client->index_send;

	if (package->need_ack) {
		client->index_send++;
	}

	reliable_udp_add_package_locked(client, package, clock_gettime_ms_real());

	client->send_unlock(client);
}

static bool reliable_udp_send_empty_package(struct reliable_udp_client *client, u8 flags, bool need_ack)
{
	struct reliable_udp_header *header;
	struct reliable_udp_package *package = reliable_udp_package_alloc(sizeof(struct reliable_udp_header), need_ack);

	if (package == NULL) {
		return false;
	}

	header = package->package;
	header->index = client->index;
	header->index_ack = client->index_ack;
	header->flags = flags;

	reliable_udp_send_package(client, package);

	return true;
}

void reliable_udp_client_init(struct reliable_udp_client *client, void *data)
{
	client->client_data = data;
	client->index = client->index_send = clock_gettime_ms_mono();
}

void reliable_udp_send_main_loop(struct reliable_udp_client *client)
{
	client->send_lock(client);

	while (1) {
		struct reliable_udp_package *p = client->send_head;

		if (p == NULL) {
			client->send_unlock(client);
			client->send_wait(client, 0);
			client->send_lock(client);
		} else {
			u64 time = clock_gettime_ms_real();

			if (p->time < time) {
				client->send_unlock(client);
				client->send_wait(client, time - p->time);
				client->send_lock(client);
			} else {
				client->send_head = p->next;

				if (p->need_ack) {
					p->send_count++;
					reliable_udp_add_package_locked(client, p, time + p->send_count * 5000);
				}

				client->send_unlock(client);
				client->send(client, p->package, p->size);
				p->send_time = time;
				client->send_lock(client);

				if (!p->need_ack) {
					free(p);
				}
			}
		}
	}

	client->send_unlock(client);
}

void reliable_udp_recv_main_loop(struct reliable_udp_client *client)
{
	ssize_t rdlen;
	char buff[2048];
	struct reliable_udp_header *header = (struct reliable_udp_header *) buff;

	while (1) {
		rdlen = client->recv(client, buff, sizeof(buff));
		if (rdlen < 0) {
			return;
		}

		switch (client->state) {
		case RELIABLE_UDP_STATE_CLOSED:
			if (header->flags == RELIABLE_UDP_FLAG_SYN) {
				client->state = RELIABLE_UDP_STATE_SYN_RECV;
				client->index_ack = header->index + 1;
				reliable_udp_send_empty_package(client, RELIABLE_UDP_FLAG_SYN | RELIABLE_UDP_FLAG_ACK, true);
			} else if (header->flags & RELIABLE_UDP_FLAG_FIN) {
				client->index_ack = header->index + 1;
				reliable_udp_send_empty_package(client, RELIABLE_UDP_FLAG_ACK, false);
			}
			break;

		case RELIABLE_UDP_STATE_SYN_SEND:
			if (header->flags != (RELIABLE_UDP_FLAG_SYN | RELIABLE_UDP_FLAG_ACK)) {
				break;
			}

			if (client->index + 1 != header->index_ack) {
				break;
			}

			client->state = RELIABLE_UDP_STATE_CONNECTED;
			client->index = header->index_ack;
			client->index_ack = header->index + 1;
			reliable_udp_send_empty_package(client, RELIABLE_UDP_FLAG_ACK, false);
			break;

		case RELIABLE_UDP_STATE_SYN_RECV:
			if (header->flags != RELIABLE_UDP_FLAG_ACK) {
				break;
			}

			if (client->index + 1 != header->index_ack) {
				break;
			}

			client->state = RELIABLE_UDP_STATE_CONNECTED;
			client->index = header->index_ack;
			client->index_ack = header->index + 1;
			break;

		case RELIABLE_UDP_STATE_CONNECTED:
			if (header->flags & RELIABLE_UDP_FLAG_RST) {
				client->state = RELIABLE_UDP_STATE_CLOSED;
			} else if (header->flags & RELIABLE_UDP_FLAG_FIN) {
				client->state = RELIABLE_UDP_STATE_CLOSED;
				client->index_ack = header->index + 1;
				reliable_udp_send_empty_package(client, RELIABLE_UDP_FLAG_ACK, false);
			} else {
				if (header->flags & RELIABLE_UDP_FLAG_ACK) {
					if (client->index + 1 == header->index_ack) {
						client->index = header->index_ack;
					}
				}
			}
			break;
		}
	}
}
