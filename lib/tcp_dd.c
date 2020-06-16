/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Sat Jan 14 12:59:51 CST 2012
 */

#include <cavan.h>
#include <cavan/fb.h>
#include <cavan/file.h>
#include <cavan/event.h>
#include <cavan/input.h>
#include <cavan/tcp_dd.h>
#include <cavan/service.h>
#include <cavan/device.h>
#include <cavan/command.h>
#include <cavan/swan_vk.h>
#include <cavan/android.h>

#define TCP_DD_AUTO_CLOSE_UINPUT	1

static const u16 tcp_dd_remote_ctrl_keys[] = {
	KEY_BACK, KEY_MENU, KEY_HOMEPAGE, KEY_SEARCH,
	KEY_POWER, KEY_DELETE, KEY_ENTER,
	KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,
	KEY_VOLUMEUP, KEY_VOLUMEDOWN, KEY_MUTE,
	KEY_BRIGHTNESSUP, KEY_BRIGHTNESSDOWN,
	KEY_PLAY, KEY_PLAYPAUSE, KEY_PREVIOUSSONG, KEY_NEXTSONG
};

static u16 tcp_dd_keypad_char_key_map[] = {
	[27] = KEY_ESC,
	['1'] = KEY_1,
	['2'] = KEY_2,
	['3'] = KEY_3,
	['4'] = KEY_4,
	['5'] = KEY_5,
	['6'] = KEY_6,
	['7'] = KEY_7,
	['8'] = KEY_8,
	['9'] = KEY_9,
	['0'] = KEY_0,
	['-'] = KEY_MINUS,
	['='] = KEY_EQUAL,
	[127] = KEY_BACKSPACE,
	['\t'] = KEY_TAB,
	['q'] = KEY_Q,
	['w'] = KEY_W,
	['e'] = KEY_E,
	['r'] = KEY_R,
	['t'] = KEY_T,
	['y'] = KEY_Y,
	['u'] = KEY_U,
	['i'] = KEY_I,
	['o'] = KEY_O,
	['p'] = KEY_P,
	['['] = KEY_LEFTBRACE,
	[']'] = KEY_RIGHTBRACE,
	[10] = KEY_ENTER,
	// [] = KEY_LEFTCTRL,
	['a'] = KEY_A,
	['s'] = KEY_S,
	['d'] = KEY_D,
	['f'] = KEY_F,
	['g'] = KEY_G,
	['h'] = KEY_H,
	['j'] = KEY_J,
	['k'] = KEY_K,
	['l'] = KEY_L,
	[';'] = KEY_SEMICOLON,
	['\''] = KEY_APOSTROPHE,
	['`'] = KEY_GRAVE,
	// [] = KEY_LEFTSHIFT,
	['\\'] = KEY_BACKSLASH,
	['z'] = KEY_Z,
	['x'] = KEY_X,
	['c'] = KEY_C,
	['v'] = KEY_V,
	['b'] = KEY_B,
	['n'] = KEY_N,
	['m'] = KEY_M,
	[','] = KEY_COMMA,
	['.'] = KEY_DOT,
	['/'] = KEY_SLASH,
	// [] = KEY_RIGHTSHIFT,
	// [] = KEY_KPASTERISK,
	// [] = KEY_LEFTALT,
	[' '] = KEY_SPACE,
	// [] = KEY_CAPSLOCK,

	// =========================================================================

	['!'] = KEY_1 | TCP_KEYPADF_SHIFT,
	['@'] = KEY_2 | TCP_KEYPADF_SHIFT,
	['#'] = KEY_3 | TCP_KEYPADF_SHIFT,
	['$'] = KEY_4 | TCP_KEYPADF_SHIFT,
	['%'] = KEY_5 | TCP_KEYPADF_SHIFT,
	['^'] = KEY_6 | TCP_KEYPADF_SHIFT,
	['&'] = KEY_7 | TCP_KEYPADF_SHIFT,
	['*'] = KEY_8 | TCP_KEYPADF_SHIFT,
	['('] = KEY_9 | TCP_KEYPADF_SHIFT,
	[')'] = KEY_0 | TCP_KEYPADF_SHIFT,
	['_'] = KEY_MINUS | TCP_KEYPADF_SHIFT,
	['+'] = KEY_EQUAL | TCP_KEYPADF_SHIFT,
	// [] = KEY_BACKSPACE,
	// ['\t'] = KEY_TAB,
	['Q'] = KEY_Q | TCP_KEYPADF_SHIFT,
	['W'] = KEY_W | TCP_KEYPADF_SHIFT,
	['E'] = KEY_E | TCP_KEYPADF_SHIFT,
	['R'] = KEY_R | TCP_KEYPADF_SHIFT,
	['T'] = KEY_T | TCP_KEYPADF_SHIFT,
	['Y'] = KEY_Y | TCP_KEYPADF_SHIFT,
	['U'] = KEY_U | TCP_KEYPADF_SHIFT,
	['I'] = KEY_I | TCP_KEYPADF_SHIFT,
	['O'] = KEY_O | TCP_KEYPADF_SHIFT,
	['P'] = KEY_P | TCP_KEYPADF_SHIFT,
	['{'] = KEY_LEFTBRACE | TCP_KEYPADF_SHIFT,
	['}'] = KEY_RIGHTBRACE | TCP_KEYPADF_SHIFT,
	// [] = KEY_ENTER,
	// [] = KEY_LEFTCTRL,
	['A'] = KEY_A | TCP_KEYPADF_SHIFT,
	['S'] = KEY_S | TCP_KEYPADF_SHIFT,
	['D'] = KEY_D | TCP_KEYPADF_SHIFT,
	['F'] = KEY_F | TCP_KEYPADF_SHIFT,
	['G'] = KEY_G | TCP_KEYPADF_SHIFT,
	['H'] = KEY_H | TCP_KEYPADF_SHIFT,
	['J'] = KEY_J | TCP_KEYPADF_SHIFT,
	['K'] = KEY_K | TCP_KEYPADF_SHIFT,
	['L'] = KEY_L | TCP_KEYPADF_SHIFT,
	[':'] = KEY_SEMICOLON | TCP_KEYPADF_SHIFT,
	['"'] = KEY_APOSTROPHE | TCP_KEYPADF_SHIFT,
	['~'] = KEY_GRAVE | TCP_KEYPADF_SHIFT,
	// [] = KEY_LEFTSHIFT,
	['|'] = KEY_BACKSLASH | TCP_KEYPADF_SHIFT,
	['Z'] = KEY_Z | TCP_KEYPADF_SHIFT,
	['X'] = KEY_X | TCP_KEYPADF_SHIFT,
	['C'] = KEY_C | TCP_KEYPADF_SHIFT,
	['V'] = KEY_V | TCP_KEYPADF_SHIFT,
	['B'] = KEY_B | TCP_KEYPADF_SHIFT,
	['N'] = KEY_N | TCP_KEYPADF_SHIFT,
	['M'] = KEY_M | TCP_KEYPADF_SHIFT,
	['<'] = KEY_COMMA | TCP_KEYPADF_SHIFT,
	['>'] = KEY_DOT | TCP_KEYPADF_SHIFT,
	['?'] = KEY_SLASH | TCP_KEYPADF_SHIFT,
	// [] = KEY_RIGHTSHIFT,
	// [] = KEY_KPASTERISK,
	// [] = KEY_LEFTALT,
	// [] = KEY_SPACE,
	// [] = KEY_CAPSLOCK,

	// =========================================================================

	[1] = KEY_A | TCP_KEYPADF_CTRL,
	[2] = KEY_B | TCP_KEYPADF_CTRL,
	[3] = KEY_C | TCP_KEYPADF_CTRL,
	[4] = KEY_D | TCP_KEYPADF_CTRL,
	[5] = KEY_E | TCP_KEYPADF_CTRL,
	[6] = KEY_F | TCP_KEYPADF_CTRL,
	[7] = KEY_G | TCP_KEYPADF_CTRL,
	[8] = KEY_H | TCP_KEYPADF_CTRL,
	// [9] = KEY_I | TCP_KEYPADF_CTRL,
	// [10] = KEY_J | TCP_KEYPADF_CTRL,
	[11] = KEY_K | TCP_KEYPADF_CTRL,
	[12] = KEY_L | TCP_KEYPADF_CTRL,
	[13] = KEY_M | TCP_KEYPADF_CTRL,
	[14] = KEY_N | TCP_KEYPADF_CTRL,
	[15] = KEY_O | TCP_KEYPADF_CTRL,
	[16] = KEY_P | TCP_KEYPADF_CTRL,
	[17] = KEY_Q | TCP_KEYPADF_CTRL,
	[18] = KEY_R | TCP_KEYPADF_CTRL,
	[19] = KEY_S | TCP_KEYPADF_CTRL,
	[20] = KEY_T | TCP_KEYPADF_CTRL,
	[21] = KEY_U | TCP_KEYPADF_CTRL,
	[22] = KEY_V | TCP_KEYPADF_CTRL,
	[23] = KEY_W | TCP_KEYPADF_CTRL,
	[24] = KEY_X | TCP_KEYPADF_CTRL,
	[25] = KEY_Y | TCP_KEYPADF_CTRL,
	[26] = KEY_Z | TCP_KEYPADF_CTRL,
};

static void tcp_dd_show_response(struct tcp_dd_response_package *res)
{
	if (res->message[0] == 0) {
		return;
	}

	if ((int) res->code < 0) {
		if (res->number) {
			pd_red_info("%s [%s]", res->message, strerror(res->number));
		} else {
			pd_red_info("%s", res->message);
		}
	} else {
		// pd_green_info("%s", res->message);
	}
}

void tcp_dd_set_package_type(struct tcp_dd_package *pkg, u16 type)
{
	pkg->type = type;
	pkg->type_inverse = ~type;
}

