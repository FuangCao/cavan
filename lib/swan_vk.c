/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Tue Sep 18 11:33:57 CST 2012
 */

#include <cavan.h>
#include <cavan/swan_vk.h>
#include <cavan/event.h>
#include <cavan/input.h>
#include <cavan/file.h>
#include <cavan/network.h>
#include <cavan/adb.h>
#include <cavan/swan_upgrade.h>

static int client_active;
static const struct swan_vk_descriptor swan_vk_table[] =
{
	{"search", SWAN_VKEY_SEARCH},
	{"back", SWAN_VKEY_BACK},
	{"menu", SWAN_VKEY_MENU},
	{"home", SWAN_VKEY_HOME},
	{"vup", SWAN_VKEY_VOLUME_UP},
	{"vdown", SWAN_VKEY_VOLUME_DOWN},
	{"call", SWAN_VKEY_CALL},
	{"endcall", SWAN_VKEY_ENDCALL},
	{"power", SWAN_VKEY_POWER},
	{"up", SWAN_VKEY_UP},
	{"down", SWAN_VKEY_DOWN},
	{"left", SWAN_VKEY_LEFT},
	{"right", SWAN_VKEY_RIGHT},
	{"camera", SWAN_VKEY_CAMERA},
	{"center", 232},
	{"space", 57},
	{"tab", 15},
	{"enter", 28},
	{"grave", 399},
	{"pound", 228},
	{"star", 227},
	{"backslash", 43},
	{"del", 14}
};

static void swan_vk_client_stop_handle(int signum)
{
	pr_bold_pos();
	client_active = 0;
}

static void swan_vk_serial_send_stop_signal(int fd_tty, u32 value, int count)
{
	while (count-- && ffile_write(fd_tty, (void *)&value, sizeof(value)) > 0)
	{
		msleep(100);
	}
}

int swan_vk_serial_client(const char *tty_path)
{
	int count;
	int fd_tty;
	struct pollfd event_fds[16];
	struct input_event events[32];
	ssize_t rdlen, wrlen;

	pr_bold_pos();

	fd_tty = -1;

	if (tty_path)
	{
		fd_tty = open(tty_path, O_WRONLY | O_SYNC | O_NOCTTY);
	}

	if (fd_tty < 0)
	{
		tty_path = (char *)events;
		fd_tty = file_find_and_open("/dev/ttyUSB", (char *)tty_path, 0, 10, O_WRONLY | O_SYNC);
	}

	if (fd_tty < 0)
	{
		error_msg("open tty device failed");
		return fd_tty;
	}

	println("ttypath = %s", tty_path);

	count = open_event_devices(event_fds, ARRAY_SIZE(event_fds), O_RDONLY);
	if (count <= 0)
	{
		error_msg("no input device found");
		goto out_close_tty;
	}

	signal(SIGINT, swan_vk_client_stop_handle);
	client_active = 1;

	while (client_active)
	{
		rdlen = poll_event_devices(event_fds, count, events, sizeof(events));
		if (rdlen < 0)
		{
			error_msg("poll_event_devices");
			break;
		}

		wrlen = ffile_write(fd_tty, events, rdlen);
		if (wrlen < 0)
		{
			error_msg("write");
			break;
		}
	}

	swan_vk_serial_send_stop_signal(fd_tty, SWAN_VK_STOP_VALUE, 2);
	close_event_devices(event_fds, count);
out_close_tty:
	close(fd_tty);

	return -1;
}

// ================================================================================

static void swan_vk_serial_server_stop_handle(int signum)
{
	restore_tty_attr(-1);
	exit(0);
}

