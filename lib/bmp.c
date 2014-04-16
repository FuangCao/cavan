#include <cavan.h>
#include <linux/fb.h>
#include <cavan/bmp.h>
#include <cavan/file.h>
#include <cavan/fb.h>

void bmp_show_file_header(struct bmp_file_header *file_hdr)
{
	print_sep(60);
	println("file_hdr->type = %c%c", file_hdr->type[0], file_hdr->type[1]);
	println("file_hdr->size = %dk", file_hdr->size >> 10);
	println("file_hdr->offset = %d", file_hdr->offset);
}

void bmp_show_info_header(struct bmp_info_header *info_hdr)
{
	print_sep(60);
	println("info_hdr->size = %d", info_hdr->size);
	println("info_hdr->width = %d", info_hdr->width);
	println("info_hdr->height = %d", info_hdr->height);
	println("info_hdr->planes = %d", info_hdr->planes);
	println("info_hdr->bit_count = %d", info_hdr->bit_count);
	println("info_hdr->compress = %d", info_hdr->compress);
	println("info_hdr->size_image = %dk", info_hdr->size_image >> 10);
	println("info_hdr->x_pels_per_meter = %d", info_hdr->x_pels_per_meter);
	println("info_hdr->y_pels_per_meter = %d", info_hdr->y_pels_per_meter);
	println("info_hdr->clr_used = %d", info_hdr->clr_used);
	println("info_hdr->clr_important = %d", info_hdr->clr_important);
}

int bmp_read_file_header(int fd, struct bmp_file_header *file_hdr)
{
	int ret;

	ret = ffile_read(fd, file_hdr, sizeof(*file_hdr));
	if (ret < 0)
	{
		print_error("read");
		return ret;
	}

	if (file_hdr->type[0] != 'B' || file_hdr->type[1] != 'M')
	{
		error_msg("This File Is Not A BMP File");
		return -1;
	}

	bmp_show_file_header(file_hdr);

	return ret;
}

int bmp_read_info_header(int fd, struct bmp_info_header *info_hdr)
{
	int ret;

	ret = ffile_read(fd, info_hdr, sizeof(*info_hdr));
	if (ret < 0)
	{
		print_error("read");
		return ret;
	}

	bmp_show_info_header(info_hdr);

	return ret;
}

static int bmp_555_565(int fd, struct pixel565 *fb, int width, int height, int xres, int yres)
{
	int ret;
	int i, j;
	struct pixel565 *tmp_fb;
	struct pixel555 buff[width];


	for (i = height - 1; i >= 0; i--)
	{
		ret = read(fd, &buff, sizeof(buff));
		if (ret < 0)
		{
			print_error("read");
			return ret;
		}

		for (j = 0; j < width; j++)
		{
			tmp_fb = fb + i * xres + j;

			tmp_fb->red = buff[j].red;
			tmp_fb->green = buff[j].green;
			tmp_fb->green <<= 1;
			tmp_fb->blue = buff[j].blue;
		}
	}

	return 0;
}

static int bmp_555_888(int fd, struct pixel888 *fb, int width, int height, int xres, int yres)
{
	int ret;
	int i, j;
	struct pixel888 *tmp_fb;
	struct pixel555 buff[width];


	for (i = height - 1; i >= 0; i--)
	{
		ret = read(fd, &buff, sizeof(buff));
		if (ret < 0)
		{
			print_error("read");
			return ret;
		}

		for (j = 0; j < width; j++)
		{
			tmp_fb = fb + i * xres + j;

			tmp_fb->red = buff[j].red;
			tmp_fb->red <<= 3;
			tmp_fb->green = buff[j].green;
			tmp_fb->green <<= 3;
			tmp_fb->blue = buff[j].blue;
			tmp_fb->blue <<= 3;
		}
	}

	return 0;
}

static int bmp_555_8888(int fd, struct pixel8888 *fb, int width, int height, int xres, int yres)
{
	int ret;
	int i, j;
	struct pixel8888 *tmp_fb;
	struct pixel555 buff[width];

	for (i = height - 1; i >= 0; i--)
	{
		ret = read(fd, &buff, sizeof(buff));
		if (ret < 0)
		{
			print_error("read");
			return ret;
		}

		for (j = 0; j < width; j++)
		{
			tmp_fb = fb + i * xres + j;

			tmp_fb->red = buff[j].red;
			tmp_fb->red <<= 3;
			tmp_fb->green = buff[j].green;
			tmp_fb->green <<= 3;
			tmp_fb->blue = buff[j].blue;
			tmp_fb->blue <<= 3;
			tmp_fb->transp = 0;
		}
	}

	return 0;
}

