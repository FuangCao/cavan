#include <cavan.h>
#include <cavan/text.h>
#include <linux/kd.h>
#include <termios.h>

#define MAX_BUFF_LEN	KB(4)

FILE *console_fp;
static struct termios tty_attr;

int cavan_tty_set_attr(int fd, int action, struct termios *attr)
{
	int ret;

	if (!isatty(fd)) {
		return 0;
	}

	ret = tcsetattr(fd, action, attr);
	if (ret < 0) {
		return ret;
	}

#ifndef CONFIG_ANDROID
	tcdrain(fd);
#endif

	tcflush(fd, TCIOFLUSH);
	tcflow(fd, TCOON);

	return 0;
}

int cavan_tty_set_mode(int fd, int mode, struct termios *attr_bak)
{
	int ret;
	struct termios attr;

	if (!isatty(fd)) {
		return 0;
	}

	if (attr_bak == NULL) {
		attr_bak = &tty_attr;
	}

	ret = cavan_tty_get_attr(fd, attr_bak);
	if (ret < 0) {
		pr_red_info("cavan_tty_get_attr");
		return ret;
	}

	attr = *attr_bak;

	switch (mode) {
	case 2:
		attr.c_iflag = BRKINT | IXON;
		attr.c_oflag = 0;
		attr.c_cflag &= ~PARENB;
		attr.c_cflag |= CS8;
		attr.c_lflag = ISIG;
		attr.c_cc[VINTR] = 030;
		attr.c_cc[VQUIT] = _POSIX_VDISABLE;
		attr.c_cc[VMIN] = 1;
		attr.c_cc[VTIME] = 1;
		return cavan_tty_set_attr(fd, TCSADRAIN, &attr);

	case 1:
	case 3:
		attr.c_iflag = IGNBRK;
		if (mode == 3) {
			attr.c_iflag |= IXOFF;
		}

		attr.c_lflag &= ~(ECHO | ICANON | ISIG);
		attr.c_oflag = 0;
		attr.c_cflag &= ~(PARENB);
		attr.c_cflag &= ~(CSIZE);
		attr.c_cflag |= CS8;
		attr.c_cc[VMIN] = 1;
		attr.c_cc[VTIME] = 1;
		return cavan_tty_set_attr(fd, TCSADRAIN, &attr);

	case 4:
		attr.c_lflag &= ~(ICANON | ECHO | ISIG);
		attr.c_cflag |= (CREAD | CLOCAL);
		attr.c_cflag &= ~(CSTOPB | PARENB | CRTSCTS);
		attr.c_cflag &= ~(CBAUD | CSIZE) ;
		attr.c_cflag |= (B115200 | CS8);
		return cavan_tty_set_attr(fd, TCSANOW, &attr);

	case 5:
		attr.c_lflag = 0;
		attr.c_cc[VTIME] = 0;
		attr.c_cc[VMIN] = 1;
		return cavan_tty_set_attr(fd, TCSANOW, &attr);

	default:
		pr_red_info("invalid mode %d", mode);
		return -EINVAL;
	}
}

int cavan_tty_attr_restore(int fd, struct termios *attr)
{
	if (attr == NULL) {
		attr = &tty_attr;
	}

	return cavan_tty_set_attr(fd, TCSADRAIN, attr);
}

int cavan_has_char(long sec, long usec)
{
	fd_set readset;
	struct timeval timeout;

	FD_ZERO(&readset);
	FD_SET(STDIN_FILENO, &readset);

	timeout.tv_sec = sec;
	timeout.tv_usec = usec;

	return select(STDIN_FILENO + 1, &readset, NULL, NULL, &timeout) > 0;
}

int cavan_getchar_timed(long sec, long usec)
{
	if (cavan_has_char(sec, usec)) {
		return getchar();
	}

	return -1;
}