bool tcp_dd_package_is_valid(const struct tcp_dd_package *pkg)
{
	if (pkg->version != TCP_DD_VERSION) {
		pr_red_info("invalid protocol version 0x%08x, expect 0x%08x", pkg->version, TCP_DD_VERSION);
		return false;
	}

	return (pkg->type ^ pkg->type_inverse) == 0xFFFF;
}

bool tcp_dd_package_is_invalid(const struct tcp_dd_package *pkg)
{
	if (pkg->version != TCP_DD_VERSION) {
		pr_red_info("invalid protocol version 0x%08x, expect 0x%08x", pkg->version, TCP_DD_VERSION);
		return true;
	}

	return (pkg->type ^ pkg->type_inverse) != 0xFFFF;
}

ssize_t tcp_dd_package_recv(struct network_client *client, struct tcp_dd_package *pkg)
{
	ssize_t rdlen;

	rdlen = client->recv(client, pkg, sizeof(struct tcp_dd_package));
	if (rdlen < (ssize_t) TCP_DD_PKG_BODY_OFFSET) {
		pr_red_info("Invalid package length %ld", (long) rdlen);

		if (rdlen < 0) {
			return rdlen;
		}

		return -EINVAL;
	}

	if (tcp_dd_package_is_invalid(pkg)) {
		pr_red_info("Invalid package type = 0x%04x, type_inverse = 0x%04x", pkg->type, pkg->type_inverse);
		return -EINVAL;
	}

	return rdlen;
}

ssize_t tcp_dd_package_send(struct network_client *client, struct tcp_dd_package *pkg, u16 type, size_t length, u32 flags)
{
	ssize_t wrlen;

	pkg->version = TCP_DD_VERSION;
	pkg->flags = flags;

	tcp_dd_set_package_type(pkg, type);

	wrlen = client->send(client, pkg, length);
	if (wrlen < (ssize_t) length) {
		if (wrlen < 0) {
			return wrlen;
		}

		return -EFAULT;
	}

	return 0;
}

int tcp_dd_send_request(struct network_client *client, struct tcp_dd_package *pkg, struct tcp_dd_package *response, u16 type, size_t length, u32 flags)
{
	int ret;

	ret = tcp_dd_package_send(client, pkg, type, length, flags);
	if (ret < 0) {
		pr_red_info("tcp_dd_package_send %d", ret);
		return ret;
	}

	if (response) {
		return tcp_dd_package_recv(client, response);
	}

	return 0;
}

int tcp_dd_send_request2(struct network_client *client, struct tcp_dd_package *pkg, u16 type, size_t length, u32 flags)
{
	int ret;
	struct tcp_dd_package response;

	ret = tcp_dd_send_request(client, pkg, &response, type, length, flags);
	if (ret < 0) {
		pr_red_info("tcp_dd_send_request %d", ret);
		return ret;
	}

	if (response.type != TCP_DD_RESPONSE) {
		pr_red_info("Invalid package type %d", response.type);
		return -EINVAL;
	}

	tcp_dd_show_response(&response.res_pkg);

	return (int) response.res_pkg.code;
}

int tcp_dd_send_request3(struct network_url *url, struct tcp_dd_package *pkg, struct tcp_dd_package *response, u16 type, size_t length, u32 flags)
{
	int ret;
	struct network_client client;

	ret = network_client_open(&client, url, CAVAN_NET_FLAG_TALK | CAVAN_NET_FLAG_SYNC | CAVAN_NET_FLAG_WAIT | CAVAN_NET_FLAG_NODELAY);
	if (ret < 0) {
		pr_red_info("network_client_open2");
		return ret;
	}

	return tcp_dd_send_request(&client, pkg, response, type, length, flags);
}

int tcp_dd_send_request4(struct network_url *url, struct tcp_dd_package *pkg, u16 type, size_t length, u32 flags)
{
	int ret;
	struct network_client client;

	ret = network_client_open(&client, url, CAVAN_NET_FLAG_TALK | CAVAN_NET_FLAG_SYNC | CAVAN_NET_FLAG_WAIT | CAVAN_NET_FLAG_NODELAY);
	if (ret < 0) {
		pr_red_info("network_client_open2");
		return ret;
	}

	return tcp_dd_send_request2(&client, pkg, type, length, flags);
}

int tcp_dd_send_empty_request(struct network_client *client, u16 type, struct tcp_dd_package *response, u32 flags)
{
	struct tcp_dd_package pkg;

	return tcp_dd_send_request(client, &pkg, response, type, TCP_DD_PKG_HEADER_LEN, flags);
}

static char *tcp_dd_find_platform_by_name_path(char *pathname, char *filename, size_t size)
{
	DIR *dp;
	struct dirent *entry;

	if (filename == NULL) {
		filename = text_ncopy(pathname, "/dev/block/platform/", size);
	}

	dp = opendir(pathname);
	if (dp == NULL) {
		return NULL;
	}

	while ((entry = readdir(dp))) {
		char *filename_next;

		if (entry->d_type != DT_DIR) {
			continue;
		}

		if (cavan_path_is_dot_name(entry->d_name)) {
			continue;
		}

		filename_next = text_copy(filename, entry->d_name);
		*filename_next++ = '/';
		*filename_next = 0;

		if (strcmp(entry->d_name, "by-name") == 0) {
			filename = filename_next;
			goto out_closedir;
		}

		filename_next = tcp_dd_find_platform_by_name_path(pathname, filename_next, size);
		if (filename_next) {
			filename = filename_next;
			goto out_closedir;
		}
	}

	filename = NULL;

out_closedir:
	closedir(dp);
	return filename;
}

int tcp_dd_get_partition_filename(const char *name, char *buff, size_t size)
{
	char *buff_end;
	const char *last;

	if (name[0] != '@') {
		return 0;
	}

	for (last = ++name; *last; last++);

	if (last - name < 1) {
		return 0;
	}

	if (*--last != '@') {
		return 0;
	}

	if (buff == NULL) {
		return -EFAULT;
	}

	for (buff_end = buff + size - 1; name < last && buff < buff_end; name++, buff++) {
		char c = *name;

		if (c >= 'A' && c <= 'Z') {
			*buff = c + ('a' - 'A');
		} else {
			*buff = c;
		}
	}

	*buff = 0;

	return 1;
}

const char *tcp_dd_get_partition_pathname(struct cavan_tcp_dd_service *service, const char *name)
{
	int ret;

	ret = tcp_dd_get_partition_filename(name, service->filename, sizeof(service->pathname));
	if (ret < 0) {
		return NULL;
	}

	if (ret == 0) {
		return name;
	}

	if (service->part_table && service->filename == service->pathname) {
		ret = cavan_block_get_part_pathname(service->part_table, service->filename, service->pathname, sizeof(service->pathname));
		if (ret < 0) {
			return NULL;
		}
	}

	return service->pathname;
}

static int __printf_format_34__ tcp_dd_send_response(struct network_client *client, int code, const char *fmt, ...)
{
	struct tcp_dd_package pkg;
	size_t length = MOFS(struct tcp_dd_package, res_pkg.message);

	pkg.res_pkg.code = code;
	pkg.res_pkg.number = errno;

	if (fmt == NULL) {
		pkg.res_pkg.message[0] = 0;
		length += 1;
	} else {
		va_list ap;

		va_start(ap, fmt);
		length += vsprintf(pkg.res_pkg.message, fmt, ap) + 1;
		va_end(ap);

		if (code < 0) {
			pr_error_info("%s", pkg.res_pkg.message);
		}
	}

	return tcp_dd_package_send(client, &pkg, TCP_DD_RESPONSE, length, 0);
}

static int tcp_dd_recv_response(struct network_client *client)
{
	ssize_t rdlen;
	struct tcp_dd_package pkg;

	rdlen = tcp_dd_package_recv(client, &pkg);
	if (rdlen < 0) {
		pr_red_info("tcp_dd_package_recv");
		return -EFAULT;
	}

	if (pkg.type != TCP_DD_RESPONSE) {
		pr_red_info("pkg.type = %d", pkg.type);
		return -EINVAL;
	}

	tcp_dd_show_response(&pkg.res_pkg);

	return pkg.res_pkg.code;
}

static int tcp_dd_send_file_request(struct network_client *client, struct tcp_dd_package *pkg, u16 type, const char *filename, u32 flags)
{
	int ret;
	size_t length;

	length = ADDR_OFFSET(text_copy(pkg->file_req.filename, filename), &pkg) + 1;

	ret = tcp_dd_send_request(client, pkg, pkg, type, length, flags);
	if (ret < 0) {
		pr_red_info("tcp_dd_send_request: %d", ret);
		return ret;
	}

	switch (pkg->type) {
	case TCP_DD_RESPONSE:
		tcp_dd_show_response(&pkg->res_pkg);
		return pkg->res_pkg.code;

	case TCP_DD_BREAKPOINT:
		if (type != TCP_DD_WRITE) {
			return -EINVAL;
		}

		ret = tcp_dd_send_response(client, 0, "[Client] Start send file");
		if (ret < 0) {
			pr_red_info("tcp_dd_send_response: %d", ret);
			return ret;
		}

		return 0;

	case TCP_DD_WRITE:
	case TCP_DD_FILE_STAT:
		if (type != TCP_DD_READ) {
			return -EINVAL;
		}

		return 0;

	default:
		return -EINVAL;
	}
}

static int tcp_dd_send_exec_request(struct network_client *client, int ttyfd, const char *command)
{
	char *p;
	size_t length;
	struct tcp_dd_package pkg;

	tty_get_win_size3(ttyfd, &pkg.exec_req.lines, &pkg.exec_req.columns);

	// pd_info("terminal size = %d x %d", pkg.exec_req.lines, pkg.exec_req.columns);

	if (command) {
		p = text_copy(pkg.exec_req.command, command);
	} else {
		p = pkg.exec_req.command;
		*p = 0;
	}

	length = ADDR_OFFSET(p, &pkg) + 1;

	return tcp_dd_send_request2(client, &pkg, TCP_DD_EXEC, length, 0);
}

