#include <cavan.h>
#include <cavan/text.h>
#include <linux/kd.h>
#include <termios.h>

#define MAX_BUFF_LEN	MB(1)

static struct termios tty_old_attr;
static int old_tty_fd = -1;
FILE *console_fp;

int backup_tty_attr(int fd, int force)
{
	int ret;

	if (fd == old_tty_fd && force == 0)
	{
		return 0;
	}

	ret = tcgetattr(fd, &tty_old_attr);
	if (ret < 0)
	{
		return ret;
	}

	old_tty_fd = fd;

	return 0;
}

int restore_tty_attr(int fd)
{
	if (fd < 0)
	{
		fd = old_tty_fd;
	}

	if (fd < 0)
	{
		return -EINVAL;
	}

#ifndef CONFIG_BUILD_FOR_ANDROID
	tcdrain(fd);
#endif

	tcflush(fd, TCIOFLUSH);
	tcflow(fd, TCOON);

	return tcsetattr(fd, TCSADRAIN, &tty_old_attr);
}

int set_tty_mode(int fd, int mode)
{
	int ret;

	struct termios tty_attr;

	switch (mode)
	{
	case 2:
		ret = backup_tty_attr(fd, 0);
		if (ret < 0)
		{
			return ret;
		}

		tty_attr = tty_old_attr;
		tty_attr.c_iflag = BRKINT | IXON;
		tty_attr.c_oflag = 0;
		tty_attr.c_cflag &= ~PARENB;
		tty_attr.c_cflag |= CS8;
		tty_attr.c_lflag = ISIG;
		tty_attr.c_cc[VINTR] = 030;
		tty_attr.c_cc[VQUIT] = _POSIX_VDISABLE;
		tty_attr.c_cc[VMIN] = 1;
		tty_attr.c_cc[VTIME] = 1;

		return tcsetattr(fd,TCSADRAIN, &tty_attr);

	case 1:
	case 3:
		ret = backup_tty_attr(fd, 0);
		if (ret < 0)
		{
			return ret;
		}

		tty_attr = tty_old_attr;
		tty_attr.c_iflag = IGNBRK;
		if (mode == 3)
		{
			tty_attr.c_iflag |= IXOFF;
		}

		tty_attr.c_lflag &= ~(ECHO | ICANON | ISIG);
		tty_attr.c_oflag = 0;
		tty_attr.c_cflag &= ~(PARENB);
		tty_attr.c_cflag &= ~(CSIZE);
		tty_attr.c_cflag |= CS8;
		tty_attr.c_cc[VMIN] = 1;
		tty_attr.c_cc[VTIME] = 1;

		return tcsetattr(fd, TCSADRAIN, &tty_attr);

	case 4:
		ret = backup_tty_attr(fd, 0);
		if (ret < 0)
		{
			return ret;
		}

		tty_attr = tty_old_attr;
		tty_attr.c_lflag &= ~(ICANON | ECHO | ISIG);
		tty_attr.c_cflag |= (CREAD | CLOCAL);
		tty_attr.c_cflag &= ~(CSTOPB | PARENB | CRTSCTS);
		tty_attr.c_cflag &= ~(CBAUD | CSIZE) ;
		tty_attr.c_cflag |= (B115200 | CS8);

		return tcsetattr(fd, TCSANOW, &tty_attr);

	case 0:
		return restore_tty_attr(fd);

	default:
		return -EINVAL;
	}

	return 0;
}

int has_char(long sec, long usec)
{
	fd_set readset;
	struct timeval timeout;

	FD_ZERO(&readset);
	FD_SET(STDIN_FILENO, &readset);

	timeout.tv_sec = sec;
	timeout.tv_usec = usec;

	return select(STDIN_FILENO + 1, &readset, NULL, NULL, &timeout) > 0;
}

int timeout_getchar(long sec, long usec)
{
	if (has_char(sec, usec))
	{
		return getchar();
	}

	return -1;
}

char *sprint(char *buff, const char *fmt, ...)
{
	char *ret_buff;
	va_list ap;

	va_start(ap, fmt);
	ret_buff = vformat_text(buff, fmt, ap);
	va_end(ap);

	return ret_buff;
}

char *sprintln(char *buff, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	buff = vformat_text(buff, fmt, ap);
	va_end(ap);

	*buff++ = '\n';
	buff[0] = 0;

	return buff;
}

