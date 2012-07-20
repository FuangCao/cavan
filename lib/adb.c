/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Thu Jul 19 10:31:41 CST 2012
 */

#include <cavan.h>
#include <cavan/adb.h>

int adb_read_status(int sockfd, char *buff, size_t size)
{
	size_t length;
	ssize_t recvlen;

	recvlen = inet_recv(sockfd, buff, 4);
	if (recvlen < 0)
	{
		text_copy(buff, "protocol fault (no status)");
		return recvlen;
	}

	buff[recvlen] = 0;
	println("status = %s", buff);

	if (text_lhcmp("OKAY", buff) == 0)
	{
		return 0;
	}

	if(text_lhcmp("FAIL", buff) == 0)
	{
		text_copy(buff, "protocol fault (status FAIL)");
		return -1;
	}

	recvlen = inet_recv(sockfd, buff, 4);
	if (recvlen < 0)
	{
		text_copy(buff, "protocol fault (status len)");
		return recvlen;
	}

	buff[recvlen] = 0;
	length = text2value_unsigned(buff, NULL, 16);
	recvlen = inet_recv(sockfd, buff, length >= size ? size - 1 : length);
	if (recvlen < 0)
	{
		text_copy(buff, "protocol fault (status read)");
		return recvlen;
	}

	buff[recvlen] = 0;
	println("status = %s", buff);

	return -1;
}

int adb_send_text(int sockfd, const char *text)
{
	int ret;
	ssize_t sendlen;
	size_t length;
	char buff[16];
	char status[256];

	length = text_len(text);
	sprintf(buff, "%04x", length);
	sendlen = inet_send(sockfd, buff, 4);
	if (sendlen < 0)
	{
		pr_red_info("inet_send");
		return sendlen;
	}

	sendlen = inet_send(sockfd, text, length);
	if (sendlen < 0)
	{
		return sendlen;
	}

	ret = adb_read_status(sockfd, status, sizeof(status));
	if (ret < 0)
	{
		pr_red_info("status = `%s'", status);
		return ret;
	}

	return 0;
}

int adb_connect_service_base(const char *ip, u16 port)
{
	int sockfd;
	int i;
	u16 ports[] = {port, ADB_PORT1, ADB_PORT2};
	struct sockaddr_in addr;

	sockfd = inet_socket(SOCK_STREAM);
	if (sockfd < 0)
	{
		print_error("socket");
		return sockfd;
	}

	if (ip == NULL)
	{
		ip = LOCAL_HOST_IP;
	}

	println("IP = %s", ip);

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip);

	for (i = 0; i < NELEM(ports); i++)
	{
		if (ports[i] == 0)
		{
			continue;
		}

		pr_std_info("Try port %04d", ports[i]);
		addr.sin_port = htons(ports[i]);

		if (inet_connect(sockfd, &addr) >= 0)
		{
			return sockfd;
		}
	}

	close(sockfd);

	return -ENOENT;
}

int adb_connect_service(int sockfd, const char *service)
{
	int ret;

	println("service = %s", service);

	if (text_lhcmp("host", service))
	{
		ret = adb_send_text(sockfd, "host:transport-any");
		if (ret < 0)
		{
			return ret;
		}
	}

	ret = adb_send_text(sockfd, service);
	if (ret < 0)
	{
		return ret;
	}

	return 0;
}

int adb_connect_service2(const char *ip, u16 port, const char *service)
{
	int ret;
	int sockfd;

	sockfd = adb_connect_service_base(ip, port);
	if (sockfd < 0)
	{
		pr_red_info("adb_create_link");
		return sockfd;
	}

	ret = adb_connect_service(sockfd, service);
	if (ret < 0)
	{
		pr_red_info("adb_connect_service");
		close(sockfd);
		return ret;
	}

	return sockfd;
}

int adb_create_tcp_link(const char *ip, u16 port, u16 tcp_port)
{
	char service[32];

	sprintf(service, "tcp:%04d", tcp_port);

	return adb_connect_service2(ip, port, service);
}

char *adb_parse_sms_single(const char *buff, const char *end, char *segments[], size_t size)
{
	int i;
	char *p;

	end = text_find_line_end(buff, end);
	if (end == NULL)
	{
		return NULL;
	}

	for (i = 0, size--; i < size; i++)
	{
		for (p = segments[i]; buff < end; p++, buff++)
		{
			if (*buff == ',')
			{
				break;
			}

			*p = *buff;
		}

		buff++;
		*p = 0;
	}

	for (p = segments[i]; buff < end; buff++, p++)
	{
		*p = *buff;
	}

	*p = 0;

	return (char *)end;
}

char *adb_parse_sms_multi(const char *buff, const char *end)
{
	char mobile[32];
	char time[32];
	char content[1024];
	char *segments[] =
	{
		mobile, time, content
	};

	while (1)
	{
 		const char *temp = adb_parse_sms_single(buff, end, segments, NELEM(segments));
		if (temp == NULL)
		{
			break;
		}

		println("mobile = %s", mobile);
		println("time = %s", time);
		println("content = %s", content);

		for (buff = temp; buff < end && (*buff == '\r' || *buff == '\n'); buff++);
	}

	return (char *)buff;
}

char *adb_parse_sms_main(char *buff, char *end)
{
	char *p = adb_parse_sms_multi(buff, end);

	while (p < end)
	{
		*buff++ = *p++;
	}

	return buff;
}

int frecv_text_and_write(int sockfd, int fd)
{
	char buff[4096], *p, *p_end;
	ssize_t recvlen;
	ssize_t writelen;

	p = buff;
	p_end = buff + sizeof(buff);

	while (1)
	{
		recvlen = recv(sockfd, p, p_end - p, 0);
		if (recvlen <= 0)
		{
			pr_red_info("inet_recv");
			return recvlen;
		}

		// println("recvlen = %d", recvlen);
		p = adb_parse_sms_main(buff, p + recvlen);

		writelen = ffile_write(fd, buff, p - buff);
		if (writelen < 0)
		{
			pr_red_info("ffile_write");
			return writelen;
		}
	}

	return 0;
}

int recv_text_and_write(int sockfd, const char *filename)
{
	int ret;
	int fd;

	fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC | O_SYNC, 0777);
	if (fd < 0)
	{
		print_error("Failed to open file `%s'", filename);
		return fd;
	}

	ret = frecv_text_and_write(sockfd, fd);
	close(fd);

	return ret;
}
