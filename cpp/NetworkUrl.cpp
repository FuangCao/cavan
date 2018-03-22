/*
 * File:		NetworkUrl.cpp
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-03-21 10:30:06
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
#include <cavan++/NetworkUrl.h>
#include <cavan++/NetworkProtocol.h>

const char *NetworkUrl::DEFAULT_PROTOCOL = "tcp";
const char *NetworkUrl::DEFAULT_HOST = "127.0.0.1";

void NetworkUrl::dump(void)
{
	println("protocol = %s", mProtocol);
	println("host = %s", mHost);
	println("port = %d", mPort);
	println("path = %s", mPath);
	println("args = %s", mArgs);
}

void NetworkUrl::clear(void)
{
	mProtocol = mHost = mPath = mArgs = NULL;
	mPort = 0;
}

bool NetworkUrl::parse(const char *url, size_t size)
{
	clear();

	if (mData != NULL) {
		delete[] mData;
	}

	mData = new char[size + 1];
	if (mData == NULL) {
		return false;
	}

	int slash = 0;
	char *p = mData;
	char *value = p;
	char *port = NULL;

	for (const char *url_end = url + size; url < url_end; url++) {
		switch (*url) {
		case 0:
			goto out_complete;

		case ':':
			if (mProtocol == NULL) {
				if (p > value) {
					mProtocol = value;
					*p++ = 0;
					value = p;
				} else {
					mProtocol = DEFAULT_PROTOCOL;
				}
			} else if (mHost == NULL) {
				if (slash != 2 && slash != 0) {
					pr_red_info("Invalid `/'");
					return false;
				}

				if (p > value) {
					mHost = value;
					*p++ = 0;
				} else {
					mHost = DEFAULT_HOST;
				}

				port = value = p;
			} else {
				pr_red_info("Too much `:'");
				return false;
			}
			break;

		case '/':
			if (mPath == NULL) {
				if (mProtocol == NULL) {
					mProtocol = "file";
					mHost = DEFAULT_HOST;
				} else if (mHost == NULL) {
					if (p > value) {
						if (slash > 0) {
							if (slash != 2) {
								pr_red_info("Invalid `/'");
								return false;
							}

							mHost = value;
						} else {
							mHost = mProtocol;
							mProtocol = DEFAULT_PROTOCOL;
							port = value;
						}
					} else {
						slash++;
						break;
					}
				}

				*p++ = 0;
				mPath = value = p;
			}

			*p++ = *url;
			break;

		case '?':
			if (mArgs == NULL) {
				*p++ = 0;
				mArgs = p;
			}
			break;

		case ' ':
		case '\t':
		case '\f':
		case '\r':
		case '\n':
			if (p > value) {
				*p++ = *url;
			}
			break;

		default:
			*p++ = *url;
		}
	}

	*p = 0;

	while (mHost == NULL) {
		if (mProtocol != NULL) {
			if (slash > 0) {
				if (slash != 2) {
					pr_red_info("Invalid `/'");
					return false;
				}

				if (p > value) {
					mHost = value;
				}

				break;
			} else {
				mHost = mProtocol;
			}
		} else {
			mHost = DEFAULT_HOST;
		}

		mProtocol = DEFAULT_PROTOCOL;

		if (p > value) {
			port = value;
		}

		break;
	}

	if (port != NULL) {
		mPort = atoi(port);
	}

out_complete:
	return true;
}

void NetworkUrl::build(struct sockaddr_in *addr, bool any)
{
	addr->sin_family = PF_INET;
	addr->sin_port = htons(getPort());

	if (any) {
		addr->sin_addr.s_addr = INADDR_ANY;
	} else {
		addr->sin_addr.s_addr = inet_addr(getHost());
	}
}

NetworkProtocol *NetworkUrl::getNetworkProtocol(void)
{
	if (mProtocol == NULL) {
		pr_red_info("Need protocol!");
		return NULL;
	}

	NetworkProtocol *protocol = NetworkProtocol::instance(mProtocol);
	if (protocol == NULL) {
		pr_red_info("Invalid protocol: %s", mProtocol);
		return NULL;
	}

	return protocol;
}

NetworkClient *NetworkUrl::newClient(void)
{
	NetworkProtocol *protocol = getNetworkProtocol();
	if (protocol == NULL) {
		return NULL;
	}

	return protocol->newClient();
}

NetworkClient *NetworkUrl::openClient(void)
{
	NetworkClient *client = newClient();
	if (client != NULL) {
		if (client->open(this)) {
			return client;
		}

		delete client;
	}

	return NULL;
}

NetworkClient *NetworkUrl::openClient(const char *url_text)
{
	NetworkUrl url;

	if (url.parse(url_text)) {
		return url.openClient();
	}

	return NULL;
}

NetworkService *NetworkUrl::newService(void)
{
	NetworkProtocol *protocol = getNetworkProtocol();
	if (protocol == NULL) {
		return NULL;
	}

	return protocol->newService();
}

NetworkService *NetworkUrl::openService(void)
{
	NetworkService *service = newService();
	if (service != NULL) {
		if (service->open(this)) {
			return service;
		}

		delete service;
	}

	return NULL;
}

NetworkService *NetworkUrl::openService(const char *url_text)
{
	NetworkUrl url;

	if (url.parse(url_text)) {
		return url.openService();
	}

	return NULL;
}
