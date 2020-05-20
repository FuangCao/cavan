#include <cavan.h>
#include <cavan/text.h>
#include <cavan/timer.h>
#include <cavan/thread.h>
#include <linux/kd.h>
#include <termios.h>

#define MAX_BUFF_LEN	KB(4)

cavan_lock_t cavan_stdout_lock = CAVAN_LOCK_INITIALIZER;
const char *cavan_line_end = "\n";

static int console_fd = -1;
static struct termios tty_attr;

static FILE *cavan_async_stdout;
static int cavan_async_stdout_pipefd[2];

cavan_stdio_function_declarer(tty, "/dev/tty");
cavan_stdio_function_declarer(kmsg, "/dev/kmsg");

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

	cavan_stdio_fflush();
	msleep(1);

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
	case CAVAN_TTY_MODE_DATA:
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

	case CAVAN_TTY_MODE_AT:
		attr.c_lflag &= ~(ICANON | ECHO | ISIG);
		attr.c_cflag |= (CREAD | CLOCAL);
		attr.c_cflag &= ~(CSTOPB | PARENB | CRTSCTS);
		attr.c_cflag &= ~(CBAUD | CSIZE) ;
		attr.c_cflag |= (B115200 | CS8);
		return cavan_tty_set_attr(fd, TCSANOW, &attr);

	case CAVAN_TTY_MODE_SSH:
		attr.c_lflag = 0;
		attr.c_cc[VTIME] = 0;
		attr.c_cc[VMIN] = 1;
		return cavan_tty_set_attr(fd, TCSANOW, &attr);

	case CAVAN_TTY_MODE_CMDLINE:
		attr.c_lflag = ISIG;
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

	return cavan_tty_set_attr(fd, TCSANOW, attr);
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

#if 0 // CONFIG_ANDROID
void print_ntext(const char *text, size_t size)
{
	char buff[size + 1];

	mem_copy(buff, text, size);
	buff[size] = 0;

	__android_log_write(ANDROID_LOG_DEBUG, LOG_TAG, buff);
}
#else
int print_ntext(const char *text, size_t size)
{
	int ret = 0;

	cavan_lock_acquire(&cavan_stdout_lock);

	if (console_fd >= 0) {
		ret |= write(console_fd, text, size);
		ret |= fsync(console_fd);
	}

	ret |= write(stdout_fd, text, size);
	ret |= fsync(stdout_fd);

	cavan_lock_release(&cavan_stdout_lock);

	return ret;
}
#endif

void time_println(const char *fmt, ...)
{
	char *p, *p_end;
	char buff[4096];
	va_list ap;
	struct timespec time;
	struct tm date;

	clock_gettime_real(&time);
	localtime_r(&time.tv_sec, &date);

	p = buff;
	p_end = p + sizeof(buff);


	p += cavan_strftime2(&date, p, p_end - p);
	p += snprintf(p, p_end - p, ".%03d", CAST_INT(time.tv_nsec / 1000000 % 1000));

	if (p < p_end) {
		*p++ = ' ';
	}

	va_start(ap, fmt);
	p += vsnprintf(p, p_end - p, fmt, ap);
	va_end(ap);

	if (p < p_end) {
		*p++ = '\n';
	}

	print_ntext(buff, p - buff);
}

static void *cavan_async_stdout_thread(void *data)
{
	int fd = cavan_async_stdout_pipefd[0];

	while (1) {
		ssize_t rdlen;
		char buff[1024];

		rdlen = read(fd, buff, sizeof(buff));
		if (rdlen < 0) {
			break;
		}

		if (print_ntext(buff, rdlen) < 0) {
			break;
		}
	}

	fclose(cavan_async_stdout);
	cavan_async_stdout = NULL;

	close(cavan_async_stdout_pipefd[0]);
	close(cavan_async_stdout_pipefd[1]);

	return NULL;
}