static int tcp_dd_send_keypad_request(struct network_client *client)
{
	struct tcp_dd_package pkg;

	return tcp_dd_send_request2(client, &pkg, TCP_KEYPAD_EVENT, TCP_DD_PKG_BODY_OFFSET, 0);
}

static int tcp_dd_send_alarm_add_request(struct network_client *client, time_t time, time_t repeat, const char *command)
{
	size_t length;
	struct tcp_dd_package pkg;

	pkg.alarm_add.time = time;
	pkg.alarm_add.repeat = repeat;
	length = ADDR_OFFSET(text_copy(pkg.alarm_add.command, command), &pkg) + 1;

	return tcp_dd_send_request2(client, &pkg, TCP_ALARM_ADD, length, 0);
}

static int tcp_dd_send_alarm_query_request(struct network_client *client, u16 type, int index)
{
	size_t length;
	struct tcp_dd_package pkg;

	pkg.alarm_query.index = index;

	length = sizeof(pkg.alarm_query) + MOFS(struct tcp_dd_package, alarm_query);

	return tcp_dd_send_request2(client, &pkg, type, length, 0);
}

static int tcp_dd_handle_read_request(struct cavan_tcp_dd_service *service, struct network_client *client, struct tcp_dd_package *pkg)
{
	int fd;
	int ret;
	off_t size;
	struct stat st;
	const char *pathname;
	struct tcp_dd_file_request *req = &pkg->file_req;

	pathname = tcp_dd_get_partition_pathname(service, req->filename);
	if (pathname == NULL) {
		ret = -ENOENT;
		tcp_dd_send_response(client, ret, "[Server] partition `%s' not found", req->filename);
		return ret;
	}

	ret = file_stat(pathname, &st);
	if (ret < 0) {
		tcp_dd_send_response(client, ret, "[Server] Get file `%s' stat failed", pathname);
		return ret;
	}

	if (S_ISDIR(st.st_mode)) {
		fd = cavan_readdir_to_file_temp(pathname, &size);
		if (fd < 0) {
			tcp_dd_send_response(client, fd, "[Server] Read dir `%s' failed", pathname);
			return fd;
		}

		req->offset = 0;
	} else {
		fd = open(pathname, O_RDONLY);
		if (fd < 0) {
			tcp_dd_send_response(client, fd, "[Server] Open file `%s' failed", pathname);
			return fd;
		}

		if (req->size == 0) {
			size = st.st_size;
		} else {
			size = req->size;
		}

		if (size > 0 && size < (off_t) req->offset) {
			ret = -EINVAL;
			tcp_dd_send_response(client, ret, "[Server] No data to be sent");
			goto out_close_fd;
		}

		if (lseek(fd, req->offset, SEEK_SET) != (off_t) req->offset) {
			ret = -EFAULT;
			tcp_dd_send_response(client, ret, "[Server] Seek file `%s' failed", pathname);
			goto out_close_fd;
		}

		if (size > 0) {
			size -= req->offset;
		}
	}

	req->size = size;
	req->mode = st.st_mode;

	ret = tcp_dd_send_request2(client, pkg, TCP_DD_FILE_STAT, ADDR_SUB2(req->filename, pkg), 0);
	if (ret < 0) {
		pd_red_info("tcp_dd_send_request2");
		goto out_close_fd;
	}

	println("filename = %s", pathname);
	println("offset = %s", size2text(req->offset));
	println("size = %s", size2text(size));

	ret = network_client_send_file(client, fd, 0, size);
	if (ret < 0) {
		pd_err_info("network_client_send_file: %d", ret);
	}

out_close_fd:
	close(fd);

	return ret;
}

static int tcp_dd_handle_write_request(struct cavan_tcp_dd_service *service, struct network_client *client, struct tcp_dd_package *pkg)
{
	int fd;
	int ret;
	mode_t mode;
	bool isfile = false;
	const char *pathname;
	struct tcp_dd_file_request *req = &pkg->file_req;

	if (S_ISDIR(req->mode)) {
		ret = cavan_mkdir_main2(req->filename, req->mode);
		return tcp_dd_send_response(client, ret, NULL);
	}

	pathname = tcp_dd_get_partition_pathname(service, req->filename);
	if (pathname == NULL) {
		ret = -ENOENT;
		tcp_dd_send_response(client, ret, "[Server] `%s' is not a partition", req->filename);
		return ret;
	}

	mode = file_get_mode(pathname);
	if (mode == 0) {
		isfile = true;
	} else if (pathname == req->filename) {
		switch (mode & S_IFMT) {
		case S_IFREG:
			isfile = true;
			if ((pkg->flags & TCP_DDF_BREAKPOINT_RESUME) == 0 || req->size < MB(200)) {
				pd_info("remove regular file %s", pathname);
				unlink(pathname);
			}
			break;

		case S_IFBLK:
			pd_info("umount block device %s", pathname);
			umount_device(pathname, MNT_DETACH);
			break;
		}
	} else {
		if (mode == 0 || (mode & S_IFMT) != S_IFBLK) {
			ret = -ENOTBLK;

			tcp_dd_send_response(client, ret, "[Server] `%s' is not a block device", pathname);
			return ret;
		}

		if (strcmp(req->filename, "@SYSTEM@") == 0) {
			android_stop_all();
		}

		pd_info("umount block device %s", pathname);
		umount_device(pathname, MNT_DETACH);
	}

	if (isfile && (pkg->flags & TCP_DDF_BREAKPOINT_RESUME)) {
		size64_t skip;

		fd = open(pathname, O_CREAT | O_WRONLY | O_APPEND | O_BINARY, req->mode);
		if (fd < 0) {
			tcp_dd_send_response(client, fd, "[Server] Open file `%s' failed", pathname);
			return fd;
		}

		skip = lseek(fd, 0, SEEK_END);
		if (skip > req->size) {
			ret = -EINVAL;
			tcp_dd_send_response(client, ret, "[Server] Invalid skip");
			goto out_close_fd;
		}

		req->size -= skip;
		req->offset = skip;
	} else {
		fd = open(pathname, O_CREAT | O_WRONLY | O_BINARY, req->mode);
		if (fd < 0) {
			tcp_dd_send_response(client, fd, "[Server] Open file `%s' failed", pathname);
			return fd;
		}

		if (lseek(fd, req->offset, SEEK_SET) != (off_t) req->offset) {
			ret = -EFAULT;
			tcp_dd_send_response(client, ret, "[Server] Seek file failed");
			goto out_close_fd;
		}

		req->offset = 0;
	}

	println("filename = %s", pathname);
	println("offset = %s", size2text(req->offset));
	println("size = %s", size2text(req->size));

	ret = tcp_dd_send_request2(client, pkg, TCP_DD_BREAKPOINT, ADDR_SUB2(req->filename, pkg), 0);
	if (ret < 0) {
		pr_red_info("tcp_dd_send_request %d", ret);
		goto out_close_fd;
	}

	if (S_ISBLK(mode)) {
	    bdev_set_read_only(fd, 0);
	}

	ret = network_client_recv_file(client, fd, 0, req->size);
	if (ret < 0) {
		pd_err_info("network_client_recv_file: %d", ret);
	}

out_close_fd:
	close(fd);

	return ret;
}

static int tcp_dd_handle_exec_request(struct network_client *client, struct tcp_dd_exec_request *req)
{
	int ret;
	int lines, columns;

	pd_info("command = `%s'", req->command);

	ret = tcp_dd_send_response(client, 0, "[Server] start execute command");
	if (ret < 0) {
		pd_red_info("tcp_dd_send_response");
		return ret;
	}

	if (client->type == NETWORK_PROTOCOL_TCP || client->type == NETWORK_PROTOCOL_UDP) {
		struct sockaddr_in addr;

		if (inet_getpeername(client->sockfd, &addr) == 0) {
			setenv(CAVAN_IP_ENV_NAME, inet_ntoa(addr.sin_addr), 1);
		}
	}

	lines = req->lines;
	if (lines == 0xFFFF) {
		lines = -1;
	}

	columns = req->columns;
	if (columns == 0xFFFF) {
		columns = -1;
	}

	return network_client_exec_main(client, req->command, lines, columns);
}

static void tcp_dd_alarm_handler(struct cavan_alarm_node *alarm, struct cavan_alarm_thread *thread, void *data)
{
	pid_t pid;

	pid = fork();
	if (pid == 0) {
		cavan_exec_command(data, 0, NULL);
	}
}

static void tcp_dd_alarm_destroy(struct cavan_alarm_node *node, void *data)
{
	free(node);
}

static int tcp_dd_handle_alarm_add_request(struct network_client *client, struct cavan_alarm_thread *alarm, struct tcp_alarm_add_request *req)
{
	int ret;
	char *command;
	struct cavan_alarm_node *node;

	node = malloc(sizeof(*node) + text_len(req->command) + 1);
	if (node == NULL) {
		pd_error_info("malloc");
		return -ENOMEM;
	}

	command = (char *) (node + 1);
	text_copy(command, req->command);

	cavan_alarm_node_init(node, command, tcp_dd_alarm_handler);
	node->time = req->time;
	node->repeat = req->repeat;
	node->destroy = tcp_dd_alarm_destroy;

	ret = cavan_alarm_insert_node(alarm, node, NULL);
	if (ret < 0) {
		tcp_dd_send_response(client, ret, "[Server] cavan_alarm_insert_node");
		goto out_free_node;
	}

	return 0;

out_free_node:
	free(node);
	return ret;
}

static int tcp_dd_handle_alarm_remove_request(struct network_client *client, struct cavan_alarm_thread *alarm, struct tcp_alarm_query_request *req)
{
	struct double_link_node *node;

	node = double_link_get_node(&alarm->link, req->index);
	if (node == NULL) {
		tcp_dd_send_response(client, -ENOENT, "[Server] alarm not found");
		return -ENOENT;
	}

	cavan_alarm_delete_node(alarm, double_link_get_container(&alarm->link, node));

	return 0;
}