static int bmp_565_565(int fd, struct pixel565 *fb, int width, int height, int xres, int yres)
{
	int ret;
	int i, j;
	struct pixel565 buff[width];

	for (i = height - 1; i >= 0; i--)
	{
		ret = read(fd, &buff, sizeof(buff));
		if (ret < 0)
		{
			print_error("read");
			return ret;
		}

		for (j = 0; j < width; j++)
		{
			(fb + i * xres)[j] = buff[j];
		}
	}

	return 0;
}

static int bmp_565_888(int fd, struct pixel888 *fb, int width, int height, int xres, int yres)
{
	int ret;
	int i, j;
	struct pixel888 *tmp_fb;
	struct pixel565 buff[width];


	for (i = height - 1; i >= 0; i--)
	{
		ret = read(fd, &buff, sizeof(buff));
		if (ret < 0)
		{
			print_error("read");
			return ret;
		}

		for (j = 0; j < width; j++)
		{
			tmp_fb = fb + i * xres + j;

			tmp_fb->red = buff[j].red;
			tmp_fb->red <<= 3;
			tmp_fb->green = buff[j].green;
			tmp_fb->green <<= 2;
			tmp_fb->blue = buff[j].blue;
			tmp_fb->blue <<= 3;
		}
	}

	return 0;
}

static int bmp_565_8888(int fd, struct pixel8888 *fb, int width, int height, int xres, int yres)
{
	int ret;
	int i, j;
	struct pixel8888 *tmp_fb;
	struct pixel565 buff[width];


	for (i = height - 1; i >= 0; i--)
	{
		ret = read(fd, &buff, sizeof(buff));
		if (ret < 0)
		{
			print_error("read");
			return ret;
		}

		for (j = 0; j < width; j++)
		{
			tmp_fb = fb + i * xres + j;

			tmp_fb->red = buff[j].red;
			tmp_fb->red <<= 3;
			tmp_fb->green = buff[j].green;
			tmp_fb->green <<= 2;
			tmp_fb->blue = buff[j].blue;
			tmp_fb->blue <<= 3;
			tmp_fb->transp = 0;
		}
	}

	return 0;
}

static int bmp_888_565(int fd, struct pixel565 *fb, int width, int height, int xres, int yres)
{
	int ret;
	int i, j;
	struct pixel565 *tmp_fb;
	struct pixel888 buff[width];


	for (i = height - 1; i >= 0; i--)
	{
		ret = read(fd, &buff, sizeof(buff));
		if (ret < 0)
		{
			print_error("read");
			return ret;
		}

		for (j = 0; j < width; j++)
		{
			tmp_fb = fb + i * xres + j;

			tmp_fb->red = buff[j].red >> 3;
			tmp_fb->green = buff[j].green >> 2;
			tmp_fb->blue = buff[j].blue >> 3;
		}
	}

	return 0;
}

static int bmp_888_888(int fd, struct pixel888 *fb, int width, int height, int xres, int yres)
{
	int ret;
	int i, j;
	struct pixel888 buff[width];

	for (i = height - 1; i >= 0; i--)
	{
		ret = read(fd, &buff, sizeof(buff));
		if (ret < 0)
		{
			print_error("read");
			return ret;
		}

		for (j = 0; j < width; j++)
		{
			(fb + i * xres)[j] = buff[j];
		}
	}

	return 0;
}

static int bmp_888_8888(int fd, struct pixel8888 *fb, int width, int height, int xres, int yres)
{
	int ret;
	int i, j;
	struct pixel8888 *tmp_fb;
	struct pixel888 buff[width];


	for (i = height - 1; i >= 0; i--)
	{
		ret = read(fd, &buff, sizeof(buff));
		if (ret < 0)
		{
			print_error("read");
			return ret;
		}

		for (j = 0; j < width; j++)
		{
			tmp_fb = fb + i * xres + j;

			tmp_fb->red = buff[j].red;
			tmp_fb->green = buff[j].green;
			tmp_fb->blue = buff[j].blue;
			tmp_fb->transp = 0;
		}
	}

	return 0;
}

static int bmp_8888_565(int fd, struct pixel565 *fb, int width, int height, int xres, int yres)
{
	int ret;
	int i, j;
	struct pixel565 *tmp_fb;
	struct pixel8888 buff[width];


	for (i = height - 1; i >= 0; i--)
	{
		ret = read(fd, &buff, sizeof(buff));
		if (ret < 0)
		{
			print_error("read");
			return ret;
		}

		for (j = 0; j < width; j++)
		{
			tmp_fb = fb + i * xres + j;

			tmp_fb->red = buff[j].red >> 3;
			tmp_fb->green = buff[j].green >> 2;
			tmp_fb->blue = buff[j].blue >> 3;
		}
	}

	return 0;
}