char *sprint(char *buff, size_t size, const char *fmt, ...)
{
	char *ret_buff;
	va_list ap;

	va_start(ap, fmt);
	ret_buff = vformat_text(buff, size, fmt, ap);
	va_end(ap);

	return ret_buff;
}

char *sprintln(char *buff, size_t size, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	buff = vformat_text(buff, size, fmt, ap);
	va_end(ap);

	*buff++ = '\n';
	buff[0] = 0;

	return buff;
}

#if 0 // CONFIG_ANDROID
void print_ntext(const char *text, size_t size)
{
	char buff[size + 1];

	mem_copy(buff, text, size);
	buff[size] = 0;

	__android_log_write(ANDROID_LOG_DEBUG, LOG_TAG, buff);
}
#else
void print_ntext(const char *text, size_t size)
{
	if (console_fp) {
		fwrite(text, 1, size, console_fp);
		fflush(console_fp);
	}

	fwrite(text, 1, size, stdout);
	fflush(stdout);
}
#endif

void print_buffer(const char *buff, size_t size)
{
	print("buff = ");
	print_ntext(buff, size);
	print_char('\n');
}

void vprint(const char *fmt, va_list ap)
{
	char *p;
	char buff[PRINT_BUFFER_LEN];

	 p = vformat_text(buff, sizeof(buff), fmt, ap);
	print_ntext(buff, p - buff);
}

void print(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vprint(fmt, ap);
	va_end(ap);
}

void vprintln(const char *fmt, va_list ap)
{
	char *tmp;
	char buff[PRINT_BUFFER_LEN];

	tmp = vformat_text(buff, sizeof(buff), fmt, ap);
	*tmp++ = '\n';
	*tmp = 0;
	print_text(buff);
}

void println(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vprintln(fmt, ap);
	va_end(ap);
}

void print_bit_mask(u64 value, const char *prompt, ...)
{
	char buff[1024];
	char *p, *p_end;

	p = buff;
	p_end = buff + sizeof(buff) - 1;

	if (prompt) {
		va_list ap;

		va_start(ap, prompt);
		p += vsnprintf(p, p_end - p, prompt, ap);
		va_end(ap);
	}

	p = value2bitlist(value, p, p_end - p, " | ");
	*p++ = '\n';

	print_ntext(buff, p - buff);
}

void print_to(int x, int y, const char *fmt, ...)
{
	va_list ap;

	set_console_cursor(y, x);

	va_start(ap, fmt);
	vprint(fmt, ap);
	va_end(ap);
}

void print_to_row(int row, const char *fmt, ...)
{
	va_list ap;

	set_console_row(row);

	va_start(ap, fmt);
	vprint(fmt, ap);
	va_end(ap);
}

void print_to_col(int col, const char *fmt, ...)
{
	va_list ap;

	set_console_col(col);

	va_start(ap, fmt);
	vprint(fmt, ap);
	va_end(ap);
}

void println_to(int x, int y, const char *fmt, ...)
{
	va_list ap;

	set_console_cursor(y, x);

	va_start(ap, fmt);
	vprintln(fmt, ap);
	va_end(ap);
}

void println_to_row(int row, const char *fmt, ...)
{
	va_list ap;

	set_console_row(row);

	va_start(ap, fmt);
	vprintln(fmt, ap);
	va_end(ap);
}

void println_to_col(int col, const char *fmt, ...)
{
	va_list ap;

	set_console_col(col);

	va_start(ap, fmt);
	vprintln(fmt, ap);
	va_end(ap);
}

void vprint_color_text(int color, const char *fmt, va_list ap)
{
	set_console_font_simple(color);
	vprintln(fmt, ap);
	set_default_font();
}

void print_color_text(int color, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vprint_color_text(color, fmt, ap);
	va_end(ap);
}

void print_sep(size_t size)
{
	char buff[size + 1];

	memset(buff, '=', size);

	buff[size] = '\n';

	print_ntext(buff, sizeof(buff));
}