static int tcp_dd_handle_alarm_list_request(struct network_client *client, struct cavan_alarm_thread *alarm, struct tcp_alarm_query_request *req)
{
	int ret;
	struct cavan_alarm_node *node;
	struct tcp_alarm_add_request item;

	ret = tcp_dd_send_response(client, 0, "[Server] start send alarm list");
	if (ret < 0) {
		pd_red_info("tcp_dd_send_response");
		return ret;
	}

	double_link_foreach(&alarm->link, node) {
		msleep(1);

		item.time = node->time;
		item.repeat = node->repeat;
		text_copy(item.command, node->private_data);

		ret = client->send(client, (char *) &item, MOFS(struct tcp_alarm_add_request, command) + text_len(item.command) + 1);
		if (ret < 0) {
			pd_red_info("inet_send");
			link_foreach_return(&alarm->link, ret);
		}
	}
	end_link_foreach(&alarm->link);

	return 0;
}

static int tcp_dd_mouse_uinput_init(struct uinput_user_dev *dev, int fd, void *data)
{
	int ret = 0;

	ret |= ioctl(fd, UI_SET_EVBIT, EV_SYN);
	ret |= ioctl(fd, UI_SET_EVBIT, EV_KEY);
	ret |= ioctl(fd, UI_SET_EVBIT, EV_REL);

	ret |= ioctl(fd, UI_SET_RELBIT, REL_X);
	ret |= ioctl(fd, UI_SET_RELBIT, REL_Y);
	ret |= ioctl(fd, UI_SET_RELBIT, REL_WHEEL);

	ret |= ioctl(fd, UI_SET_KEYBIT, BTN_LEFT);
	ret |= ioctl(fd, UI_SET_KEYBIT, BTN_RIGHT);
	ret |= ioctl(fd, UI_SET_KEYBIT, BTN_MIDDLE);

	return ret;
}

static int tcp_dd_keypad_uinput_init(struct uinput_user_dev *dev, int fd, void *data)
{
	int i;
	int ret = 0;

	ret |= ioctl(fd, UI_SET_EVBIT, EV_SYN);
	ret |= ioctl(fd, UI_SET_EVBIT, EV_KEY);
	ret |= ioctl(fd, UI_SET_EVBIT, EV_REP);
	ret |= ioctl(fd, UI_SET_EVBIT, EV_MSC);
	ret |= ioctl(fd, UI_SET_EVBIT, EV_LED);

	for (i = 0; i < KEY_CNT; i++) {
		ret |= ioctl(fd, UI_SET_KEYBIT, i);
	}

#if 0
	for (i = 0; i < LED_CNT; i++) {
		ret |= ioctl(fd, UI_SET_LEDBIT, i);
	}
#endif

	return ret;
}

static int tcp_dd_keypad_uinput_open(struct cavan_tcp_dd_service *service)
{
	int fd;

	network_service_lock(&service->service);

	pd_func_info("keypad_fd = %d, mouse_fd = %d, keypad_use_count = %d, keypad_uinput = %s, keypad_insmod = %s",
		service->keypad_fd, service->mouse_fd, service->keypad_use_count,
		cavan_bool_tostring(service->keypad_uinput),
		cavan_bool_tostring(service->keypad_insmod));

	fd = service->keypad_fd;
	if (fd < 0) {
		service->keypad_uinput = false;
		service->keypad_insmod = false;

		fd = open(TCP_KEYPAD_DEVICE, O_WRONLY);
		if (fd < 0) {
			fd = cavan_uinput_create("TCP_KEYPAD", tcp_dd_keypad_uinput_init, service);
			if (fd < 0 && service->keypad_ko) {
				cavan_system2("insmod \"%s\"", service->keypad_ko);
				service->keypad_insmod = true;
				msleep(200);
				fd = open(TCP_KEYPAD_DEVICE, O_WRONLY);
			} else {
				service->keypad_uinput = true;
				service->mouse_fd = cavan_uinput_create("TCP_MOUSE", tcp_dd_mouse_uinput_init, service);
			}
		}

		service->keypad_fd = fd;
		if (service->mouse_fd < 0) {
			service->mouse_fd = fd;
		}

		service->keypad_use_count = 0;
	}

	if (fd >= 0) {
		service->keypad_use_count++;
	}

	pd_func_info("keypad_fd = %d, mouse_fd = %d, keypad_use_count = %d, keypad_uinput = %s, keypad_insmod = %s",
		service->keypad_fd, service->mouse_fd,
		service->keypad_use_count,
		cavan_bool_tostring(service->keypad_uinput),
		cavan_bool_tostring(service->keypad_insmod));

	network_service_unlock(&service->service);

	return fd;
}

static void tcp_dd_keypad_uinput_close(struct cavan_tcp_dd_service *service, bool force)
{
	network_service_lock(&service->service);

	pd_func_info("keypad_fd = %d, mouse_fd = %d, keypad_use_count = %d, keypad_uinput = %s, keypad_insmod = %s",
		service->keypad_fd, service->mouse_fd,
		service->keypad_use_count,
		cavan_bool_tostring(service->keypad_uinput),
		cavan_bool_tostring(service->keypad_insmod));

	if (force && service->keypad_use_count > 0) {
		service->keypad_use_count = 1;
	}

	if (--service->keypad_use_count == 0 && service->keypad_fd >= 0) {
#if TCP_DD_AUTO_CLOSE_UINPUT
		if (service->mouse_fd != service->keypad_fd) {
			close(service->mouse_fd);
			service->mouse_fd = -1;
		}

		close(service->keypad_fd);
		service->keypad_fd = -1;

		if (service->keypad_insmod && service->keypad_ko) {
			cavan_system2("rmmod \"%s\"", service->keypad_ko);
		}
#endif
	}

	if (service->keypad_use_count < 0) {
		pd_red_info("unbalanced %s %d", __FUNCTION__, service->keypad_use_count);
		service->keypad_use_count = 0;
	}

	pd_func_info("keypad_fd = %d, mouse_fd = %d, keypad_use_count = %d, keypad_uinput = %s, keypad_insmod = %s",
		service->keypad_fd, service->mouse_fd,
		service->keypad_use_count,
		cavan_bool_tostring(service->keypad_uinput),
		cavan_bool_tostring(service->keypad_insmod));

	network_service_unlock(&service->service);
}

static int tcp_dd_remote_ctrl_uinput_init(struct uinput_user_dev *dev, int fd, void *data)
{
	int i;
	int code;
	int ret = 0;

	ret |= ioctl(fd, UI_SET_EVBIT, EV_SYN);
	ret |= ioctl(fd, UI_SET_EVBIT, EV_KEY);

	for (i = 0; i < NELEM(tcp_dd_remote_ctrl_keys); i++) {
		ret |= ioctl(fd, UI_SET_KEYBIT, tcp_dd_remote_ctrl_keys[i]);
	}

	for (code = KEY_1; code <= KEY_0; code++) {
		ret |= ioctl(fd, UI_SET_KEYBIT, code);
	}

	return ret;
}

static int tcp_dd_remote_ctrl_uinput_open(struct cavan_tcp_dd_service *service)
{
	int fd;

	network_service_lock(&service->service);

	pd_func_info("remote_ctrl_fd = %d", service->remote_ctrl_fd);

	if (service->remote_ctrl_fd < 0) {
		service->remote_ctrl_fd = cavan_uinput_create("REMOTE_CTRL", tcp_dd_remote_ctrl_uinput_init, service);
	}

	fd = service->remote_ctrl_fd;

	pd_func_info("remote_ctrl_fd = %d", service->remote_ctrl_fd);

	network_service_unlock(&service->service);

	return fd;
}

static void tcp_dd_remote_ctrl_uinput_close(struct cavan_tcp_dd_service *service)
{
	close(service->remote_ctrl_fd);
	service->remote_ctrl_fd = -1;
}

static int tcp_dd_display_on_off(struct cavan_tcp_dd_service *service)
{
	int brightness;

	if (service->backlight[0] == 0) {
		return -ENOENT;
	}

	brightness = file_read_s64(service->backlight, -1);
	if (brightness < 0) {
		return -EFAULT;
	}

	if (brightness > 0) {
		service->brightness = brightness;
		brightness = 0;
	} else if (service->brightness > 0) {
		brightness = service->brightness;
	} else {
		brightness = 255;
	}

	return file_write_u64(service->backlight, brightness);
}