int cavan_async_vprintf(const char *fmt, va_list ap)
{
	int ret;

	if (unlikely(cavan_async_stdout == NULL)) {
		int ret;
		pthread_t thread;

		ret = pipe(cavan_async_stdout_pipefd);
		if (ret < 0) {
			return ret;
		}

		ret = cavan_pthread_create(&thread, cavan_async_stdout_thread, NULL, false);
		if (ret < 0) {
			close(cavan_async_stdout_pipefd[0]);
			close(cavan_async_stdout_pipefd[1]);
			return ret;
		}

		cavan_async_stdout = fdopen(cavan_async_stdout_pipefd[1], "w");
		if (cavan_async_stdout == NULL) {
			cavan_pthread_kill(thread);
			close(cavan_async_stdout_pipefd[0]);
			close(cavan_async_stdout_pipefd[1]);
			return -EFAULT;
		}

		setlinebuf(cavan_async_stdout);
	}

	ret = vfprintf(cavan_async_stdout, fmt, ap);
	fflush(cavan_async_stdout);

	return ret;
}

int cavan_async_printf(const char *fmt, ...)
{
	int ret;
	va_list ap;

	va_start(ap, fmt);
	ret = cavan_async_vprintf(fmt, ap);
	va_end(ap);

	return ret;
}

int cavan_async_fflush(void)
{
	return fflush(cavan_async_stdout);
}

void print_buffer(const char *buff, size_t size)
{
	print("buff = ");
	print_ntext(buff, size);
	print_char('\n');
}

int vprint(const char *fmt, va_list ap)
{
	int length;
	char *buff = alloca(PRINT_BUFFER_LEN);

	if (buff == NULL) {
		return -ENOMEM;
	}

	length = vsnprintf(buff, PRINT_BUFFER_LEN, fmt, ap);

	return print_ntext(buff, length);
}

int print(const char *fmt, ...)
{
	int ret;
	va_list ap;

	va_start(ap, fmt);
	ret = vprint(fmt, ap);
	va_end(ap);

	return ret;
}

int vprintln(const char *fmt, va_list ap)
{
	char *text, *buff = alloca(PRINT_BUFFER_LEN);

	if (buff == NULL) {
		return -ENOMEM;
	}

	text = buff + vsnprintf(buff, PRINT_BUFFER_LEN - 1, fmt, ap);
	*text++ = '\n';
	*text = 0;

	return print_ntext(buff, text - buff);
}

int println(const char *fmt, ...)
{
	int ret;
	va_list ap;

	va_start(ap, fmt);
	ret = vprintln(fmt, ap);
	va_end(ap);

	return ret;
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
	int length;
	char buff[1024];

	va_start(ap, fmt);
	length = vsnprintf(buff, sizeof(buff), fmt, ap);
	va_end(ap);

	if (errno == 0) {
		buff[length++] = '\n';
		buff[length] = 0;
	} else {
		length += snprintf(buff + length, sizeof(buff) - length, ": %s[%d]\n", strerror(errno), errno);
	}

	print_ntext(buff, length);
}

int open_console(const char *dev_path)
{
	int fd;

	if (dev_path == NULL || dev_path[0] == 0) {
		dev_path = DEFAULT_CONSOLE_DEVICE;
	}

	fd = open(dev_path, O_RDWR | O_CLOEXEC | O_CREAT | O_TRUNC, 0777);
	if (fd < 0) {
		pr_err_info("open %s", dev_path);
		return fd;
	}

	close_console();
	console_fd = fd;

	set_default_font();
	fswitch2text_mode(console_fd);

	return 0;
}

void close_console(void)
{
	if (console_fd < 0) {
		return;
	}

	set_default_font();
	fswitch2graph_mode(console_fd);

	close(console_fd);
	console_fd = -1;
}