int swan_vk_serial_server(const char *tty_path, const char *data_path)
{
	int ret;
	int fd_tty, fd_data;
	struct input_event events[32];
	ssize_t rdlen, wrlen;

	pr_bold_pos();

	fd_tty = open(tty_path, O_RDONLY | O_NOCTTY);
	if (fd_tty < 0)
	{
		print_error("open tty device \"%s\" failed", tty_path);
		return fd_tty;
	}

	ret = set_tty_mode(fd_tty, 3);
	if (ret < 0)
	{
		print_error("set_tty_mode");
		goto out_close_tty;
	}

	signal(SIGKILL, swan_vk_serial_server_stop_handle);

	fd_data = open(data_path, O_WRONLY);
	if (fd_data < 0)
	{
		print_error("open data device \"%s\" failed", data_path);
		goto out_close_tty;
	}

	while (1)
	{
		rdlen = read(fd_tty, events, sizeof(events));
		if (rdlen < 0)
		{
			print_error("read");
			break;
		}

		if (*(u32 *)events == SWAN_VK_STOP_VALUE)
		{
			break;
		}

		wrlen = ffile_write(fd_data, events, rdlen);
		if (wrlen < 0)
		{
			print_error("write");
			break;
		}
	}

	restore_tty_attr(fd_tty);

	close(fd_data);
out_close_tty:
	close(fd_tty);

	pr_bold_info("%s exit", __FUNCTION__);

	return -1;
}

// ================================================================================

static void swan_vk_map_keys(struct input_event *ep, size_t count)
{
	struct input_event *ep_end;

	for (ep_end = ep + count; ep < ep_end; ep++)
	{
		if (ep->type != EV_KEY)
		{
			continue;
		}

		switch (ep->code)
		{
		case KEY_F9:
		case KEY_ESC:
			ep->code = SWAN_VKEY_BACK;
			break;

		case KEY_F10:
		case KEY_HOME:
			ep->code = SWAN_VKEY_HOME;
			break;

		case KEY_F11:
			ep->code = SWAN_VKEY_MENU;
			break;

		case KEY_F12:
			ep->code = SWAN_VKEY_SEARCH;
			break;

		case KEY_F5:
		case KEY_F6:
			ep->code = SWAN_VKEY_VOLUME_DOWN;
			break;

		case KEY_F7:
		case KEY_F8:
			ep->code = SWAN_VKEY_VOLUME_UP;
			break;

		case KEY_F1:
			ep->code = SWAN_VKEY_POWER;
			break;

		case KEY_F2:
			ep->code = SWAN_VKEY_CAMERA;
			break;

		case KEY_F3:
			ep->code = SWAN_VKEY_CALL;
			break;

		case KEY_F4:
			ep->code = SWAN_VKEY_ENDCALL;
			break;
		}
	}
}

static int swan_vk_release_all_key(int sockfd)
{
	u16 code;
	ssize_t wrlen;
	struct input_event events[2] =
	{
		{
			.type = EV_KEY,
			.value = 0
		},
		{
			.type = EV_SYN,
			.code = SYN_REPORT,
			.value = 0
		}
	};

	for (code = 0; code < KEY_MAX; code++)
	{
		events[0].code = code;
		wrlen = inet_send(sockfd, events, sizeof(events));
		if (wrlen < 0)
		{
			return wrlen;
		}
	}

	return 0;
}

int swan_vk_adb_client(const char *ip, u16 port)
{
	int count;
	int sockfd;
	struct pollfd event_fds[16];
	struct input_event events[32];
	ssize_t rdlen, wrlen;

	pr_bold_pos();

	sockfd = adb_create_tcp_link(ip, 0, port);
	if (sockfd < 0)
	{
		pr_red_info("adb_create_tcp_link");
		return sockfd;
	}

	count = open_event_devices(event_fds, ARRAY_SIZE(event_fds), O_RDONLY);
	if (count <= 0)
	{
		error_msg("no input device found");
		goto out_close_sockfd;
	}

	signal(SIGINT, swan_vk_client_stop_handle);
	client_active = 1;

	while (client_active)
	{
		rdlen = poll_event_devices(event_fds, count, events, sizeof(events));
		if (rdlen < 0)
		{
			error_msg("poll_event_devices");
			goto out_close_devices;
		}

		swan_vk_map_keys(events, rdlen / sizeof(events[0]));

		wrlen = inet_send(sockfd, events, rdlen);
		if (wrlen < 0)
		{
			error_msg("inet_send");
			goto out_close_devices;
		}
	}

out_close_devices:
	close_event_devices(event_fds, count);
out_close_sockfd:
	swan_vk_release_all_key(sockfd);
	inet_close_tcp_socket(sockfd);

	return -1;
}