int show_file(const char *dev_name, u64 start, u64 size)
{
	int fd;
	int ret;
	char buff[size];

#if __WORDSIZE == 64
	println("start = %ld, size = %ld", start, size);
#else
	println("start = %Ld, size = %Ld", start, size);
#endif

	fd = open(dev_name, O_RDONLY | O_BINARY);
	if (fd == -1) {
		pr_err_info("open");
		ret = -1;
		goto out_return;
	}

	ret = lseek(fd, start, SEEK_SET);
	if (ret == -1) {
		pr_err_info("lseek");
		goto out_close_file;
	}

	ret = read(fd, buff, size);
	if (ret == -1) {
		pr_err_info("read");
		goto out_close_file;
	}

	print_ntext(buff, ret);
	ret = 0;

out_close_file:
	close(fd);
out_return:
	return ret;
}

int cat_file(const char *filename)
{
	int ret;
	int fd;
	char buff[MAX_BUFF_LEN];

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		pr_err_info("open file \"%s\"", filename);
		return fd;
	}

	while (1) {
		ssize_t readlen;

		readlen = read(fd, buff, sizeof(buff));
		if (readlen < 0) {
			pr_err_info("read");
			ret = readlen;
			goto out_close_fd;
		}

		if (readlen == 0) {
			break;
		}

		print_ntext(buff, readlen);
	}

	print_char('\n');
	ret = 0;

out_close_fd:
	close(fd);
	return ret;
}

void print_title(const char *title, char sep, size_t size)
{
	size_t i;

	for (i = 0; i < size; i++) {
		print_char(sep);
	}

	print_char(' ');

	while (title[0]) {
		print_char(*title++);
	}

	print_char(' ');

	for (i = 0; i < size; i++) {
		print_char(sep);
	}

	print_char('\n');
}

void print_mem(const char *promp, const u8 *mem, size_t size, ...)
{
	char *p;
	char buff[size * 2 + 3];

	p = mem_tostring(mem, size, buff, sizeof(buff));

	if (promp) {
		va_list ap;

		va_start(ap, size);
		vprint(promp, ap);
		va_end(ap);
	}

	*p = '\n';
	print_ntext(buff, sizeof(buff));
}

void print_error_base(const char *fmt, ...)
{
	va_list ap;
	char buff[1024];

	va_start(ap, fmt);
	vformat_text(buff, sizeof(buff), fmt, ap);
	va_end(ap);

	if (errno == 0) {
		print_string(buff);
	} else {
		println("%s: %s[%d]", buff, strerror(errno), errno);
	}
}

int open_console(const char *dev_path)
{
	FILE *fp;

	if (dev_path == NULL || dev_path[0] == 0) {
		dev_path = DEFAULT_CONSOLE_DEVICE;
	}

	fp = fopen(dev_path, "w");
	if (fp == NULL) {
		pr_err_info("fopen");
		return -1;
	}

	if (console_fp) {
		close_console();
	}

	console_fp = fp;

	set_default_font();
	fswitch2text_mode(fileno(fp));

	return 0;
}

void close_console(void)
{
	if (console_fp == NULL) {
		return;
	}

	set_default_font();
	fswitch2graph_mode(fileno(console_fp));

	fflush(console_fp);
	fclose(console_fp);
	console_fp = NULL;
}

void fflush_console(void)
{
	if (console_fp) {
		fflush(console_fp);
	}

	fflush(stdout);
}

void show_menu(int x, int y, const char *menu[], int count, int select)
{
	int i;

	set_console_row(y);

	for (i = 0; i < select; i++) {
		print_string_to_col(x, menu[i]);
	}

	set_console_col(x);
	set_console_font(FONT_BOLD, FONT_BROWN_FOREGROUND, FONT_BLUE_BACKGROUND);
	print_text(menu[i]);
	set_default_font();
	print_char('\n');

	for (i++; i < count; i++) {
		print_string_to_col(x, menu[i]);
	}
}

