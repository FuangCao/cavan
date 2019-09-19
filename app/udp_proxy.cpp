#define CAVAN_CMD_NAME udp_proxy

/*
 * File:		udp_proxy.cpp
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-09-03 19:39:13
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
#include <cavan/command.h>
#include <cavan++/Udp.h>

class UdpProxyService : public UdpSock {
public:
	class UdpProxyLink : public UdpLink {
	private:
		struct network_client mClient;

		static void *TcpReceiveThread(void *data) {
			UdpProxyLink *link = (UdpProxyLink *) data;
			struct network_client *client = link->getTcpClient();

			while (1) {
				char buff[1024];
				int rdlen;

				rdlen = network_client_recv(client, buff, sizeof(buff));
				if (rdlen <= 0) {
					break;
				}

				link->send(buff, rdlen, false);
			}

			return NULL;
		}

	public:
		UdpProxyLink(UdpProxyService *service, const struct sockaddr_in *addr, u16 channel) : UdpLink(service, addr, channel) {}

		virtual struct network_client *getTcpClient(void) {
			return &mClient;
		}

	protected:
		virtual bool onUdpAccepted(void) __OVERRIDE {
			network_client_open2(&mClient, "127.0.0.1:1234", 0);
			cavan_pthread_run(TcpReceiveThread, this);
			return true;
		}

		virtual void onUdpDataReceived(const void *buff, u16 length) __OVERRIDE {
			// println("onUdpDataReceived: %d", length);
			network_client_send(&mClient, buff, length);
		}
	};

public:
	UdpProxyService(void) {}

protected:
	virtual UdpLink *newUdpLink(const struct sockaddr_in *addr, u16 channel) {
		return new UdpProxyLink(this, addr, channel);
	}
};

class UdpProxyClient : public UdpSock {
public:
	class UdpProxyLink : public UdpLink {
	private:
		struct network_client *mClient;

	public:
		UdpProxyLink(UdpProxyClient *client, const struct sockaddr_in *addr, u16 channel) : UdpLink(client, addr, channel) {}

		virtual struct network_client *getNetworkClient(void) {
			return mClient;
		}

		virtual void setNetworkClient(struct network_client *client) {
			mClient = client;
		}

	protected:
		virtual void onUdpDataReceived(const void *buff, u16 length) {
			// println("onUdpDataReceived: %d", length);
			mClient->send(mClient, buff, length);
		}
	};

	static void *ProxyThread(void *data) {
		UdpProxyClient *proxy = (UdpProxyClient *) data;
		UdpProxyLink *link = (UdpProxyLink *) proxy->accept();
		struct network_client *client = link->getNetworkClient();

		while (1) {
			char buff[1024];
			int rdlen;

			rdlen = network_client_recv(client, buff, sizeof(buff));
			if (rdlen <= 0) {
				break;
			}

			link->send(buff, rdlen, false);
		}

		return NULL;
	}

	static void *AcceptThread(void *data) {
		UdpProxyClient *proxy = (UdpProxyClient *) data;
		struct network_service service;

		pr_pos_info();

		if (network_service_open2(&service, "1233", 0) < 0) {
			return NULL;
		}

		pr_pos_info();

		while (1) {
			struct network_client *client = (struct network_client *) malloc(sizeof(struct network_client));
			if (client == NULL) {
				break;
			}

			if (network_service_accept(&service, client, 0) < 0) {
				break;
			}

			UdpProxyLink *link = (UdpProxyLink *) proxy->connect("127.0.0.1:1234");
			link->setNetworkClient(client);
			cavan_pthread_run(ProxyThread, proxy);
		}

		pr_pos_info();

		return NULL;
	}

private:
	pthread_t mAcceptThread;

public:
	UdpProxyClient(void) {}

protected:
	virtual UdpLink *newUdpLink(const struct sockaddr_in *addr, u16 channel) {
		return new UdpProxyLink(this, addr, channel);
	}

public:
	virtual int start(void) __OVERRIDE {
		int ret;

		ret = UdpSock::start();
		if (ret < 0) {
			return ret;
		}

		return cavan_pthread_create(&mAcceptThread, AcceptThread, this, true);
	}

	virtual int join(void) __OVERRIDE {
		int ret;

		ret = UdpSock::join();
		if (ret < 0) {
			return ret;
		}

		return cavan_pthread_join(mAcceptThread);
	}
};

static int cavan_udp_proxy_client_main(int argc, char *argv[])
{
	UdpProxyClient client;

	catch_sigsegv();

	if (client.open(0) < 0) {
		return -EFAULT;
	}

	if (client.start() < 0) {
		return -EFAULT;
	}

	client.join();

	return 0;
}

static int cavan_udp_proxy_service_main(int argc, char *argv[])
{
	UdpProxyService service;

	assert(argc > 1);

	catch_sigsegv();

	if (service.open(atoi(argv[1])) < 0) {
		return -EFAULT;
	}

	if (service.start() < 0) {
		return -EFAULT;
	}

	service.join();

	return 0;
}

CAVAN_COMMAND_MAP_START {
	{ "client", cavan_udp_proxy_client_main },
	{ "service", cavan_udp_proxy_service_main },
} CAVAN_COMMAND_MAP_END