static int swan_vk_server_handler(int index, cavan_shared_data_t data)
{
	struct swan_vk_service_descriptor *desc = data.type_void;
	int sockfd = desc->sockfd;
	int datafd = desc->datafd;
	pthread_mutex_t *mutex_lock = &desc->mutex_lock;
	struct sockaddr_in addr;
	socklen_t addrlen;
	int sockfd_client;
	struct input_event events[32];
	ssize_t rdlen, wrlen;

	sockfd_client = inet_accept(sockfd, &addr, &addrlen);
	if (sockfd_client < 0)
	{
		pr_red_info("inet_accept");
		return -1;
	}

	pr_bold_info("IP = %s, port = %d", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

	while (1)
	{
		rdlen = recv(sockfd_client, events, sizeof(events), 0);
		if (rdlen < sizeof(events[0]))
		{
			pr_red_info("inet_recv");
			return rdlen;
		}

		pthread_mutex_lock(mutex_lock);
		wrlen = ffile_write(datafd, events, rdlen);
		pthread_mutex_unlock(mutex_lock);
		if (wrlen < 0)
		{
			pr_red_info("ffile_write");
			return wrlen;
		}
	}

	return 0;
}

int swan_vk_adb_server(struct cavan_service_description *desc, const char *data_path, u16 port)
{
	int ret;
	int sockfd, datafd;
	struct swan_vk_service_descriptor vk_desc;

	ret = pthread_mutex_init(&vk_desc.mutex_lock, NULL);
	if (ret < 0)
	{
		pr_red_info("pthread_mutex_init");
		return ret;
	}

	datafd = file_open_wo(data_path);
	if (datafd < 0)
	{
		pr_red_info("open file `%s' failed", data_path);
		return datafd;
	}

	sockfd = inet_create_tcp_service(port);
	if (sockfd < 0)
	{
		pr_red_info("inet_create_tcp_service");
		close(datafd);
		return sockfd;
	}

	vk_desc.datafd = datafd;
	vk_desc.sockfd = sockfd;
	desc->handler = swan_vk_server_handler;
	desc->data.type_void = (void *)&vk_desc;
	ret = cavan_service_run(desc);
	cavan_service_stop(desc);
	inet_close_tcp_socket(sockfd);
	close(datafd);

	return ret;
}

// ================================================================================

static void swan_vk_show_key_table(const struct swan_vk_descriptor *descs, size_t size)
{
	const struct swan_vk_descriptor *desc_end;

	for (desc_end = descs + size; descs < desc_end; descs++)
	{
		println("%s = %d", descs->name, descs->code);
	}
}

static const struct swan_vk_descriptor *swan_vk_match_key(const char *keyname, const struct swan_vk_descriptor *descs, size_t size)
{
	const struct swan_vk_descriptor *desc_end;

	for (desc_end = descs + size; descs < desc_end && text_lhcmp(keyname, descs->name); descs++);

	if (descs < desc_end)
	{
		return descs;
	}

	return NULL;
}

int swan_vk_commadline(const char *data_path)
{
	int fd_data;
	char name[1024], *name_p;
	const struct swan_vk_descriptor *p, *old_p;
	ssize_t wrlen;

	fd_data = open(data_path, O_WRONLY);
	if (fd_data < 0)
	{
		print_error("open file \"%s\" failed", data_path);
		return fd_data;
	}

	old_p = swan_vk_table;

	while (1)
	{
		print("\033[01;32m" SWAN_VK_PROMPT "\033[0m# ");

		for (name_p = name; is_empty_character((*name_p = getchar())) == 0; name_p++);

		if (name_p == name)
		{
			goto label_repo_key;
		}

		*name_p = 0;

		p = swan_vk_match_key(name, swan_vk_table, ARRAY_SIZE(swan_vk_table));
		if (p == NULL)
		{
			swan_vk_show_key_table(swan_vk_table, ARRAY_SIZE(swan_vk_table));
			continue;
		}

		old_p = p;

label_repo_key:
		pr_bold_info("repo key = %s = %d", old_p->name, old_p->code);

		wrlen = write(fd_data, (void *)&old_p->code, sizeof(old_p->code));
		if (wrlen < 0)
		{
			print_error("write");
			break;
		}
	}

	close(fd_data);

	return -1;
}
