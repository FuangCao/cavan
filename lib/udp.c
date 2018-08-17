/*
 * File:		udp.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-08-16 14:50:12
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
#include <cavan/udp.h>
#include <cavan/math.h>
#include <cavan/timer.h>

static bool cavan_udp_sock_post_link(struct cavan_udp_sock *sock, struct cavan_udp_link *link, u64 time);
static bool cavan_udp_sock_send_wind(struct cavan_udp_sock *sock, struct cavan_udp_link *link);

static inline void cavan_udp_sock_lock(struct cavan_udp_sock *sock)
{
	pthread_mutex_lock(&sock->lock);
}

static inline void cavan_udp_sock_unlock(struct cavan_udp_sock *sock)
{
	pthread_mutex_unlock(&sock->lock);
}

static inline void cavan_udp_sock_wait(struct cavan_udp_sock *sock)
{
	pthread_cond_wait(&sock->cond, &sock->lock);
}

static inline void cavan_udp_sock_timedwait(struct cavan_udp_sock *sock, const struct timespec *time)
{
	pthread_cond_timedwait(&sock->cond, &sock->lock, time);
}

static inline void cavan_udp_sock_wakeup(struct cavan_udp_sock *sock)
{
	pthread_cond_signal(&sock->cond);
}

static inline void cavan_udp_link_lock(struct cavan_udp_link *link)
{
	pthread_mutex_lock(&link->lock);
}

static inline void cavan_udp_link_unlock(struct cavan_udp_link *link)
{
	pthread_mutex_unlock(&link->lock);
}

static inline void cavan_udp_win_wait(struct cavan_udp_win *win, struct cavan_udp_link *link)
{
	pthread_cond_wait(&win->cond, &link->lock);
}

static inline int cavan_udp_win_timedwait(struct cavan_udp_win *win, struct cavan_udp_link *link, const struct timespec *time)
{
	return pthread_cond_timedwait(&win->cond, &link->lock, time);
}

static int cavan_udp_win_delayedwait(struct cavan_udp_win *win, struct cavan_udp_link *link, u32 delay)
{
	struct timespec time;

	clock_gettime_real(&time);
	cavan_timespec_add_ms(&time, delay);

	return cavan_udp_win_timedwait(win, link, &time);
}

static inline void cavan_udp_win_wakeup(struct cavan_udp_win *win)
{
	pthread_cond_signal(&win->cond);
}

static bool cavan_udp_win_invalid(struct cavan_udp_win *win, u16 sequence)
{
	u16 last = win->index + NELEM(win->packs) - 1;

	if (win->index < last) {
		return (sequence < win->index || sequence > last);
	} else {
		return (sequence < win->index && sequence > last);
	}
}

static bool cavan_udp_link_set_sequence(struct cavan_udp_link *link, u16 sequence)
{
	if (sequence == link->sequence) {
		return false;
	}

	if (sequence < link->sequence) {
		u16 end = link->sequence + CAVAN_UDP_WIN_SIZE;

		if (link->sequence < end || sequence > end) {
			println("%d -> %d -> %d", link->sequence, sequence, end);
			return false;
		}
	}

	link->sequence = sequence;

	return true;
}

static struct cavan_udp_pack *cavan_udp_pack_alloc(u16 length)
{
	struct cavan_udp_pack *pack;

	pack = (struct cavan_udp_pack *) malloc(sizeof(struct cavan_udp_pack) + length);
	if (pack == NULL) {
		return NULL;
	}

	pack->length = length;
	pack->times = 0;
	pack->time = clock_gettime_real_ms();

	return pack;
}

static bool cavan_udp_win_resend(struct cavan_udp_win *win, struct cavan_udp_link *link, struct cavan_udp_sock *sock, u64 time);

static int cavan_udp_link_flush(struct cavan_udp_sock *sock, struct cavan_udp_link *link, u64 time)
{
#if 0
	struct cavan_udp_header *header = &pack->header;
	struct cavan_udp_link *link = sock->links[header->src_channel];
	bool writeable = true;
	int delay;

	if (link == NULL) {
		return 0;
	}

	cavan_udp_link_lock(link);

	switch (header->type) {
	case CAVAN_UDP_WIND:
		if (link->wind == pack) {
			header->sequence = link->recv_win.index + NELEM(link->recv_win.packs);
			delay = 500;
		} else {
			writeable = false;
			delay = 0;
		}
		break;

	case CAVAN_UDP_PING:
		delay = 60000;
		break;

	default:
		if (cavan_udp_win_invalid(&link->send_win, header->sequence) || cavan_udp_win_get_pack(&link->send_win, header->sequence) == NULL) {
			delay = 0;
		} else {
			if (pack->times > 0) {
				u16 sequence = pack->header.sequence;
				u16 cwnd = link->send_win.index + link->cwnd;

				pr_pos_info();

				if (sequence > cwnd && sequence - cwnd < CAVAN_UDP_WIN_SIZE) {
					writeable = false;
				} else {
					link->ssthresh = link->cwnd / 2;
					link->cwnd = 1;
					pack->times++;
				}
			} else {
				pack->times = 1;
			}

			delay = CAVAN_UDP_OVERTIME;

			println("times = %d", pack->times);
		}
		break;
	}

	cavan_udp_link_unlock(link);

	if (writeable) {
		int wrlen = inet_sendto(sock->sockfd, &pack->header, sizeof(pack->header) + pack->length, &link->addr);
		if (wrlen < 0) {
			return wrlen;
		}
	}

	return delay;
#else
	int delay = 0;

	if (link->wtime) {
		if (link->wtime > time) {
			delay = link->wtime - time;
		} else {
			cavan_udp_sock_send_wind(sock, link);
			link->wtime = time + CAVAN_UDP_OVERTIME;
			delay = CAVAN_UDP_OVERTIME;
		}
	}

	if (cavan_udp_win_resend(&link->send_win, link, sock, time)) {
		if (delay > CAVAN_UDP_OVERTIME || delay == 0) {
			delay = CAVAN_UDP_OVERTIME;
		}
	}

	return delay;
#endif
}

static void cavan_udp_win_init(struct cavan_udp_win *win)
{
	int i;

	pthread_cond_init(&win->cond, NULL);

	for (i = 0; i < NELEM(win->packs); i++) {
		win->packs[i] = NULL;
	}

	win->length = 0;
	win->index = 0;
	win->ready = 0;
	win->full = false;
}

static void cavan_udp_win_destroy(struct cavan_udp_win *win, bool all)
{
	u16 index, end;

	if (all) {
		index = win->index % NELEM(win->packs);
	} else {
		index = win->ready % NELEM(win->packs);
	}

	end = (win->index + win->length) % NELEM(win->packs);

	while (index != end) {
		if (win->packs[index]) {
			free(win->packs[index]);
		}

		index++;
	}

	pthread_cond_destroy(&win->cond);
}

static void cavan_udp_win_setup_ack(struct cavan_udp_win *win, struct cavan_udp_header *header, u16 sequence)
{
	header->win = win->index + NELEM(win->packs) - sequence;
	header->sequence = sequence;
}

static int cavan_udp_win_flush(struct cavan_udp_win *win, struct cavan_udp_link *link, struct cavan_udp_sock *sock)
{
	u16 sequence, end;
	u64 overtime;
	int count;

	if (win->length > link->cwnd) {
		end = win->index + link->cwnd;

		if (win->ready >= end && win->ready - end < NELEM(win->packs)) {
			return 0;
		}
	} else {
		end = win->index + win->length;
	}

	sequence = link->sequence;

	if (sequence < win->ready) {
		if (sequence > NELEM(win->packs) || win->ready - sequence < NELEM(win->packs)) {
			return 0;
		}

		if (sequence > end || win->ready <= end) {
			sequence = end;
		}
	} else {
		if (sequence > end && win->ready <= end) {
			sequence = end;
		}
	}

	overtime = clock_gettime_real_ms() + CAVAN_UDP_OVERTIME;
	count = 0;

	cavan_udp_sock_lock(sock);

	while (win->ready != sequence) {
		u16 index = win->ready % NELEM(win->packs);
		struct cavan_udp_pack *pack = win->packs[index];
		if (pack != NULL) {
			println("flush[%d]: %d", win->ready, pack->header.sequence);
			inet_sendto(sock->sockfd, &pack->header, pack->length + sizeof(pack->header), &link->addr);
			pack->time = overtime;
			count++;
		}

		win->ready++;
	}

	if (count > 0) {
		cavan_udp_sock_post_link(sock, link, overtime);
	}

	cavan_udp_sock_unlock(sock);

	return count;
}

static bool cavan_udp_win_resend(struct cavan_udp_win *win, struct cavan_udp_link *link, struct cavan_udp_sock *sock, u64 time)
{
	u16 index = win->index;
	u16 end;

	for (end = index + MIN(win->length, link->cwnd); index != end && index != win->ready; index++) {
		struct cavan_udp_pack *pack = win->packs[index % NELEM(win->packs)];

		if (pack != NULL) {
			if (pack->time <= time) {
				println("resend: %d", pack->header.sequence);
				inet_sendto(sock->sockfd, &pack->header, sizeof(pack->header) + pack->length, &link->addr);
				pack->time = time + CAVAN_UDP_OVERTIME;

				win->ready = index + 1;
				link->ssthresh = link->cwnd / 2;
				link->cwnd = 1;
			}

			return true;
		}
	}

	return false;
}

static bool cavan_udp_win_enqueue(struct cavan_udp_win *win, struct cavan_udp_pack *pack)
{
	if (win->length < NELEM(win->packs)) {
		struct cavan_udp_header *header = &pack->header;
		u16 sequence = win->index + win->length;
		int index = sequence % NELEM(win->packs);
		header->sequence = sequence;
		win->packs[index] = pack;
		win->length++;
		return true;
	}

	return false;
}

static struct cavan_udp_pack *cavan_udp_win_dequeue(struct cavan_udp_win *win, u16 channel, struct cavan_udp_link *link, struct cavan_udp_sock *sock)
{
	if (win->index != win->ready) {
		int index = win->index % NELEM(win->packs);
		struct cavan_udp_pack *pack;

		pack = win->packs[index];
		win->packs[index] = NULL;
		win->length--;
		win->index++;

		if (win->full && win->length < CAVAN_UDP_WIN_SIZE / 2) {
			win->full = false;
			cavan_udp_sock_send_wind(sock, link);
			link->wtime = clock_gettime_real_ms() + CAVAN_UDP_OVERTIME;

			cavan_udp_sock_lock(sock);
			cavan_udp_sock_post_link(sock, link, link->wtime);
			cavan_udp_sock_unlock(sock);
		}

		return pack;
	}

	return NULL;
}

static int cavan_udp_win_receive(struct cavan_udp_win *win, struct cavan_udp_link *link, struct cavan_udp_header *header, u16 length)
{
	struct cavan_udp_pack *pack;
	int count;
	u16 index;

	if (cavan_udp_win_invalid(win, header->sequence)) {
		return 0; // -EINVAL;
	}

	index = header->sequence % NELEM(win->packs);
	if (win->packs[index]) {
		return 0;
	}

	pack = cavan_udp_pack_alloc(length);
	if (pack == NULL) {
		return -ENOMEM;
	}

	memcpy(pack->header.data, header->data, length);
	win->packs[index] = pack;

	for (count = 0; win->packs[win->ready % NELEM(win->packs)]; count++) {
		println("recv: %d, length = %d", win->ready, win->length);

		if (win->length < NELEM(win->packs)) {
			win->length++;

			if (win->length == 1) {
				cavan_udp_win_wakeup(win);
			}

			win->ready++;
		} else {
			win->full = true;
			break;
		}
	}

	return count;
}

static int cavan_udp_win_ack(struct cavan_udp_win *win, struct cavan_udp_link *link, struct cavan_udp_sock *sock, struct cavan_udp_header *header)
{
	int count;
	u16 index;

	if (cavan_udp_win_invalid(win, header->sequence)) {
		return -1;
	}

	index = header->sequence % NELEM(win->packs);
	cavan_udp_link_set_sequence(link, header->sequence + header->win);
	println("win = %d", link->sequence);

	if (cavan_udp_link_set_sequence(link, header->sequence + header->win)) {
		cavan_udp_win_flush(win, link, sock);
	}

	if (win->packs[index] == NULL) {
		return 0;
	}

	free(win->packs[index]);
	win->packs[index]->time = 0;
	win->packs[index] = NULL;

	if (link->cwnd < link->ssthresh || link->acks > link->cwnd) {
		if (link->cwnd < CAVAN_UDP_CWND) {
			link->cwnd++;
		}

		link->acks = 0;
	} else {
		link->acks++;
	}

	println("cwnd = %d, ssthresh = %d, acks = %d", link->cwnd, link->ssthresh, link->acks);

	for (count = 0; win->length > 0 && win->packs[win->index % NELEM(win->packs)] == NULL; count++) {
		win->length--;
		win->index++;
	}

	println("count = %d, full = %d, length = %d", count, win->full, win->length);

	if (count > 0 && win->full) {
		win->full = false;
		cavan_udp_win_wakeup(win);
	}

	return count;
}

static void cavan_udp_link_init(struct cavan_udp_link *link, u16 channel)
{
	pthread_mutex_init(&link->lock, NULL);

	cavan_udp_win_init(&link->send_win);
	cavan_udp_win_init(&link->recv_win);

	link->local_channel = channel;
	link->remote_channel = 0;

	link->sequence = 1;
	link->cwnd = 1;
	link->acks = 0;
	link->ssthresh = 16;
	link->prev = link->next = link;
}

static void cavan_udp_link_deinit(struct cavan_udp_link *link)
{
	cavan_udp_link_lock(link);
	cavan_udp_win_destroy(&link->send_win, false);
	cavan_udp_win_destroy(&link->recv_win, true);
	cavan_udp_link_unlock(link);

	pthread_mutex_destroy(&link->lock);
}

int cavan_udp_sock_open(struct cavan_udp_sock *sock, u16 port)
{
	int sockfd;
	int i;

	sockfd = inet_socket(SOCK_DGRAM);
	if (sockfd < 0) {
		pr_err_info("inet_socket");
		return sockfd;
	}

	if (port > 0) {
		struct sockaddr_in addr;
		int ret;

		inet_sockaddr_init(&addr, NULL, port);

		ret = inet_bind(sockfd, &addr);
		if (ret < 0) {
			close(sockfd);
			return ret;
		}
	}

	sock->sockfd = sockfd;
	sock->head = NULL;
	sock->index = 0;

	for (i = 0; i < NELEM(sock->links); i++) {
		sock->links[i] = NULL;
	}

	pthread_mutex_init(&sock->lock, NULL);
	pthread_cond_init(&sock->cond, NULL);

	return 0;
}

void cavan_udp_sock_close(struct cavan_udp_sock *sock)
{
	pthread_cond_destroy(&sock->cond);
	pthread_mutex_destroy(&sock->lock);
}

static void cavan_udp_sock_remove_link(struct cavan_udp_link *link)
{
	link->prev->next = link->next;
	link->next->prev = link->prev;
	link->prev = link->next = link;
}

static void cavan_udp_sock_add_link(struct cavan_udp_link *link, struct cavan_udp_link *prev, struct cavan_udp_link *next)
{
	prev->next = link;
	link->prev = prev;
	next->prev = link;
	link->next = next;
}

static bool cavan_udp_sock_post_link(struct cavan_udp_sock *sock, struct cavan_udp_link *link, u64 time)
{
	if (link != link->next) {
		return false;
	}

	link->time = time;

	if (sock->head == NULL) {
		sock->head = link;
		cavan_udp_sock_wakeup(sock);
	} else {
		struct cavan_udp_link *node = sock->head;

		while (1) {
			if (node->time > time) {
				cavan_udp_sock_add_link(link, node->prev, node);
				break;
			}

			node = node->next;

			if (node == sock->head) {
				cavan_udp_sock_add_link(link, node->prev, node);
				cavan_udp_sock_wakeup(sock);
				break;
			}
		}
	}

	return true;
}

static bool cavan_udp_sock_enqueue(struct cavan_udp_sock *sock, struct cavan_udp_link *link, struct cavan_udp_pack *pack, bool nonblock)
{
	bool success;

	cavan_udp_link_lock(link);

	while (1) {
		struct cavan_udp_win *win = &link->send_win;

		success = cavan_udp_win_enqueue(win, pack);
		if (success || nonblock) {
			break;
		}

		win->full = true;
		cavan_udp_win_wait(win, link);
	}

	cavan_udp_win_flush(&link->send_win, link, sock);

	cavan_udp_link_unlock(link);

	return success;
}

static bool cavan_udp_sock_send_wind(struct cavan_udp_sock *sock, struct cavan_udp_link *link)
{
	struct cavan_udp_win *win = &link->recv_win;
	struct cavan_udp_header header;

	header.type = CAVAN_UDP_WIND;
	header.src_channel = link->local_channel;
	header.dest_channel = link->remote_channel;
	header.sequence = win->index + NELEM(win->packs);

	inet_sendto(sock->sockfd, &header, sizeof(header), &link->addr);

	return true;
}

struct cavan_udp_link *cavan_udp_link_alloc(struct cavan_udp_sock *sock)
{
	struct cavan_udp_link *link;
	int channel;

	for (channel = sock->index + 1; channel < NELEM(sock->links); channel++) {
		if (sock->links[channel] == NULL) {
			goto out_found;
		}
	}

	for (channel = 1; channel < sock->index; channel++) {
		if (sock->links[channel] == NULL) {
			goto out_found;
		}
	}

	return NULL;

out_found:
	link = cavan_malloc_type(struct cavan_udp_link);
	if (link == NULL) {
		return NULL;
	}

	cavan_udp_link_init(link, channel);
	sock->links[channel] = link;
	sock->index = channel;

	return link;
}

void cavan_udp_link_free(struct cavan_udp_sock *sock, struct cavan_udp_link *link)
{
	sock->links[link->local_channel] = NULL;
	cavan_udp_link_deinit(link);
	free(link);
}

void cavan_udp_sock_send_loop(struct cavan_udp_sock *sock)
{
	cavan_udp_sock_lock(sock);

	while (1) {
		struct cavan_udp_link *link = sock->head;
		struct timespec time;
		u64 mseconds;
		int delay;

		if (link == NULL) {
			println("cavan_udp_sock_wait");
			cavan_udp_sock_wait(sock);
			continue;
		}

		clock_gettime_real(&time);
		mseconds = cavan_timespec_mseconds(&time);

		if (link->time > mseconds) {
			u32 delay = link->time - mseconds;

			if (delay > 100) {
				cavan_timespec_add_ms(&time, delay);
				println("cavan_udp_sock_timedwait: %d", delay);
				cavan_udp_sock_timedwait(sock, &time);
				continue;
			}
		}

		sock->head = link->next;
		if (sock->head == link) {
			sock->head = NULL;
		}

		cavan_udp_sock_remove_link(link);

		cavan_udp_sock_unlock(sock);
		cavan_udp_link_lock(link);
		delay = cavan_udp_link_flush(sock, link, mseconds);
		cavan_udp_link_unlock(link);
		cavan_udp_sock_lock(sock);

		if (delay > 0) {
			cavan_udp_sock_post_link(sock, link, mseconds + delay);
		} else if (delay < 0) {
			cavan_udp_link_free(sock, link);
		}
	}

	cavan_udp_sock_unlock(sock);
}

void cavan_udp_sock_recv_loop(struct cavan_udp_sock *sock)
{
	struct cavan_udp_header *header;
	char buff[2048];

	header = (struct cavan_udp_header *) buff;

	while (1) {
		struct cavan_udp_link *link;
		struct sockaddr_in addr;
		int rdlen;

		rdlen = inet_recvfrom(sock->sockfd, buff, sizeof(buff), &addr);
		if (rdlen < 0) {
			pr_err_info("inet_recvfrom");
			break;
		}

		switch (header->type) {
		case CAVAN_UDP_SYNC:
			println("CAVAN_UDP_SYNC");

			link = cavan_udp_link_alloc(sock);
			if (link != NULL) {
				struct cavan_udp_header *response;
				struct cavan_udp_pack *pack;

				pack = cavan_udp_pack_alloc(sizeof(struct cavan_udp_header));
				if (pack == NULL) {
					break;
				}

				cavan_udp_link_lock(link);

				link->recv_win.index = 1;
				link->recv_win.ready = 1;
				memcpy(&link->addr, &addr, sizeof(addr));

				cavan_udp_link_unlock(link);

				response = &pack->header;
				response->dest_channel = header->src_channel;
				response->src_channel = link->local_channel;
				response->type = CAVAN_UDP_SYNC_ACK1;
				cavan_udp_win_setup_ack(&link->recv_win, response, 1);

				cavan_udp_sock_enqueue(sock, link, pack, true);
			}
			break;

		case CAVAN_UDP_SYNC_ACK1:
			println("CAVAN_UDP_SYNC_ACK1");

			link = sock->links[header->dest_channel];
			if (link != NULL) {
				struct cavan_udp_header response;
				int count;

				cavan_udp_link_lock(link);

				if (link->remote_channel == 0 || link->remote_channel == header->src_channel) {
					link->remote_channel = header->src_channel;
					count = cavan_udp_win_ack(&link->send_win, link, sock, header);
					println("cavan_udp_win_ack: count = %d", count);
					cavan_udp_win_wakeup(&link->recv_win);
					response.type = CAVAN_UDP_SYNC_ACK2;
				} else {
					response.type = CAVAN_UDP_ERROR;
					println("linked: %d <> %d", link->remote_channel, header->src_channel);
				}

				cavan_udp_link_unlock(link);

				response.dest_channel = header->src_channel;
				response.src_channel = header->dest_channel;

				inet_sendto(sock->sockfd, &response, sizeof(response), &addr);
			}
			break;

		case CAVAN_UDP_SYNC_ACK2:
			println("CAVAN_UDP_SYNC_ACK2: %d", header->dest_channel);
			link = sock->links[header->dest_channel];
			if (link != NULL && link->remote_channel == 0) {
				link->remote_channel = header->src_channel;

				cavan_udp_link_lock(link);
				cavan_udp_win_ack(&link->send_win, link, sock, header);
				cavan_udp_link_unlock(link);

				if (sock->on_connected) {
					sock->on_connected(sock, link);
				}
			}
			break;

		case CAVAN_UDP_DATA:
			println("CAVAN_UDP_DATA: %d", header->sequence);

			link = sock->links[header->dest_channel];
			if (link != NULL) {
				struct cavan_udp_header response;
				int count;

				cavan_udp_link_lock(link);
				count = cavan_udp_win_receive(&link->recv_win, link, header, rdlen - sizeof(struct cavan_udp_header));
				cavan_udp_win_setup_ack(&link->recv_win, &response, header->sequence);
				cavan_udp_link_unlock(link);

				println("cavan_udp_win_receive: %d", count);

				if (count < 0) {
					break;
				}

				response.dest_channel = header->src_channel;
				response.src_channel = header->dest_channel;
				response.type = CAVAN_UDP_DATA_ACK;

				inet_sendto(sock->sockfd, &response, sizeof(response), &addr);
			} else {
				struct cavan_udp_header response;

				response.dest_channel = header->src_channel;
				response.src_channel = header->dest_channel;
				response.type = CAVAN_UDP_ERROR;

				inet_sendto(sock->sockfd, &response, sizeof(response), &addr);
			}
			break;

		case CAVAN_UDP_DATA_ACK:
			println("CAVAN_UDP_DATA_ACK: %d", header->sequence);
			link = sock->links[header->dest_channel];
			if (link != NULL) {
				cavan_udp_link_lock(link);
				cavan_udp_win_ack(&link->send_win, link, sock, header);
				cavan_udp_link_unlock(link);
			}
			break;

		case CAVAN_UDP_WIND:
			link = sock->links[header->dest_channel];
			if (link != NULL) {
				struct cavan_udp_header response;

				response.dest_channel = header->src_channel;
				response.src_channel = header->dest_channel;
				response.type = CAVAN_UDP_WIND_ACK;

				inet_sendto(sock->sockfd, &response, sizeof(response), &addr);

				cavan_udp_link_lock(link);

				if (cavan_udp_link_set_sequence(link, header->sequence)) {
					cavan_udp_win_flush(&link->send_win, link, sock);
				}

				cavan_udp_link_unlock(link);
			}
			break;

		case CAVAN_UDP_WIND_ACK:
			println("CAVAN_UDP_WIND_ACK");
			link = sock->links[header->dest_channel];
			if (link != NULL) {
				cavan_udp_link_lock(link);
				link->wtime = 0;
				cavan_udp_link_unlock(link);
			}
			break;

		case CAVAN_UDP_PING: {
				struct cavan_udp_header response;

				println("CAVAN_UDP_PING");

				link = sock->links[header->dest_channel];
				if (link != NULL) {
					response.type = CAVAN_UDP_PONG;
				} else {
					response.type = CAVAN_UDP_ERROR;
				}

				response.dest_channel = header->src_channel;
				response.src_channel = header->dest_channel;

				inet_sendto(sock->sockfd, &response, sizeof(response), &addr);
			}
			break;

		case CAVAN_UDP_PONG:
			println("CAVAN_UDP_PONG: %d", header->sequence);
			break;

		case CAVAN_UDP_ERROR:
			println("CAVAN_UDP_ERROR");

			link = sock->links[header->dest_channel];
			sock->links[header->dest_channel] = NULL;

			if (link != NULL) {
				cavan_udp_link_deinit(link);
				free(link);
			}
			break;

		default:
			pr_red_info("Invalid pack: %d", header->type);
		}
	}
}

ssize_t cavan_udp_sock_send(struct cavan_udp_sock *sock, u16 channel, const void *buff, size_t size, bool nonblock)
{
	struct cavan_udp_header *header;
	struct cavan_udp_pack *pack;
	struct cavan_udp_link *link;

	link = sock->links[channel];
	if (link == NULL) {
		ERROR_RETURN(ENOENT);
	}

	pack = cavan_udp_pack_alloc(size);
	if (pack == NULL) {
		ERROR_RETURN(ENOMEM);
	}

	memcpy(pack->header.data, buff, size);

	header = &pack->header;
	header->src_channel = link->local_channel;
	header->dest_channel = link->remote_channel;
	header->type = CAVAN_UDP_DATA;

	if (cavan_udp_sock_enqueue(sock, link, pack, nonblock)) {
		return size;
	}

	free(pack);

	ERROR_RETURN(EAGAIN);
}

ssize_t cavan_udp_sock_recv(struct cavan_udp_sock *sock, u16 channel, void *buff, size_t size, bool nonblock)
{
	struct cavan_udp_link *link = sock->links[channel];
	struct cavan_udp_pack *pack;

	if (link == NULL) {
		ERROR_RETURN(ENOLINK);
	}

	cavan_udp_link_lock(link);

	while (1) {
		pack = cavan_udp_win_dequeue(&link->recv_win, channel, link, sock);
		if (nonblock || pack != NULL) {
			break;
		}

		cavan_udp_win_wait(&link->recv_win, link);
	}

	cavan_udp_link_unlock(link);

	if (pack == NULL) {
		ERROR_RETURN(EAGAIN);
	}

	if (size > pack->length) {
		size = pack->length;
	}

	memcpy(buff, pack->header.data, size);
	free(pack);

	return size;
}

int cavan_udp_sock_shutdown(struct cavan_udp_sock *sock)
{
	return 0;
}

int cavan_udp_sock_accept(struct cavan_udp_sock *sock)
{
	return 0;
}

int cavan_udp_sock_connect(struct cavan_udp_sock *sock, const char *url)
{
	struct cavan_udp_header *header;
	struct cavan_udp_pack *pack;
	struct cavan_udp_link *link;
	int ret;

	link = cavan_udp_link_alloc(sock);
	if (link == NULL) {
		pr_red_info("cavan_udp_channel_alloc");
		return -ENOMEM;
	}

	println("channel = %d", link->local_channel);

	pack = cavan_udp_pack_alloc(sizeof(struct cavan_udp_header));
	if (pack == NULL) {
		pr_red_info("cavan_udp_pack_alloc");
		goto out_cavan_udp_link_free;
	}

	inet_sockaddr_init_url(&link->addr, url);

	header = &pack->header;
	header->src_channel = link->local_channel;
	header->dest_channel = 0;
	header->type = CAVAN_UDP_SYNC;

	cavan_udp_sock_enqueue(sock, link, pack, true);

	cavan_udp_link_lock(link);
	ret = cavan_udp_win_delayedwait(&link->recv_win, link, 5000);
	cavan_udp_link_unlock(link);

	if (ret != 0) {
		pr_red_info("cavan_udp_link_delayedwait: %d", ret);
		goto out_cavan_udp_link_free;
	}

	return link->local_channel;

out_cavan_udp_link_free:
	cavan_udp_link_free(sock, link);
	return -EFAULT;
}
