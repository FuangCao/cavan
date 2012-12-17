/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Mon Dec 17 15:10:18 CST 2012
 */

#include <cavan.h>
#include <cavan/tcp_proxy.h>

static void *tcp_proxy_send_thread_handle(void *data)
{
	struct tcp_proxy_translator *translator = data;

	inet_tcp_transmit_loop(translator->client_sockfd, translator->proxy_sockfd);

	pthread_mutex_lock(&translator->lock);
	pthread_cond_broadcast(&translator->cond);
	pthread_mutex_unlock(&translator->lock);

	return NULL;
}

static void *tcp_proxy_receive_thread_handle(void *data)
{
	struct tcp_proxy_translator *translator = data;

	inet_tcp_transmit_loop(translator->proxy_sockfd, translator->client_sockfd);

	pthread_mutex_lock(&translator->lock);
	pthread_cond_broadcast(&translator->cond);
	pthread_mutex_unlock(&translator->lock);

	return NULL;
}

static int tcp_proxy_service_handle(int index, cavan_shared_data_t data)
{
	int ret;
	socklen_t addrlen;
	struct sockaddr_in addr;
	struct tcp_proxy_service *proxy_service = data.type_void;
	int client_sockfd, proxy_sockfd;
	int server_sockfd = proxy_service->sockfd;
	struct tcp_proxy_translator translator;
	pthread_t send_thread, recv_thread;

	client_sockfd = inet_accept(server_sockfd, &addr, &addrlen);
	if (client_sockfd < 0)
	{
		print_error("inet_accept");
		return client_sockfd;
	}

	pr_bold_info("IP = %s, port = %d", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

	proxy_sockfd = proxy_service->open_connect(proxy_service->proxy_ip, proxy_service->proxy_port);
	if (proxy_sockfd < 0)
	{
		ret = proxy_sockfd;
		pr_red_info("inet_connect");
		goto out_close_client_sockfd;
	}

	ret = pthread_mutex_init(&translator.lock, NULL);
	if (ret < 0)
	{
		pr_error_info("pthread_mutex_init");
		goto out_close_proxy_sockfd;
	}

	ret = pthread_cond_init(&translator.cond, NULL);
	if (ret < 0)
	{
		pr_error_info("pthread_cond_init");
		goto out_pthread_mutex_destroy;
	}

	translator.client_sockfd = client_sockfd;
	translator.proxy_sockfd = proxy_sockfd;

	ret = pthread_create(&send_thread, NULL, tcp_proxy_send_thread_handle, &translator);
	if (ret < 0)
	{
		pr_error_info("pthread_create");
		goto out_pthread_cond_destroy;
	}

	ret = pthread_create(&recv_thread, NULL, tcp_proxy_receive_thread_handle, &translator);
	if (ret < 0)
	{
		pr_error_info("pthread_create");
		goto out_pthread_cond_destroy;
	}

	pthread_mutex_lock(&translator.lock);
	pthread_cond_wait(&translator.cond, &translator.lock);
	pthread_mutex_unlock(&translator.lock);

	pthread_cancel(send_thread);
	pthread_cancel(recv_thread);

out_pthread_cond_destroy:
	pthread_cond_destroy(&translator.cond);
out_pthread_mutex_destroy:
	pthread_mutex_destroy(&translator.lock);
out_close_proxy_sockfd:
	close(proxy_sockfd);
out_close_client_sockfd:
	close(client_sockfd);
	return ret;
}

int tcp_proxy_service_run(struct tcp_proxy_service *proxy_service)
{
	int ret;
	int sockfd;
	struct cavan_service_description *service;

	sockfd = inet_create_tcp_service(proxy_service->port);
	if (sockfd < 0)
	{
		pr_red_info("inet_create_tcp_service");
		return sockfd;
	}

	proxy_service->sockfd = sockfd;

	if (proxy_service->open_connect == NULL)
	{
		proxy_service->open_connect = inet_create_tcp_link2;
	}

	if (proxy_service->close_connect == NULL)
	{
		proxy_service->close_connect = inet_close_tcp_socket;
	}

	service = &proxy_service->service;
	service->data.type_void = proxy_service;
	service->handler = tcp_proxy_service_handle;
	service->threads = NULL;
	ret = cavan_service_run(service);
	cavan_service_stop(service);
	inet_close_tcp_socket(sockfd);

	return ret;
}