static int tcp_dd_handle_tcp_keypad_event_request(struct cavan_tcp_dd_service *service, struct network_client *client)
{
	int fd;
	int ret;
	int code;
	ssize_t rdlen;
	bool android = cavan_is_android();

	fd = tcp_dd_keypad_uinput_open(service);
	if (fd < 0) {
		tcp_dd_send_response(client, fd, "[Server] Failed to open input");
		return fd;
	}

	ret = tcp_dd_send_response(client, 0, "[Server] Start recv and write event");
	if (ret < 0) {
		pd_red_info("tcp_dd_send_response");
		goto out_tcp_dd_service_close_input;
	}

	if (service->keypad_uinput) {
		struct input_event event;

		memset(&event.time, 0, sizeof(event.time));

		while (1) {
			rdlen = client->recv(client, (void *) &event.type, sizeof(struct cavan_input_event));
			if (rdlen < (int) sizeof(struct cavan_input_event)) {
				break;
			}

			switch (event.type) {
			case EV_REL:
				fd = service->mouse_fd;
				break;

			case EV_KEY:
				switch (event.code) {
				case BTN_LEFT:
				case BTN_RIGHT:
				case BTN_MIDDLE:
					fd = service->mouse_fd;
					break;

				default:
					fd = service->keypad_fd;
					if (android) {
						switch (event.code) {
						case KEY_F1:
						case KEY_F5:
						case KEY_F9:
							event.code = KEY_BACK;
							break;

						case KEY_F2:
							event.code = KEY_COMPOSE;
							break;

						case KEY_F6:
						case KEY_F10:
							event.code = KEY_MENU;
							break;

						case KEY_F3:
						case KEY_F7:
						case KEY_F11:
							event.code = KEY_HOMEPAGE;
							break;

						case KEY_F4:
							event.code = KEY_COFFEE;
							break;

						case KEY_F8:
						case KEY_F12:
							event.code = KEY_POWER;
							break;

						case KEY_PAGEUP:
							event.code = KEY_VOLUMEUP;
							break;

						case KEY_PAGEDOWN:
							event.code = KEY_VOLUMEDOWN;
							break;

						case KEY_DISPLAYTOGGLE:
							if (event.value > 0) {
								tcp_dd_display_on_off(service);
							}
							continue;
						}
					}
				}
				break;

			case EV_SYN:
				break;

			default:
				fd = service->keypad_fd;
			}

			ret = cavan_input_event(fd, &event, 1);
			if (ret < 0) {
				pd_error_info("write events");
				goto out_tcp_dd_service_close_input;
			}
		}

		event.type = EV_KEY;
		event.value = 0;

		for (code = 0; code < KEY_CNT; code++) {
			event.code = code;
			ret = cavan_input_event(fd, &event, 1);
			if (ret < 0) {
				goto out_tcp_dd_service_close_input;
			}
		}

		ret = cavan_input_sync(fd);
		if (ret < 0) {
			goto out_tcp_dd_service_close_input;
		}
	} else {
		ssize_t wrlen;
		struct cavan_input_event events[32];

		while (1) {
			rdlen = client->recv(client, events, sizeof(events));
			if (rdlen < (int) sizeof(struct cavan_input_event)) {
				break;
			}

			wrlen = ffile_write(fd, events, rdlen);
			if (wrlen < rdlen) {
				pd_error_info("write events");
				ret = wrlen < 0 ? wrlen : -EFAULT;
				goto out_tcp_dd_service_close_input;
			}
		}

		events[0].type = EV_KEY;
		events[0].value = 0;

		for (code = 1; code < KEY_CNT; code++) {
			events[0].code = code;
			ret = ffile_write(fd, events, sizeof(events[0]));
			if (ret < 0) {
				goto out_tcp_dd_service_close_input;
			}
		}

		events[0].type = EV_SYN;
		events[0].code = SYN_REPORT;
		ret = ffile_write(fd, events, sizeof(events[0]));
		if (ret < 0) {
			goto out_tcp_dd_service_close_input;
		}
	}

	ret = 0;

out_tcp_dd_service_close_input:
	tcp_dd_keypad_uinput_close(service, false);
	return ret;
}

static int tcp_dd_handle_remote_ctrl_request(struct cavan_tcp_dd_service *service, struct network_client *client)
{
	int i;
	int fd;
	int ret;
	struct input_event events[2];

	fd = tcp_dd_remote_ctrl_uinput_open(service);
	if (fd < 0) {
		tcp_dd_send_response(client, fd, "[Server] Failed to open input");
		return fd;
	}

	ret = tcp_dd_send_response(client, 0, "[Server] Start recv and write event");
	if (ret < 0) {
		pd_red_info("tcp_dd_send_response");
		return ret;
	}

	events[0].type = EV_KEY;
	events[1].type = EV_SYN;
	events[1].code = SYN_REPORT;
	events[1].value = 0;

	while (1) {
		int count;
		ssize_t rdlen;
		u16 buff[32];

		rdlen = client->recv(client, buff, sizeof(buff));
		if (rdlen < 2) {
			break;
		}

		count = rdlen / 2;

		for (i = 0; i < count; i++) {
			events[0].code = buff[i] & 0x7FFF;
			events[0].value = buff[i] >> 15;

			switch (events[0].code) {
			case KEY_DISPLAYTOGGLE:
				if (events[0].value > 0) {
					tcp_dd_display_on_off(service);
				}
				break;

			default:
				ret = cavan_input_event(fd, events, NELEM(events));
				if (ret < 0) {
					pd_error_info("cavan_input_event");
					return ret;
				}
			}
		}
	}

	events[0].value = 0;

	for (i = 0; i < KEY_CNT; i++) {
		events[0].code = i;
		ret = cavan_input_event(fd, events, 1);
		if (ret < 0) {
			pd_error_info("cavan_input_event");
			return ret;
		}
	}

	return cavan_input_sync(fd);
}

static int tcp_dd_handle_mkdir_request(struct network_client *client, struct tcp_dd_mkdir_request *req)
{
	int ret;

	ret = cavan_mkdir_main2(req->pathname, req->mode);
	tcp_dd_send_response(client, ret, NULL);

	return ret;
}

static int tcp_dd_handle_rddir_request(struct network_client *client, struct tcp_dd_rddir_request *req)
{
	DIR *dp;
	int ret;
	struct dirent *en;

	dp = opendir(req->pathname);
	if (dp == NULL) {
		ret = -ENOENT;
		tcp_dd_send_response(client, ret, "opendir");
		return ret;
	}

	ret = tcp_dd_send_response(client, 0, NULL);
	if (ret < 0) {
		goto out_closedir;
	}

	while ((en = cavan_readdir_skip_dot(dp))) {
		char *name = en->d_name;
		int length = strlen(name);

		name[length] = '\n';

		ret = client->send(client, name, length + 1);
		if (ret < 0) {
			break;
		}
	}

out_closedir:
	closedir(dp);
	return ret;
}

static int tcp_dd_handle_discovery_request(struct network_client *client)
{
	char hostname[64];

	return network_client_printf(client, "TCP_DD: hostname = %s",
		network_get_hostname(hostname, sizeof(hostname)));
}

static void tcp_dd_apk_clean_handler(void *data)
{
	char pathname[1024];

	cavan_path_build_tmp_path(TCP_DD_APK_CACHE_NAME, pathname, sizeof(pathname));
	pd_info("remove directory: %s", pathname);
	remove_directory(pathname);
}

static int tcp_dd_handle_install_request(struct network_client *client, size64_t size)
{
	int fd;
	int ret;
	char pathname[1024];

	fd = cavan_temp_file_open(pathname, sizeof(pathname), TCP_DD_APK_CACHE_NAME "/XXXXXX", false);
	if (fd < 0) {
		pd_red_info("cavan_temp_file_open: %d", fd);
		return fd;
	}

	ret = tcp_dd_send_response(client, 0, "save: %s", pathname);
	if (ret < 0) {
		pd_red_info("cavan_temp_file_open: %d", fd);
		goto out_close_fd;
	}

	ret = network_client_recv_file(client, fd, 0, size);
	if (ret < 0) {
		pr_red_info("network_client_recv_file: %d", ret);
		goto out_close_fd;
	}

	close(fd);
	chmod(pathname, 0777);

	cavan_async_command_cancel(NULL, tcp_dd_apk_clean_handler, 0);
	ret = android_install_application(pathname);
	cavan_async_command_execute(NULL, tcp_dd_apk_clean_handler, NULL, 1000 * 60 * 5);
	if (ret < 0) {
		tcp_dd_send_response(client, ret, "android_install_application");
		goto out_unlink;
	}

	tcp_dd_send_response(client, 0, NULL);

	return 0;

out_close_fd:
	close(fd);
out_unlink:
	unlink(pathname);
	return ret;
}

static int tcp_dd_service_open_connect(struct cavan_dynamic_service *service, void *conn)
{
	struct cavan_tcp_dd_service *dd_service = cavan_dynamic_service_get_data(service);

	return dd_service->service.accept(&dd_service->service, conn, CAVAN_NET_FLAG_NODELAY);
}

static bool tcp_dd_service_close_connect(struct cavan_dynamic_service *service, void *conn)
{
	network_client_close(conn);
	return false;
}

static int tcp_dd_service_start_handler(struct cavan_dynamic_service *service)
{
	int ret;
	struct cavan_tcp_dd_service *dd_service = cavan_dynamic_service_get_data(service);

	ret = network_service_open(&dd_service->service, &dd_service->url, 0);
	if (ret < 0) {
		pd_red_info("network_service_open2");
		return ret;
	}

	if (dd_service->discovery.port > 0) {
		char hostname[64];

		ret = udp_discovery_service_start(&dd_service->discovery, "TCP_DD: port = %d, hostname = %s",
		dd_service->url.port, network_get_hostname(hostname, sizeof(hostname)));
		if (ret < 0) {
			pd_red_info("udp_discovery_service_start");
			goto out_network_service_close;
		}
	}

	ret = cavan_alarm_thread_init(&dd_service->alarm);
	if (ret < 0) {
		pd_red_info("cavan_alarm_thread_init");
		goto out_udp_discovery_service_stop;
	}

	ret = cavan_alarm_thread_start(&dd_service->alarm);
	if (ret < 0) {
		pd_red_info("cavan_alarm_thread_start");
		goto out_cavan_alarm_thread_deinit;
	}

	dd_service->filename = tcp_dd_find_platform_by_name_path(dd_service->pathname, NULL, sizeof(dd_service->pathname));
	if (dd_service->filename) {
		pd_green_info("pathname = %s", dd_service->pathname);
	} else {
		dd_service->filename = dd_service->pathname;
	}

	dd_service->part_table = cavan_block_get_part_table2();
	if (dd_service->part_table != NULL) {
		cavan_part_table_dump(dd_service->part_table);
	}

	if (cavan_fb_get_backlight_path(dd_service->backlight, sizeof(dd_service->backlight)) == NULL) {
		dd_service->backlight[0] = 0;
	} else {
		pd_green_info("backlight = %s", dd_service->backlight);
	}

	dd_service->mouse_fd = -1;
	dd_service->keypad_fd = -1;
	dd_service->remote_ctrl_fd = -1;
	dd_service->keypad_use_count = 0;

	cavan_async_command_execute(NULL, tcp_dd_apk_clean_handler, NULL, 0);

	return 0;

out_cavan_alarm_thread_deinit:
	cavan_alarm_thread_deinit(&dd_service->alarm);
out_udp_discovery_service_stop:
	if (dd_service->discovery.port > 0) {
		udp_discovery_service_stop(&dd_service->discovery);
	}
out_network_service_close:
	network_service_close(&dd_service->service);
	return ret;
}