int get_menu_select(const char *input_dev_path, const char *menu[], int count)
{
	int fd;
	int select;

	fd = open(input_dev_path, O_RDONLY | O_BINARY);
	if (fd < 0) {
		pr_err_info("open");
		return -1;
	}

	select = 0;

	while (1) {
		show_menu(10, 10, menu, count, select);
	}

	return select;
}

int fswitch2text_mode(int tty_fd)
{
	return ioctl(tty_fd, KDSETMODE, KD_TEXT);
}

int switch2text_mode(const char *tty_path)
{
	int ret;
	int tty_fd;

	tty_fd = open(tty_path, 0);
	if (tty_fd < 0) {
		pr_err_info("open tty device \"%s\" failed", tty_path);
		return -1;
	}

	ret = fswitch2text_mode(tty_fd);
	if (ret < 0) {
		pr_err_info("switch to text mode failed");
	}

	close(tty_fd);

	return ret;
}

int fswitch2graph_mode(int tty_fd)
{
	return ioctl(tty_fd, KDSETMODE, KD_GRAPHICS);
}

int switch2graph_mode(const char *tty_path)
{
	int ret;
	int tty_fd;

	tty_fd = open(tty_path, 0);
	if (tty_fd < 0) {
		pr_err_info("open tty device \"%s\" failed", tty_path);
		return -1;
	}

	ret = fswitch2graph_mode(tty_fd);
	if (ret < 0) {
		pr_err_info("switch to graph mode failed");
	}

	close(tty_fd);

	return ret;
}

void show_author_info(void)
{
	println("Fuang.Cao <cavan.cfa@gmail.com>");
	println("Fuang.Cao <fuang.cao@eavoo.com>");
	println("Fuang.Cao <caofuang@huamobile.com>");
	println("Fuang.Cao <caofuang@hipad.hk>");
	println("https://github.com/FuangCao/cavan.git");
}

bool cavan_get_choose_yesno(const char *prompt, bool def_choose, int timeout_ms)
{
	char buff[8];
	ssize_t rdlen;
	const char *option;

	if (prompt == NULL || timeout_ms == 0) {
		return def_choose;
	}

	option = def_choose ? "Y/n" : "y/N";

	while (1) {
		print("%s [%s]: ", prompt, option);

		if (timeout_ms > 0) {
			rdlen = file_read_timeout(stdin_fd, buff, sizeof(buff), timeout_ms);
		} else {
			rdlen = read(stdin_fd, buff, sizeof(buff));
		}

		if (rdlen <= 0) {
			if (def_choose) {
				println("Y");
				return true;
			}

			println("N");
			return false;
		}

		switch (buff[0]) {
		case '\n':
			return def_choose;

		case 'y':
		case 'Y':
			if (rdlen == 1 || buff[1] == '\n') {
				return true;
			}

		case 'n':
		case 'N':
			if (rdlen == 1 || buff[1] == '\n') {
				return false;
			}
		}

		pr_red_info("Please input [Y/y/N/n/Enter]");
	}
}

bool cavan_get_choose_yesno_format(bool def_choose, int timeout_ms, const char *format, ...)
{
	va_list ap;
	char buff[1024];

	va_start(ap, format);
	vsnprintf(buff, sizeof(buff), format, ap);
	va_end(ap);

	return cavan_get_choose_yesno(buff, def_choose, timeout_ms);
}

const char *cavan_get_temp_path(void)
{
	int i;
	const char *paths[] = { "/tmp", "/data/local/tmp", "/dev", "/data", "/cache" };

	for (i = 0; i < NELEM(paths); i++) {
		if (file_access_e(paths[i])) {
			return paths[i];
		}
	}

	return paths[0];
}

const char *cavan_build_temp_path(const char *filename, char *buff, size_t size)
{
	const char *temp;

	temp = cavan_get_temp_path();
	text_path_cat(buff, size, temp, filename);

	return buff;
}
