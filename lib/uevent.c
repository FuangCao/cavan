// Fuang.Cao <cavan.cfa@gmail.com> Fri May 13 17:54:01 CST 2011

#include <cavan.h>
#include <cavan/uevent.h>
#include <cavan/text.h>
#include <cavan/parser.h>

int uevent_init(struct uevent_desc *desc)
{
	int ret;
	int buffsize;
	int sockfd;
	struct sockaddr_nl addr;

	sockfd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
	if (sockfd < 0)
	{
		print_error("socket");
		return sockfd;
	}

	buffsize = KB(64);
    ret = setsockopt(sockfd, SOL_SOCKET, SO_RCVBUFFORCE, &buffsize, sizeof(buffsize));
	if (ret < 0)
	{
		print_error("setsockopt");
		goto out_close_sockfd;
	}

	addr.nl_family = AF_NETLINK;
	addr.nl_pid = getpid();
	addr.nl_groups = 0xFFFFFFFF;
	addr.nl_pad = 0;

	ret = bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0)
	{
		print_error("bind");
		goto out_close_sockfd;
	}

	desc->sockfd = sockfd;

	return 0;

out_close_sockfd:
	close(sockfd);

	return ret;
}

void uevent_uninit(struct uevent_desc *desc)
{
	close(desc->sockfd);
}

int get_device_uevent(struct uevent_desc *desc, const char *actions[], const char *typename, const char *type, char *devname)
{
	int sockfd = desc->sockfd;

	while (1)
	{
		int i;
		int ret;
		char recvbuff[1024], *p, *end_p;
		char option[32], devtype[32];

		ret = recv(sockfd, recvbuff, sizeof(recvbuff), 0);
		if (ret < 0)
		{
			print_error("recv");
			return ret;
		}

		for (i = 0; actions[i] && text_lhcmp(actions[i], recvbuff); i++);

		if (actions[i] == NULL)
		{
			continue;
		}

		p = recvbuff;
		end_p = recvbuff + ret;
		devtype[0] = devname[0] = 0;

		while (1)
		{
			while (*p++);

			if (p >= end_p)
			{
				break;
			}

			if (text_lhcmp(typename, p) == 0)
			{
				parse_parameter_base(p, option, devtype);

				if (devname[0])
				{
					break;
				}
			}
			else if (text_lhcmp("DEVNAME", p) == 0)
			{
				parse_parameter_base(p, option, devname);

				if (devtype[0])
				{
					break;
				}
			}
		}

		if (devname[0] == 0 || text_cmp(type, devtype))
		{
			continue;
		}

		return 0;
	}
}