void print_ntext(const char *text, size_t size)
{
	if (console_fp)
	{
		fwrite(text, 1, size, console_fp);
		fflush(console_fp);
	}

	fwrite(text, 1, size, stdout);
	fflush(stdout);
}

void print_buffer(const char *buff, size_t size)
{
	print("buff = ");
	print_ntext(buff, size);
	print_char('\n');
}

void vprint(const char *fmt, va_list ap)
{
	char buff[PRINT_BUFFER_LEN];

	vformat_text(buff, fmt, ap);
	print_text(buff);
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

	tmp = vformat_text(buff, fmt, ap);
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
	if (fd == -1)
	{
		print_error("open");
		ret = -1;
		goto out_return;
	}

	ret = lseek(fd, start, SEEK_SET);
	if (ret == -1)
	{
		print_error("lseek");
		goto out_close_file;
	}

	ret = read(fd, buff, size);
	if (ret == -1)
	{
		print_error("read");
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
	if (fd < 0)
	{
		error_msg("open file \"%s\"", filename);
		return fd;
	}

	while (1)
	{
		ssize_t readlen;

		readlen = read(fd, buff, sizeof(buff));
		if (readlen < 0)
		{
			print_error("read");
			ret = readlen;
			goto out_close_fd;
		}

		if (readlen == 0)
		{
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

	for (i = 0; i < size; i++)
	{
		print_char(sep);
	}

	print_char(' ');

	while (title[0])
	{
		print_char(*title++);
	}

	print_char(' ');

	for (i = 0; i < size; i++)
	{
		print_char(sep);
	}

	print_char('\n');
}

void print_mem(const void *mem, size_t size)
{
	const u8 *p, *endp;

	for (p = mem, endp = p + size; p < endp; p++)
	{
		print("%02x", *p);
	}

	print_char('\n');
}

void print_error_base(const char *fmt, ...)
{
	va_list ap;
	char buff[1024];

	va_start(ap, fmt);
	vformat_text(buff, fmt, ap);
	va_end(ap);

	if (errno == 0)
	{
		print_string(buff);
	}
	else
	{
		println("%s: %s[%d]", buff, strerror(errno), errno);
	}
}

int open_console(const char *dev_path)
{
	FILE *fp;

	if (dev_path == NULL || dev_path[0] == 0)
	{
		dev_path = DEFAULT_CONSOLE_DEVICE;
	}

	fp = fopen(dev_path, "w");
	if (fp == NULL)
	{
		print_error("fopen");
		return -1;
	}

	if (console_fp)
	{
		close_console();
	}

	console_fp = fp;

	set_default_font();
	fswitch2text_mode(fileno(fp));

	return 0;
}

void close_console(void)
{
	if (console_fp == NULL)
	{
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
	if (console_fp)
	{
		fflush(console_fp);
	}

	fflush(stdout);
}

void show_menu(int x, int y, const char *menu[], int count, int select)
{
	int i;

	set_console_row(y);

	for (i = 0; i < select; i++)
	{
		print_string_to_col(x, menu[i]);
	}

	set_console_col(x);
	set_console_font(FONT_BOLD, FONT_BROWN_FOREGROUND, FONT_BLUE_BACKGROUND);
	print_text(menu[i]);
	set_default_font();
	print_char('\n');

	for (i++; i < count; i++)
	{
		print_string_to_col(x, menu[i]);
	}
}

int get_menu_select(const char *input_dev_path, const char *menu[], int count)
{
	int fd;
	int select;

	fd = open(input_dev_path, O_RDONLY | O_BINARY);
	if (fd < 0)
	{
		print_error("open");
		return -1;
	}

	select = 0;

	while (1)
	{
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
	if (tty_fd < 0)
	{
		print_error("open tty device \"%s\" failed", tty_path);
		return -1;
	}

	ret = fswitch2text_mode(tty_fd);
	if (ret < 0)
	{
		error_msg("switch to text mode failed");
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
	if (tty_fd < 0)
	{
		print_error("open tty device \"%s\" failed", tty_path);
		return -1;
	}

	ret = fswitch2graph_mode(tty_fd);
	if (ret < 0)
	{
		error_msg("switch to graph mode failed");
	}

	close(tty_fd);

	return ret;
}

void show_author_info(void)
{
	println("Fuang.Cao <cavan.cfa@gmail.com>");
	println("Fuang.Cao <fuang.cao@eavoo.com>");
}

