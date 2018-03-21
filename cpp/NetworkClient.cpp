/*
 * File:		NetworkClient.cpp
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-03-21 10:29:51
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
#include <cavan++/NetworkClient.h>

#define NETWORK_DATA_MASK	0xAA

int NetworkClient::openSocket(void)
{
	return socket(PF_INET, SOCK_STREAM, 0);
}

void NetworkClient::closeSocket(int sockfd)
{
	shutdown(sockfd, SHUT_RDWR);
	close(sockfd);
}

bool NetworkClient::connect(const NetworkUrl *url)
{
	int sockfd;

	sockfd = openSocket();
	if (sockfd < 0) {
		return false;
	}

#if 0
	if (url->connect(sockfd)) {
		mSockfd = sockfd;
		return true;
	}
#endif

	closeSocket(sockfd);

	return false;
}

ssize_t NetworkClient::send(const void *buff, size_t size)
{
	size_t remain = size;
	int retry = 100;

	while (remain > 0) {
		ssize_t wrlen = sendRaw(buff, remain);
		if (wrlen > 0) {
			buff = (char *) buff + wrlen;
			remain -= wrlen;
		} else {
			if (wrlen < 0 && ERRNO_NOT_RETRY()) {
				return wrlen;
			}

			if (retry <= 0) {
				return wrlen;
			}

			msleep(100);
		}
	}

	return size;
}

ssize_t NetworkClient::recv(void *buff, size_t size)
{
	int retry = 100;

	while (1) {
		ssize_t rdlen = recvRaw(buff, size);
		if (rdlen >= 0 || retry <= 0 || ERRNO_NOT_RETRY()) {
			return rdlen;
		}

		msleep(100);
		retry--;
	}
}

ssize_t NetworkClient::fill(void *buff, size_t size)
{
	ssize_t remain = size;

	while (remain > 0) {
		ssize_t rdlen = recv(buff, remain);
		if (rdlen > 0) {
			buff = (char *) buff + rdlen;
			remain -= rdlen;
		} else {
			return size - remain;
		}
	}

	return size;
}

void NetworkClient::mask(uchar *buff, size_t size)
{
	for (uchar *end = buff + size; buff < end; buff++) {
		*buff ^= NETWORK_DATA_MASK;
	}
}

void NetworkClient::mask(const uchar *src, uchar *dest, size_t size)
{
	for (const uchar *end = src + size; src < end; src++, dest++) {
		*dest = (*src) ^ NETWORK_DATA_MASK;
	}
}

ssize_t NetworkClient::sendPacket(const void *buff, size_t size)
{
	if (!send<u16>(size)) {
		return -EIO;
	}

	if (size > 0 && send(buff, size) < (ssize_t) size) {
		return -EIO;
	}

	return size;
}

ssize_t NetworkClient::recvPacket(void *buff, size_t size)
{
	u16 length;

	if (!recv<u16>(&length)) {
		return -EFAULT;
	}

	if (length > size) {
		return -ENOMEM;
	}

	if (length > 0 && recv(buff, length) < length) {
		return -EIO;
	}

	return length;
}

ssize_t NetworkClient::sendMasked(const void *buff, size_t size)
{
	if (size < 4096) {
		uchar data[size];
		mask((const uchar *) buff, data, size);
		return send(data, size);
	}

	uchar *data = new uchar[size];

	if (data == NULL) {
		return -ENOMEM;
	}

	mask((const uchar *) buff, data, size);
	ssize_t wrlen = send(data, size);
	delete[] data;

	return wrlen;
}

ssize_t NetworkClient::recvMasked(void *buff, size_t size)
{
	ssize_t rdlen = recv(buff, size);

	if (rdlen > 0) {
		mask((uchar *) buff, rdlen);
	}

	return rdlen;
}