static void tcp_dd_service_stop_handler(struct cavan_dynamic_service *service)
{
	struct cavan_tcp_dd_service *dd_service = cavan_dynamic_service_get_data(service);

	tcp_dd_keypad_uinput_close(dd_service, true);
	tcp_dd_remote_ctrl_uinput_close(dd_service);

	cavan_alarm_thread_stop(&dd_service->alarm);
	cavan_alarm_thread_deinit(&dd_service->alarm);

	if (dd_service->discovery.port > 0) {
		udp_discovery_service_stop(&dd_service->discovery);
	}

	network_service_close(&dd_service->service);
}

static int tcp_dd_service_run_handler(struct cavan_dynamic_service *service, void *conn)
{
	int ret;
	bool need_response;
	struct tcp_dd_package pkg;
	struct network_client *client = conn;
	struct cavan_tcp_dd_service *dd_service = cavan_dynamic_service_get_data(service);

	ret = tcp_dd_package_recv(client, &pkg);
	if (ret < 0) {
		pd_error_info("tcp_dd_package_recv %d", ret);
		return ret < 0 ? ret : -EFAULT;
	}

	need_response = false;

	switch (pkg.type) {
	case TCP_DD_READ:
		pd_bold_info("TCP_DD_READ");
		ret = tcp_dd_handle_read_request(dd_service, client, &pkg);
		break;

	case TCP_DD_WRITE:
		pd_bold_info("TCP_DD_WRITE");
		ret = tcp_dd_handle_write_request(dd_service, client, &pkg);
		need_response = true;
		break;

	case TCP_DD_EXEC:
		pd_bold_info("TCP_DD_EXEC");
		ret = tcp_dd_handle_exec_request(client, &pkg.exec_req);
		break;

	case TCP_ALARM_ADD:
		pd_bold_info("TCP_ALARM_ADD");
		ret = tcp_dd_handle_alarm_add_request(client, &dd_service->alarm, &pkg.alarm_add);
		need_response = true;
		break;

	case TCP_ALARM_REMOVE:
		pd_bold_info("TCP_ALARM_REMOVE");
		ret = tcp_dd_handle_alarm_remove_request(client, &dd_service->alarm, &pkg.alarm_query);
		need_response = true;
		break;

	case TCP_ALARM_LIST:
		pd_bold_info("TCP_ALARM_LIST");
		ret = tcp_dd_handle_alarm_list_request(client, &dd_service->alarm, &pkg.alarm_query);
		break;

	case TCP_KEYPAD_EVENT:
		pd_bold_info("TCP_KEYPAD_EVENT");
		ret = tcp_dd_handle_tcp_keypad_event_request(dd_service, client);
		break;

	case TCP_DD_MKDIR:
		pd_bold_info("TCP_DD_MKDIR");
		ret = tcp_dd_handle_mkdir_request(client, &pkg.mkdir_pkg);
		break;

	case TCP_DD_RDDIR:
		pd_bold_info("TCP_DD_RDDIR");
		ret = tcp_dd_handle_rddir_request(client, &pkg.rddir_pkg);
		break;

	case TCP_DD_DISCOVERY:
		pd_bold_info("TCP_DD_DISCOVERY");
		ret = tcp_dd_handle_discovery_request(client);
		break;

	case TCP_REMOTE_CTRL:
		pd_bold_info("TCP_REMOTE_CTRL");
		ret = tcp_dd_handle_remote_ctrl_request(dd_service, client);
		break;

	case TCP_DD_INSTALL:
		pd_bold_info("TCP_DD_INSTALL");
		ret = tcp_dd_handle_install_request(client, pkg.value64);
		break;

	default:
		pd_red_info("Unknown package type %d", pkg.type);
		return -EINVAL;
	}

	if (need_response && ret >= 0) {
		tcp_dd_send_response(client, ret, NULL);
	}

	msleep(100);

	return ret;
}

int tcp_dd_service_run(struct cavan_dynamic_service *service)
{
	service->name = "TCP_DD";
	service->conn_size = sizeof(struct network_client);

	service->start = tcp_dd_service_start_handler;
	service->stop = tcp_dd_service_stop_handler;
	service->run = tcp_dd_service_run_handler;
	service->open_connect = tcp_dd_service_open_connect;
	service->close_connect = tcp_dd_service_close_connect;

	return cavan_dynamic_service_run(service);
}

static int tcp_dd_check_file_request(struct network_file_request *file_req, const char **src_file, const char **dest_file)
{
	if (file_req->src_file[0] == 0 && file_req->dest_file[0] == 0) {
		pd_red_info("src_file == NULL && dest_file == NULL");
		ERROR_RETURN(EINVAL);
	}

	if (file_req->src_file[0] == 0) {
		*src_file = *dest_file = file_req->dest_file;
	} else if (file_req->dest_file[0] == 0) {
		*src_file = *dest_file = file_req->src_file;
	} else {
		*src_file = file_req->src_file;
		*dest_file = file_req->dest_file;
	}

	return 0;
}

int tcp_dd_send_file(struct network_url *url, struct network_file_request *file_req, u32 flags)
{
	int fd;
	int ret;
	bool directory;
	struct stat st;
	const char *src_file = NULL;
	const char *dest_file = NULL;
	struct network_client client;
	struct tcp_dd_package pkg;
	struct tcp_dd_file_request *req = &pkg.file_req;

	ret = tcp_dd_check_file_request(file_req, &src_file, &dest_file);
	if (ret < 0) {
		return ret;
	}

	fd = open(src_file, O_RDONLY);
	if (fd < 0) {
		pr_error_info("Open file `%s' failed", src_file);
		return fd;
	}

	ret = fstat(fd, &st);
	if (ret < 0) {
		pr_error_info("Get file `%s' stat failed", src_file);
		goto out_close_fd;
	}

	directory = S_ISDIR(st.st_mode);
	if (!directory) {
		if (st.st_size > 0) {
			if (file_req->size == 0) {
				file_req->size = st.st_size;
			}

			if (file_req->size < file_req->src_offset) {
				pr_red_info("No data to sent");
				return -EINVAL;
			}
		} else {
			file_req->size = 0;
		}

		if (lseek(fd, file_req->src_offset, SEEK_SET) != (off_t) file_req->src_offset) {
			ret = -EFAULT;
			pr_error_info("Seek file `%s' failed", src_file);
			goto out_close_fd;
		}

		if (file_req->size > 0) {
			file_req->size -= file_req->src_offset;
		}
	}

	ret = network_client_open(&client, url, CAVAN_NET_FLAG_TALK | CAVAN_NET_FLAG_SYNC | CAVAN_NET_FLAG_WAIT | CAVAN_NET_FLAG_NODELAY);
	if (ret < 0) {
		pr_red_info("network_client_open2");
		goto out_close_fd;
	}

	req->offset = file_req->dest_offset;
	req->size = file_req->size;
	req->mode = st.st_mode;

	ret = tcp_dd_send_file_request(&client, &pkg, TCP_DD_WRITE, dest_file, flags);
	if (ret < 0) {
		pr_red_info("tcp_dd_send_file_request: %d", ret);
		goto out_client_close;
	}

	if (directory) {
		DIR *dp;
		struct dirent *en;
		char *src_fname, *dest_fname;
		struct network_file_request sub_req;

		dp = opendir(file_req->src_file);
		if (dp == NULL) {
			pr_err_info("opendir");
			ret = -EFAULT;
			goto out_client_close;
		}

		src_fname = cavan_path_copy(sub_req.src_file, sizeof(sub_req.src_file), file_req->src_file, true);
		dest_fname = cavan_path_copy(sub_req.dest_file, sizeof(sub_req.dest_file), file_req->dest_file, true);
		sub_req.src_offset = sub_req.dest_offset = 0;

		while ((en = cavan_readdir_skip_dot(dp))) {
			sub_req.size = 0;
			strcpy(src_fname, en->d_name);
			strcpy(dest_fname, en->d_name);

			ret = tcp_dd_send_file(url, &sub_req, flags);
			if (ret < 0) {
				pr_red_info("tcp_dd_send_file");
				break;
			}
		}

		closedir(dp);
	} else {
		size64_t skip;

		if (pkg.type == TCP_DD_BREAKPOINT) {
			skip = req->offset;
			if (skip > (size64_t) file_req->size) {
				ret = -EINVAL;
				pr_red_info("invalid skip");
				goto out_close_fd;
			}

			file_req->size -= skip;
		} else {
			skip = 0;
		}

		println("%s => %s", src_file, dest_file);
		println("offset = %s", size2text(file_req->src_offset));
		println("size = %s", size2text(file_req->size));
		println("skip = %s", size2text(skip));

		msleep(200);

		ret = network_client_send_file(&client, fd, skip, file_req->size);
		if (ret < 0) {
			pr_red_info("network_client_send_file");
			goto out_close_fd;
		}

		if (file_req->size > 0) {
			ret = tcp_dd_recv_response(&client);
		}
	}

out_client_close:
	// msleep(100);
	client.close(&client);
out_close_fd:
	close(fd);
	return ret;
}