static int bmp_8888_888(int fd, struct pixel888 *fb, int width, int height, int xres, int yres)
{
	int ret;
	int i, j;
	struct pixel888 *tmp_fb;
	struct pixel8888 buff[width];


	for (i = height - 1; i >= 0; i--)
	{
		ret = read(fd, &buff, sizeof(buff));
		if (ret < 0)
		{
			print_error("read");
			return ret;
		}

		for (j = 0; j < width; j++)
		{
			tmp_fb = fb + i * xres + j;

			tmp_fb->red = buff[j].red;
			tmp_fb->green = buff[j].green;
			tmp_fb->blue = buff[j].blue;
		}
	}

	return 0;
}

static int bmp_8888_8888(int fd, struct pixel8888 *fb, int width, int height, int xres, int yres)
{
	int ret;
	int i, j;
	struct pixel8888 buff[width];

	for (i = height - 1; i >= 0; i--)
	{
		ret = read(fd, &buff, sizeof(buff));
		if (ret < 0)
		{
			print_error("read");
			return ret;
		}

		for (j = 0; j < width; j++)
		{
			(fb + i * xres)[j] = buff[j];
		}
	}

	return 0;
}

int bmp_view(const char *file_name, const char *fb_name)
{
	int ret;
	void *screen;
	int fd, fb;
	struct bmp_file_header file_hdr;
	struct bmp_info_header info_hdr;
	struct fb_fix_screeninfo fix;
	struct fb_var_screeninfo var;

	println("file_name = %s", file_name);

	fd = open(file_name, O_RDONLY | O_BINARY);
	if (fd < 0)
	{
		print_error("open");
		return -1;
	}

	ret = bmp_read_file_header(fd, &file_hdr);
	if (ret < 0)
	{
		error_msg("read_file_header");
		goto out_close_fd;
	}

	ret = bmp_read_info_header(fd, &info_hdr);
	if (ret < 0)
	{
		error_msg("read_info_header");
		goto out_close_fd;
	}

	ret = lseek(fd, file_hdr.offset, SEEK_SET);
	if (ret < 0)
	{
		print_error("lseek");
		goto out_close_fd;
	}

	fb = open(fb_name, O_RDWR | O_BINARY);
	if (fb < 0)
	{
		print_error("Fail to open fb");
		return fb;
	}

	ioctl(fb, FBIOGET_VSCREENINFO, &var);
	ioctl(fb, FBIOGET_FSCREENINFO, &fix);

	show_fb_var_info(&var);
	show_fb_fix_info(&fix);

	screen = mmap(NULL, fix.smem_len, PROT_WRITE | PROT_READ, MAP_SHARED, fb, 0);

	switch (info_hdr.bit_count)
	{
	case 16:
		if (info_hdr.compress)
		{
			println("picture format is: RGB565");
			switch (var.bits_per_pixel)
			{
			case 16:
				bmp_565_565(fd, screen, info_hdr.width, info_hdr.height, var.xres, var.yres);
				break;
			case 24:
				bmp_565_888(fd, screen, info_hdr.width, info_hdr.height, var.xres, var.yres);
				break;
			case 32:
				bmp_565_8888(fd, screen, info_hdr.width, info_hdr.height, var.xres, var.yres);
				break;
			default:
				error_msg("Unknown bits_per_pixel");
			}
		}
		else
		{
			println("picture format is: RGB555");
			switch (var.bits_per_pixel)
			{
			case 16:
				bmp_555_565(fd, screen, info_hdr.width, info_hdr.height, var.xres, var.yres);
				break;
			case 24:
				bmp_555_888(fd, screen, info_hdr.width, info_hdr.height, var.xres, var.yres);
				break;
			case 32:
				bmp_555_8888(fd, screen, info_hdr.width, info_hdr.height, var.xres, var.yres);
				break;
			default:
				error_msg("Unknown bits_per_pixel");
			}
		}
		break;
	case 24:
		println("picture format is: RGB888");
		switch (var.bits_per_pixel)
		{
		case 16:
			bmp_888_565(fd, screen, info_hdr.width, info_hdr.height, var.xres, var.yres);
			break;
		case 24:
			bmp_888_888(fd, screen, info_hdr.width, info_hdr.height, var.xres, var.yres);
			break;
		case 32:
			bmp_888_8888(fd, screen, info_hdr.width, info_hdr.height, var.xres, var.yres);
			break;
		default:
			error_msg("Unknown bits_per_pixel");
		}
		break;
	case 32:
		println("picture format is: RGB8888");
		switch (var.bits_per_pixel)
		{
		case 16:
			bmp_8888_565(fd, screen, info_hdr.width, info_hdr.height, var.xres, var.yres);
			break;
		case 24:
			bmp_8888_888(fd, screen, info_hdr.width, info_hdr.height, var.xres, var.yres);
			break;
		case 32:
			bmp_8888_8888(fd, screen, info_hdr.width, info_hdr.height, var.xres, var.yres);
			break;
		default:
			error_msg("Unknown bits_per_pixel");
		}
		break;
	default:
		error_msg("Unknown bit_count");
	}

	close(fb);
out_close_fd:
	close(fd);

	return ret;
}
