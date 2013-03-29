/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Wed Mar 27 18:25:16 CST 2013
 */

#include <cavan.h>
#include <cavan/adb.h>
#include <cavan/network.h>
#include <cavan/service.h>
#include <cavan/screen_mirror.h>

static int screen_mirror_service_handler(struct cavan_service_description *service, int index, cavan_shared_data_t data)
{
	int ret;
	int client_sockfd;
	struct sockaddr_in client_addr;
	socklen_t client_addrlen;
	char buff[1024];
	int sockfd = data.type_int;

	client_sockfd = inet_accept(sockfd, &client_addr, &client_addrlen);
	if (client_sockfd < 0)
	{
		pr_red_info("inet_accept");
		return client_sockfd;
	}

	inet_show_sockaddr(&client_addr);

	ret = inet_recv(client_sockfd, buff, sizeof(buff));
	if (ret > 0)
	{
		buff[ret] = 0;
		println("buff = %s", buff);
	}

	inet_close_tcp_socket(client_sockfd);

	return ret;
}

int screen_mirror_service_run(struct screen_mirror_service *service)
{
	int sockfd;

	sockfd = inet_create_tcp_service(6789);
	if (sockfd < 0)
	{
		pr_red_info("inet_create_tcp_service");

		return sockfd;
	}

	service->service.data.type_int = sockfd;
	service->service.handler = screen_mirror_service_handler;

	return cavan_service_run(&service->service);
}

int screen_mirror_client(const char *buff, size_t size)
{
	int ret;
	int sockfd;

	sockfd = adb_create_tcp_link2("127.0.0.1", 6789);
	if (sockfd < 0)
	{
		pr_red_info("adb_create_tcp_link2");
		return sockfd;
	}

	ret = inet_send(sockfd, buff, size);

	msleep(100);

	inet_close_tcp_socket(sockfd);

	return ret;
}