int tcp_dd_receive_file(struct network_url *url, struct network_file_request *file_req, u32 flags)
{
	int fd;
	int ret;
	mode_t mode;
	const char *src_file = NULL;
	const char *dest_file = NULL;
	struct network_client client;
	struct tcp_dd_package pkg;
	struct tcp_dd_file_request *req = &pkg.file_req;

	umask(0);

	ret = tcp_dd_check_file_request(file_req, &src_file, &dest_file);
	if (ret < 0) {
		return ret;
	}

	if (file_test(dest_file, "b") == 0) {
		umount_partition(dest_file, MNT_DETACH);
	}

	ret = network_client_open(&client, url, CAVAN_NET_FLAG_TALK | CAVAN_NET_FLAG_SYNC | CAVAN_NET_FLAG_WAIT | CAVAN_NET_FLAG_NODELAY);
	if (ret < 0) {
		pr_red_info("inet_create_tcp_link2");
		return ret;
	}

	req->offset = file_req->src_offset;
	req->size = file_req->size;

	ret = tcp_dd_send_file_request(&client, &pkg, TCP_DD_READ, src_file, flags);
	if (ret < 0) {
		pr_red_info("tcp_dd_send_read_request");
		goto out_client_close;
	}

	mode = pkg.file_req.mode;
	if (S_ISDIR(mode)) {
		char *dest_fname, *src_fname;
		struct network_file_request sub_req;

		ret = cavan_mkdir_main2(dest_file, mode);
		if (ret < 0) {
			tcp_dd_send_response(&client, ret, "[Client] mkdir `%s' failed", dest_file);
			goto out_client_close;
		}

		ret = tcp_dd_send_response(&client, 0, "[Client] Start receive directory");
		if (ret < 0) {
			pr_red_info("tcp_dd_send_response");
			goto out_client_close;
		}

		dest_fname = cavan_path_copy(sub_req.dest_file, sizeof(sub_req.dest_file), file_req->dest_file, true);
		src_fname = cavan_path_copy(sub_req.src_file, sizeof(sub_req.src_file), file_req->src_file, true);
		sub_req.dest_offset = sub_req.src_offset = 0;

		while (1) {
			char *p, line[256];

			p = network_client_recv_line(&client, line, sizeof(line));
			if (p == NULL) {
				ret = -EFAULT;
				pr_red_info("network_client_recv_line");
				goto out_client_close;
			}

			if (p == line) {
				break;
			}

			*p = 0;

			println("line = %s", line);

			strcpy(dest_fname, line);
			strcpy(src_fname, line);
			sub_req.size = 0;

			ret = tcp_dd_receive_file(url, &sub_req, flags);
			if (ret < 0) {
				pr_red_info("tcp_dd_receive_file");
				goto out_client_close;
			}
		}

		goto out_client_close;
	} else {
		if (file_req->size == 0) {
			file_req->size = pkg.file_req.size;
			if (file_req->size == 0) {
				mode = 0777;
			}
		}

		fd = open(dest_file, O_CREAT | O_WRONLY | O_TRUNC | O_BINARY, mode);
		if (fd < 0) {
			ret = fd;
			tcp_dd_send_response(&client, fd, "[Client] Open file `%s' failed", dest_file);
			goto out_client_close;
		}

		if (lseek(fd, file_req->dest_offset, SEEK_SET) != (off_t) file_req->dest_offset) {
			ret = -EFAULT;
			tcp_dd_send_response(&client, ret, "[Client] Seek file `%s' failed", dest_file);
			goto out_close_fd;
		}

		ret = tcp_dd_send_response(&client, 0, "[Client] Start receive file");
		if (ret < 0) {
			pr_red_info("tcp_dd_send_response");
			goto out_close_fd;
		}

		println("%s <= %s", dest_file, src_file);
		println("offset = %s", size2text(file_req->dest_offset));
		println("size = %s", size2text(file_req->size));

		ret = network_client_recv_file(&client, fd, 0, file_req->size);
	}

out_close_fd:
	close(fd);
out_client_close:
	// msleep(100);
	client.close(&client);
	return ret;
}

int tcp_dd_exec_command(struct network_url *url, const char *command)
{
	int ret;
	struct termios tty_attr;
	struct network_client client;

	ret = network_client_open(&client, url, CAVAN_NET_FLAG_TALK | CAVAN_NET_FLAG_SYNC | CAVAN_NET_FLAG_WAIT | CAVAN_NET_FLAG_NODELAY);
	if (ret < 0) {
		pr_red_info("network_client_open2");
		return ret;
	}

	ret = tcp_dd_send_exec_request(&client, stdout_fd, command);
	if (ret < 0) {
		pr_red_info("tcp_dd_send_exec_request");
		goto out_client_close;
	}

	ret = cavan_tty_set_mode(stdin_fd, CAVAN_TTY_MODE_SSH, &tty_attr);
	if (ret < 0) {
		pr_red_info("cavan_tty_set_mode");
		goto out_client_close;
	}

	ret = network_client_exec_redirect(&client, stdin_fd, stdout_fd);
	cavan_tty_attr_restore(stdin_fd, &tty_attr);
out_client_close:
	client.close(&client);
	return ret;
}

static bool tcp_dd_keypad_event_handler(struct cavan_event_device *dev, struct input_event *event_raw, void *data)
{
	ssize_t wrlen;
	struct network_client *client = data;
	struct cavan_input_event *event = (struct cavan_input_event *) &event_raw->type;

	wrlen = client->send(client, event, sizeof(struct cavan_input_event));
	if (wrlen < 0) {
		cavan_event_should_stop(dev->service);
	}

	return true;
}

static void tcp_dd_keypad_event_key(struct cavan_input_event *event, int code, int value)
{
	event->type = EV_KEY;
	event->code = code;
	event->value = value;
}

static void tcp_dd_keypad_event_mouse(struct cavan_input_event *event, int code, int value)
{
	event->type = EV_REL;
	event->code = code;
	event->value = value;
}

static void tcp_dd_keypad_event_sync(struct cavan_input_event *event)
{
	event->type = EV_SYN;
	event->code = SYN_REPORT;
	event->value = 0;
}

static void tcp_dd_keypad_key_shift(struct cavan_input_event *event, int value)
{
	tcp_dd_keypad_event_key(event, KEY_LEFTSHIFT, value);
}

static void tcp_dd_keypad_key_ctrl(struct cavan_input_event *event, int value)
{
	tcp_dd_keypad_event_key(event, KEY_LEFTCTRL, value);
}

static int tcp_dd_keypad_send_key_events(struct network_client *client, u16 code)
{
	struct cavan_input_event events[8];
	struct cavan_input_event *event;

	if (code == 0) {
		return 0;
	}

	event = events;

	if (code & TCP_KEYPADF_CTRL) {
		tcp_dd_keypad_key_ctrl(event++, 1);
	}

	if (code & TCP_KEYPADF_SHIFT) {
		tcp_dd_keypad_key_shift(event++, 1);
	}

	tcp_dd_keypad_event_key(event++, code & 0x0FFF, 1);
	tcp_dd_keypad_event_sync(event++);

	tcp_dd_keypad_event_key(event++, code & 0x0FFF, 0);

	if (code & TCP_KEYPADF_SHIFT) {
		tcp_dd_keypad_key_shift(event++, 0);
	}

	if (code & TCP_KEYPADF_CTRL) {
		tcp_dd_keypad_key_ctrl(event++, 0);
	}

	tcp_dd_keypad_event_sync(event++);

	return client->send(client, events, (event - events) * sizeof(struct cavan_input_event));
}

static int tcp_dd_keypad_send_mouse_events(struct network_client *client, u16 code)
{
	struct cavan_input_event events[8];
	struct cavan_input_event *event;

	if (code == 0) {
		return 0;
	}

	event = events;

	switch (code) {
	case KEY_LEFT:
	case KEY_J:
		tcp_dd_keypad_event_mouse(event++, REL_X, -10);
		break;

	case KEY_RIGHT:
	case KEY_L:
		tcp_dd_keypad_event_mouse(event++, REL_X, 10);
		break;

	case KEY_UP:
	case KEY_I:
		tcp_dd_keypad_event_mouse(event++, REL_Y, -10);
		break;

	case KEY_DOWN:
	case KEY_K:
		tcp_dd_keypad_event_mouse(event++, REL_Y, 10);
		break;

	case KEY_PAGEUP:
	case KEY_E:
		tcp_dd_keypad_event_mouse(event++, REL_WHEEL, 1);
		break;

	case KEY_PAGEDOWN:
	case KEY_D:
		tcp_dd_keypad_event_mouse(event++, REL_WHEEL, -1);
		break;

	case KEY_ENTER:
		return tcp_dd_keypad_send_key_events(client, BTN_LEFT);

	case KEY_SPACE:
	case KEY_F:
		return tcp_dd_keypad_send_key_events(client, BTN_RIGHT);

	case KEY_H:
		return tcp_dd_keypad_send_key_events(client, BTN_MIDDLE);

	case KEY_W:
		return tcp_dd_keypad_send_key_events(client, KEY_LEFTMETA);

	case KEY_M:
		return tcp_dd_keypad_send_key_events(client, KEY_RIGHTMETA);

	default:
		return 0;
	}

	tcp_dd_keypad_event_sync(event++);

	return client->send(client, events, (event - events) * sizeof(struct cavan_input_event));
}

static int tcp_dd_keypad_read_keycode(void)
{
	char buff[8];
	int length;
	int i;

	length = ffile_read(stdin_fd, buff, sizeof(buff));
	if (length <= 0) {
		pr_err_info("read: %d", length);

		if (length < 0) {
			return length;
		}

		return -EINVAL;
	}

	for (i = 0; i < length; i++) {
		println("char[%d] = 0x%02x = %d", i, buff[i], buff[i]);
	}

	if (length < 2) {
		int value = buff[0];

		if (value == 28) {
			return EOF;
		}

		return tcp_dd_keypad_char_key_map[value];
	}

	if (buff[0] != 27 || buff[1] != 91 || length < 3) {
		return 0;
	}

	switch (buff[2]) {
	case 49:
		if (length < 4) {
			break;
		}

		switch (buff[3]) {
		case 126:
			return KEY_HOME;

		case 49:
			return KEY_F1;

		case 50:
			return KEY_F2;

		case 51:
			return KEY_F3;

		case 52:
			return KEY_F4;

		case 53:
			return KEY_F5;

		case 55:
			return KEY_F6;

		case 56:
			return KEY_F7;

		case 57:
			return KEY_F8;
		}
		break;

	case 50:
		if (length < 4) {
			break;
		}

		switch (buff[3]) {
		case 126:
			return KEY_INSERT;

		case 48:
			return KEY_F9;

		case 49:
			return KEY_F10;

		case 51:
			return KEY_F11;

		case 52:
			return KEY_F12;
		}
		break;

	case 51:
		return KEY_DELETE;

	case 52:
		return KEY_END;

	case 53:
		return KEY_PAGEUP;

	case 54:
		return KEY_PAGEDOWN;

	case 65:
		return KEY_UP;

	case 66:
		return KEY_DOWN;

	case 67:
		return KEY_RIGHT;

	case 68:
		return KEY_LEFT;
	}

	return 0;
}