void fflush_console(void)
{
	if (console_fd >= 0) {
		fsync(console_fd);
	}

	fsync(stdout_fd);
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
			break;

		case 'n':
		case 'N':
			if (rdlen == 1 || buff[1] == '\n') {
				return false;
			}
			break;
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

int cavan_stdio_redirect1(int ttyfds[3])
{
	int i;
	int ret;

	cavan_stdio_fflush();

	for (i = 0; i < 3; i++) {
		if (ttyfds[i] < 0) {
			continue;
		}

		ret = dup2(ttyfds[i], i);
		if (ret < 0) {
			pd_error_info("dup2 stdio %d", i);
			return ret;
		}
	}

	for (i = 0; i < 3; i++) {
		int j;

		if (ttyfds[i] < 0) {
			continue;
		}

		j = i;

		do {
			if (--j < 0) {
				close(ttyfds[i]);
				break;
			}
		} while (ttyfds[i] != ttyfds[j]);
	}

	cavan_stdio_setlinebuf();

	return 0;
}

int cavan_stdio_redirect2(int fd, int flags)
{
	int i;
	int ttyfds[3];

	for (i = 0; i < 3; i++) {
		if (flags & (1 << i)) {
			ttyfds[i] = fd;
		} else {
			ttyfds[i] = -1;
		}
	}

	return cavan_stdio_redirect1(ttyfds);
}

int cavan_stdio_redirect3(const char *pathname, int flags)
{
	int fd;
	int open_flags;

	pd_bold_info("pathname = %s, flags = 0x%02x", pathname, flags);

	if ((flags & 0x01)) {
		if ((flags & 0x06)) {
			open_flags = O_RDWR;
		} else {
			open_flags = O_RDONLY;
		}
	} else {
		open_flags = O_WRONLY;
	}

	fd = open(pathname, open_flags | O_CREAT | O_TRUNC, 0777);
	if (fd < 0) {
		pd_error_info("open file `%s' failed", pathname);
		return fd;
	}

	return cavan_stdio_redirect2(fd, flags);
}

int cavan_stdout_write_line(const char *line, int length)
{
	int ret = 0;

	cavan_lock_acquire(&cavan_stdout_lock);

	if (line != NULL && length > 0) {
		ret |= write(stdout_fd, line, length);
	}

	ret |= write(stdout_fd, "\n", 1);

	cavan_lock_release(&cavan_stdout_lock);

	return ret;
}

int nsleep(ulong nseconds)
{
	struct timespec ts = {
		.tv_sec = (slong) (nseconds / 1000000000ul),
		.tv_nsec = (slong) (nseconds % 1000000000ul),
	};

	return nanosleep(&ts, NULL);
}

int msleep(ulong mseconds)
{
	struct timespec ts = {
		.tv_sec = (slong) (mseconds / 1000),
		.tv_nsec = (slong) ((mseconds % 1000) * 1000000ul),
	};

	return nanosleep(&ts, NULL);
}

speed_t serial_rate2speed(int rate)
{
	switch (rate)
	{
	case 0:
		return B0;
	case 50:
		return B50;
	case 75:
		return B75;
	case 110:
		return B110;
	case 134:
		return B134;
	case 150:
		return B150;
	case 200:
		return B200;
	case 300:
		return B300;
	case 600:
		return B600;
	case 1200:
		return B1200;
	case 1800:
		return B1800;
	case 2400:
		return B2400;
	case 4800:
		return B4800;
	case 9600:
		return B9600;
	case 19200:
		return B19200;
	case 38400:
		return B38400;
	case 57600:
		return B57600;
	case 115200:
		return B115200;
	case 230400:
		return B230400;
	case 460800:
		return B460800;
	case 500000:
		return B500000;
	case 576000:
		return B576000;
	case 921600:
		return B921600;
	case 1000000:
		return B1000000;
	case 1152000:
		return B1152000;
	case 1500000:
		return B1500000;
	case 2000000:
		return B2000000;
	case 2500000:
		return B2500000;
	case 3000000:
		return B3000000;
	case 3500000:
		return B3500000;
	case 4000000:
		return B4000000;
	}

	return B115200;
}

int serial_open(const char *pathname, int rate, const char *line_end)
{
	char buff[1024];
	int ret;
	int fd;

	if (pathname == NULL) {
		int index = 0;

		pathname = buff;

		while (1) {
			if (index > 100) {
				pr_err_info("No serial device found");
				return -ENOENT;
			}

			snprintf(buff, sizeof(buff), "/dev/ttyUSB%d", index);

			fd = open(pathname, O_RDWR | O_NOCTTY);
			if (fd < 0) {
				if (errno != ENOENT) {
					pr_err_info("open: %s", pathname);
					return fd;
				}

				index++;
			} else {
				break;
			}
		}
	} else {
		fd = open(pathname, O_RDWR | O_NOCTTY);
		if (fd < 0) {
			pr_err_info("open: %s", pathname);
			return fd;
		}
	}

	println("serial_open: pathname = %s, rate = %d", pathname, rate);

    if (isatty(fd)) {
        struct termios attr;
        speed_t speed;

        ret = tcgetattr(fd, &attr);
        if (ret < 0) {
            pr_err_info("tcgetattr");
            goto out_close_fd;
        }

        speed = serial_rate2speed(rate);

        println("rate = %d, speed = %o", rate, speed);

        ret = cfsetispeed(&attr, speed);
        if (ret < 0) {
            pr_err_info("cfsetispeed");
            goto out_close_fd;
        }

        ret = cfsetospeed(&attr, speed);
        if (ret < 0) {
            pr_err_info("cfsetospeed");
            goto out_close_fd;
        }

		if (line_end && line_end[0] == '\r' && line_end[1] != '\n') {
			attr.c_iflag &= ~(IXON);
			attr.c_iflag |= ICRNL;
		} else {
			attr.c_iflag &= ~(ICRNL | IXON);
		}

		attr.c_cflag |= CLOCAL | CREAD;
		attr.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
		attr.c_oflag &= ~OPOST;

		ret = tcsetattr(fd, TCSANOW, &attr);
		if (ret < 0) {
			pr_err_info("tcsetattr");
			goto out_close_fd;
		}
    }

	return fd;

out_close_fd:
	close(fd);
	return ret;
}

void cavan_serial_init(struct cavan_serial_desc *serial)
{
	cavan_block_cache_init(&serial->cache);
	serial->line_end = "\n";
}

void cavan_serial_deinit(struct cavan_serial_desc *serial)
{
	cavan_block_cache_deinit(&serial->cache);
}

void cavan_serial_read_loop(struct cavan_serial_desc *serial)
{
	char buff[4096];

	while (1) {
		int length = read(serial->fd, buff, sizeof(buff));

		if (length < 0) {
			break;
		}

		if (serial->paused) {
			cavan_block_cache_write(&serial->cache, buff, length);
		} else {
			fwrite(buff, 1, length, stdout);
			fflush(stdout);
		}
	}
}

static void *cavan_serial_read_thread(void *data)
{
	cavan_serial_read_loop((struct cavan_serial_desc *) data);
	return NULL;
}

int cavan_serial_cmdline(struct cavan_serial_desc *serial)
{
	int line_end_size = strlen(serial->line_end);
	struct termios attr_bak;
	int ret;

	ret = cavan_tty_set_mode(stdin_fd, CAVAN_TTY_MODE_SSH, &attr_bak);
	if (ret < 0) {
		pr_red_info("cavan_tty_set_mode");
		return ret;
	}

	cavan_pthread_run(cavan_serial_read_thread, serial);

	while (1) {
		int ret;
		int value;

		value = getchar();
		if (value < 0) {
			pr_err_info("getchar: %d", value);
			break;
		}

		switch (value)
		{
		case 0x1C:
			serial->paused = true;
			println("Press any key to continue");

			while (1) {
				value = getchar();
				if (value < 0 || value == 0x03) {
					goto out_cavan_tty_attr_restore;
				}

				if (value != 0x1C) {
					break;
				}
			}

			while (1) {
				char buff[1024];
				int length;

				length = cavan_block_cache_read(&serial->cache, buff, sizeof(buff));
				if (length == 0) {
					break;
				}

				fwrite(buff, 1, length, stdout);
			}

			serial->paused = false;
			fflush(stdout);
			break;

		case '\n':
			ret = write(serial->fd, serial->line_end, line_end_size);
			if (ret < 0) {
				pr_err_info("write: %d", ret);
				goto out_cavan_tty_attr_restore;
			}
			break;

		default:
			ret = write(serial->fd, &value, 1);
			if (ret < 0) {
				pr_err_info("write: %d", ret);
				goto out_cavan_tty_attr_restore;
			}
			break;
		}

		fsync(serial->fd);
	}

out_cavan_tty_attr_restore:
	cavan_tty_attr_restore(stdin_fd, &attr_bak);

	return 0;
}