int tcp_dd_keypad_client_run(struct network_url *url, int flags)
{
	int ret;
	struct network_client client;

	ret = network_client_open(&client, url, CAVAN_NET_FLAG_TALK | CAVAN_NET_FLAG_SYNC | CAVAN_NET_FLAG_WAIT | CAVAN_NET_FLAG_NODELAY);
	if (ret < 0) {
		pr_red_info("network_client_open2");
		return ret;
	}

	ret = tcp_dd_send_keypad_request(&client);
	if (ret < 0) {
		pr_red_info("tcp_dd_send_exec_request");
		goto out_client_close;
	}

	if (flags & TCP_KEYPADF_CMDLINE) {
		int code = 0;
		struct cavan_input_event events[2];

		events[0].type = EV_KEY;
		events[1].value = 0;
		events[1].type = EV_SYN;
		events[1].code = SYN_REPORT;

		while (1) {
			char name[32], *p;

			print("\033[01;32mTCP-KEYPAD\033[0m> ");

			for (p = name; is_empty_character((*p = getchar())) == 0; p++);

			if (p == name) {
				if (code == 0) {
					continue;
				}

				goto label_repo_key;
			}

			*p = 0;

			if (text_is_number(name)) {
				code = text2value_unsigned(name, NULL, 10);
			} else {
				struct cavan_input_key *key;

				key = cavan_input_find_key_by_name(name);
				if (key == NULL) {
					pr_red_info("key %s not found", name);
					continue;
				}

				code = key->code;
			}

label_repo_key:
			println("keycode = %d", code);

			events[0].code = code;
			events[0].value = 1;
			ret |= client.send(&client, events, sizeof(events));
			msleep(100);
			events[0].value = 0;
			ret |= client.send(&client, events, sizeof(events));
			if (ret < 0) {
				break;
			}
		}
	} else if (flags  & (TCP_KEYPADF_KEYPAD | TCP_KEYPADF_MOUSE)) {
		bool keypad = (flags & TCP_KEYPADF_KEYPAD) != 0;
		struct termios attr;

		cavan_tty_set_mode(stdin_fd, CAVAN_TTY_MODE_SSH, &attr);

		while (1) {
			int code = tcp_dd_keypad_read_keycode();

			if (code < 0) {
				break;
			}

			if (code == KEY_F12) {
				keypad = !keypad;

				if (keypad) {
					println("switch to keypad mode");
				} else {
					println("switch to mouse mode");
				}
			} else if (keypad) {
				ret = tcp_dd_keypad_send_key_events(&client, code);
				if (ret < 0) {
					pr_err_info("tcp_dd_keypad_send_key_events: %d", ret);
					break;
				}
			} else {
				ret = tcp_dd_keypad_send_mouse_events(&client, code);
				if (ret < 0) {
					pr_err_info("tcp_dd_keypad_send_key_events: %d", ret);
					break;
				}
			}
		}

		cavan_tty_attr_restore(stdin_fd, &attr);
	}  else {
		struct cavan_event_service service;

		cavan_event_service_init(&service, NULL);
		service.event_handler = tcp_dd_keypad_event_handler;

		ret = cavan_event_service_start(&service, &client);
		if (ret < 0) {
			pr_red_info("cavan_event_service_start");
			goto out_client_close;
		}

#if 0
		if (flags & TCP_KEYPADF_EXIT_ACK) {
			cavan_set_exit_ask();
		}
#else
		signal(SIGINT, SIG_IGN);
#endif

#if 0
		cavan_event_service_join(&service);
#else
		while (1) {
			char buff[4];
			ssize_t rdlen = client.recv(&client, buff, sizeof(buff));

			if (rdlen <= 0) {
				break;
			}
		}
#endif

		cavan_event_service_stop(&service);
	}

out_client_close:
	client.close(&client);
	return ret;
}


int tcp_alarm_add(struct network_url *url, const char *command, time_t time, time_t repeat)
{
	int ret;
	struct network_client client;

	ret = network_client_open(&client, url, CAVAN_NET_FLAG_TALK | CAVAN_NET_FLAG_SYNC | CAVAN_NET_FLAG_WAIT | CAVAN_NET_FLAG_NODELAY);
	if (ret < 0) {
		pr_red_info("network_client_open2");
		return ret;
	}

	cavan_show_date2(time, "date = ");
	pr_bold_info("repeat = %lds", repeat);
	pr_bold_info("command = %s", command);

	ret = tcp_dd_send_alarm_add_request(&client, time, repeat, command);
	client.close(&client);

	return ret;
}

int tcp_alarm_remove(struct network_url *url, int index)
{
	int ret;
	struct network_client client;

	ret = network_client_open(&client, url, CAVAN_NET_FLAG_TALK | CAVAN_NET_FLAG_SYNC | CAVAN_NET_FLAG_WAIT | CAVAN_NET_FLAG_NODELAY);
	if (ret < 0) {
		pr_red_info("network_client_open2");
		return ret;
	}

	ret = tcp_dd_send_alarm_query_request(&client, TCP_ALARM_REMOVE, index);
	client.close(&client);

	return ret;
}

int tcp_alarm_list(struct network_url *url, int index)
{
	int ret;
	struct network_client client;
	struct tcp_alarm_add_request alarm;

	ret = network_client_open(&client, url, CAVAN_NET_FLAG_TALK | CAVAN_NET_FLAG_SYNC | CAVAN_NET_FLAG_WAIT | CAVAN_NET_FLAG_NODELAY);
	if (ret < 0) {
		pr_red_info("network_client_open2");
		return ret;
	}

	ret = tcp_dd_send_alarm_query_request(&client, TCP_ALARM_LIST, index);
	if (ret < 0) {
		pr_red_info("tcp_dd_send_alarm_query_request");
		goto out_client_close;
	}

	index = 0;

	while (1) {
		char prompt[1024];

		ret = client.recv(&client, &alarm, sizeof(alarm));
		if (ret <= 0) {
			break;
		}

		sprintf(prompt, "index = %d, command = %s, repeat = %ds, date = ", index++, alarm.command, alarm.repeat);
		cavan_show_date2(alarm.time, prompt);
	}

out_client_close:
	client.close(&client);
	return ret;
}

int tcp_dd_mkdir(struct network_url *url, const char *pathname, mode_t mode)
{
	size_t length;
	struct tcp_dd_package pkg;

	pkg.mkdir_pkg.mode  = mode;
	length = ADDR_OFFSET(text_ncopy(pkg.mkdir_pkg.pathname, pathname, sizeof(pkg.mkdir_pkg.pathname)), &pkg) + 1;

	return tcp_dd_send_request4(url, &pkg, TCP_DD_MKDIR, length, 0);
}

static bool tcp_dd_discovery_handler(struct tcp_discovery_client *client, struct tcp_discovery_data *data)
{
	int ret;
	ssize_t rdlen;
	char message[1024];
	struct tcp_dd_discovery_client *tcp_dd;

	ret = tcp_dd_send_empty_request(&data->client, TCP_DD_DISCOVERY, NULL, 0);
	if (ret < 0) {
		return false;
	}

	rdlen = network_client_recv(&data->client, message, sizeof(message) - 1);
	if (rdlen <= 0) {
		return false;
	}

	message[rdlen] = 0;

	if (text_lhcmp("TCP_DD:", message)) {
		return false;
	}

	tcp_dd = client->private_data;

	return tcp_dd->handler(tcp_dd, data, message, rdlen);
}

static bool tcp_dd_discovery_handler_dummy(struct tcp_dd_discovery_client *client, struct tcp_discovery_data *data, const char *message, size_t size)
{
	int ret;
	struct in_addr addr;

	ret = network_client_get_remote_ip(&data->client, &addr);
	if (ret < 0) {
		return false;
	}

	pr_green_info("%03d. %s\t\t%s", data->index, inet_ntoa(addr), message);

	return true;
}

int tcp_dd_discovery(struct tcp_dd_discovery_client *client, void *data)
{
	client->private_data = data;
	client->client.handler = tcp_dd_discovery_handler;

	if (client->handler == NULL) {
		client->handler = tcp_dd_discovery_handler_dummy;
	}

	return tcp_discovery_client_run(&client->client, client);
}

int tcp_dd_install(struct network_url *url, const char *pathname)
{
	int ret;
	size64_t size;
	struct network_client client;
	struct tcp_dd_package package;

	size = file_get_size(pathname);
	if (size == 0) {
		pr_red_info("no data send: %s", pathname);
		return -ENOENT;
	}

	ret = network_client_open(&client, url, CAVAN_NET_FLAG_TALK | CAVAN_NET_FLAG_SYNC | CAVAN_NET_FLAG_WAIT | CAVAN_NET_FLAG_NODELAY);
	if (ret < 0) {
		pr_red_info("network_client_open");
		return ret;
	}

	package.value64 = size;

	ret = tcp_dd_send_request2(&client, &package, TCP_DD_INSTALL, TCP_DD_PKG_HEADER_LEN + sizeof(package.value64), 0);
	if (ret < 0) {
		pr_red_info("tcp_dd_send_request: %d", ret);
		goto out_network_client_close;
	}

	pr_info("send: %s", pathname);

	ret = network_client_send_file2(&client, pathname, size);
	if (ret < 0) {
		pr_red_info("network_client_send_file2: %d", ret);
		goto out_network_client_close;
	}

	pr_info("install: %s", pathname);

	ret = tcp_dd_recv_response(&client);
	if (ret < 0) {
		pr_red_info("Failed");
	} else {
		pr_green_info("OK");
	}

out_network_client_close:
	network_client_close(&client);
	return ret;
}
