#define CAVAN_CMD_NAME display

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Tue Apr 10 10:36:21 CST 2012
 */

#include <cavan.h>
#include <cavan/command.h>
#include <cavan/drm.h>
#include <cavan/fb.h>

static struct cavan_display_device *cavan_display_try_create(void)
{
	struct cavan_display_device *display;

#ifdef CONFIG_CAVAN_DRM
	display = cavan_drm_display_create();
	if (display != NULL) {
		return display;
	}
#endif

	display = cavan_fb_display_create();
	if (display != NULL) {
		return display;
	}

	return NULL;
}

static struct cavan_display_device *cavan_display_try_start(void)
{
	struct cavan_display_device *display = cavan_display_try_create();

	if (display == NULL) {
		return NULL;
	}

	if (cavan_display_start(display) < 0) {
		display->destroy(display);
		return NULL;
	}

	return display;
}

static int cavan_display_rect_main(int argc, char *argv[])
{
	int ret;
	int left, top, width, height;
	struct cavan_display_device *display;

	assert(argc > 4);

	display = cavan_display_try_start();
	if (display == NULL) {
		pr_red_info("cavan_display_try_start");
		return -EFAULT;
	}

	left = text2value_unsigned(argv[1], NULL, 10);
	top = text2value_unsigned(argv[2], NULL, 10);
	width = text2value_unsigned(argv[3], NULL, 10);
	height = text2value_unsigned(argv[4], NULL, 10);

	cavan_display_set_color3f(display, 1.0, 0, 0);

	if (strcmp(argv[0], "draw_rect") == 0) {
		ret = display->draw_rect(display, left, top, width, height, display->pen_color);
	} else {
		ret = display->fill_rect(display, left, top, width, height, display->pen_color);
	}

	cavan_display_refresh_sync(display);

	cavan_display_stop(display);
	display->destroy(display);

	return ret;
}

static int cavan_display_test(struct cavan_display_device *display, int index, cavan_display_color_t color)
{
	float ratio;
	int i, j, x, width, height;
	cavan_display_color_t color_map[3][3];

	color_map[0][0] = display->build_color(display, 1.0, 0.0, 0.0, 1.0);
	color_map[0][1] = display->build_color(display, 0.0, 1.0, 0.0, 1.0);
	color_map[0][2] = display->build_color(display, 0.0, 0.0, 1.0, 1.0);

	color_map[1][0] = display->build_color(display, 1.0, 1.0, 1.0, 1.0);
	color_map[1][1] = display->build_color(display, 0.0, 0.0, 0.0, 1.0);
	color_map[1][2] = display->build_color(display, 0.5, 0.5, 0.5, 1.0);

	color_map[2][0] = display->build_color(display, 0.0, 1.0, 1.0, 1.0);
	color_map[2][1] = display->build_color(display, 1.0, 0.0, 1.0, 1.0);
	color_map[2][2] = display->build_color(display, 1.0, 1.0, 0.0, 1.0);

	println("index = %d", index);

	switch (index) {
	case 0:
		println("color = 0x%08x", color.value);
		display->fill_rect(display, 0, 0, display->xres, display->yres, color);
		break;

	case 1:
		println("Red");
		color = display->build_color(display, 1.0, 0.0, 0.0, 1.0);
		display->fill_rect(display, 0, 0, display->xres, display->yres, color);
		break;

	case 2:
		println("Green");
		color = display->build_color(display, 0.0, 1.0, 0.0, 1.0);
		display->fill_rect(display, 0, 0, display->xres, display->yres, color);
		break;

	case 3:
		println("Blue");
		color = display->build_color(display, 0.0, 0.0, 1.0, 1.0);
		display->fill_rect(display, 0, 0, display->xres, display->yres, color);
		break;

	case 4:
		println("Cyan");
		color = display->build_color(display, 0.0, 1.0, 1.0, 1.0);
		display->fill_rect(display, 0, 0, display->xres, display->yres, color);
		break;

	case 5:
		println("Purple");
		color = display->build_color(display, 1.0, 0.0, 1.0, 1.0);
		display->fill_rect(display, 0, 0, display->xres, display->yres, color);
		break;

	case 6:
		println("Yellow");
		color = display->build_color(display, 1.0, 1.0, 0.0, 1.0);
		display->fill_rect(display, 0, 0, display->xres, display->yres, color);
		break;

	case 7:
		println("White");
		color = display->build_color(display, 1.0, 1.0, 1.0, 1.0);
		display->fill_rect(display, 0, 0, display->xres, display->yres, color);
		break;

	case 8:
		println("Red\t\tGreen\t\tBlue");

		width = display->xres;
		height = display->yres / 3;

		for (i = 0; i < 3; i++) {
			display->fill_rect(display, 0, i * height, width, height, color_map[0][i]);
		}
		break;

	case 9:
		println("Red\t\tWhite\t\tCyan");
		println("Green\t\tBlack\t\tPurple");
		println("Blue\t\tGray\t\tYellow");

		width = display->xres / 3;
		height = display->yres / 3;

		for (i = 0; i < 3; i++) {
			for (j = 0; j < 3; j++) {
				display->fill_rect(display, i * width, j * height, width, height, color_map[i][j]);
			}
		}
		break;

	case 10:
		width = display->xres;
		height = display->yres;

		for (x = 0; x < width; x++) {
			ratio = ((float) x) / width;
			color = display->build_color(display, ratio, 0.0, 0.0, 1.0);
			display->fill_rect(display, x, 0, 1, height, color);
		}
		break;

	case 11:
		width = display->xres;
		height = display->yres;

		for (x = 0; x < width; x++) {
			ratio = ((float) x) / width;
			color = display->build_color(display, 0.0, ratio, 0.0, 1.0);
			display->fill_rect(display, x, 0, 1, height, color);
		}
		break;

	case 12:
		width = display->xres;
		height = display->yres;

		for (x = 0; x < width; x++) {
			ratio = ((float) x) / width;
			color = display->build_color(display, 0.0, 0.0, ratio, 1.0);
			display->fill_rect(display, x, 0, 1, height, color);
		}
		break;

	case 13:
		width = display->xres;
		height = display->yres;

		for (x = 0; x < width; x++) {
			ratio = ((float) x) / width;
			color = display->build_color(display, 0.0, ratio, ratio, 1.0);
			display->fill_rect(display, x, 0, 1, height, color);
		}
		break;

	case 14:
		width = display->xres;
		height = display->yres;

		for (x = 0; x < width; x++) {
			ratio = ((float) x) / width;
			color = display->build_color(display, ratio, 0.0, ratio, 1.0);
			display->fill_rect(display, x, 0, 1, height, color);
		}
		break;

	case 15:
		width = display->xres;
		height = display->yres;

		for (x = 0; x < width; x++) {
			ratio = ((float) x) / width;
			color = display->build_color(display, ratio, ratio, 0.0, 1.0);
			display->fill_rect(display, x, 0, 1, height, color);
		}
		break;

	case 16:
		width = display->xres;
		height = display->yres;

		for (x = 0; x < width; x++) {
			ratio = ((float) x) / width;
			color = display->build_color(display, ratio, ratio, ratio, 1.0);
			display->fill_rect(display, x, 0, 1, height, color);
		}
		break;

	case 17:
		width = display->xres;
		height = display->yres / 3;

		for (x = 0; x < width; x++) {
			ratio = ((float) x) / width;

			color = display->build_color(display, ratio, 0.0, 0.0, 1.0);
			display->fill_rect(display, x, 0, 1, height, color);

			color = display->build_color(display, 0.0, ratio, 0.0, 1.0);
			display->fill_rect(display, x, height, 1, height, color);

			color = display->build_color(display, 0.0, 0.0, ratio, 1.0);
			display->fill_rect(display, x, height * 2, 1, height, color);
		}
		break;

	case 18:
		width = display->xres;
		height = display->yres / 6;

		for (x = 0; x < width; x++) {
			ratio = ((float) x) / width;

			color = display->build_color(display, ratio, 0.0, 0.0, 1.0);
			display->fill_rect(display, x, 0, 1, height, color);

			color = display->build_color(display, 0.0, ratio, 0.0, 1.0);
			display->fill_rect(display, x, height * 1, 1, height, color);

			color = display->build_color(display, 0.0, 0.0, ratio, 1.0);
			display->fill_rect(display, x, height * 2, 1, height, color);

			color = display->build_color(display, 0.0, ratio, ratio, 1.0);
			display->fill_rect(display, x, height * 3, 1, height, color);

			color = display->build_color(display, ratio, 0.0, ratio, 1.0);
			display->fill_rect(display, x, height * 4, 1, height, color);

			color = display->build_color(display, ratio, ratio, 0.0, 1.0);
			display->fill_rect(display, x, height * 5, 1, height, color);
		}
		break;

	case 19:
		width = display->xres;
		height = display->yres;

		for (x = 0; x < width; x++) {
			ratio = ((float) x) / width;
			color = display->build_color(display, ratio, 1 - ratio, 0.0, 1.0);
			display->fill_rect(display, x, 0, 1, height, color);
		}
		break;

	case 20:
		width = display->xres;
		height = display->yres;

		for (x = 0; x < width; x++) {
			ratio = ((float) x) / width;
			color = display->build_color(display, ratio, 0.0, 1 - ratio, 1.0);
			display->fill_rect(display, x, 0, 1, height, color);
		}
		break;

	case 21:
		width = display->xres;
		height = display->yres;

		for (x = 0; x < width; x++) {
			ratio = ((float) x) / width;
			color = display->build_color(display, 0.0, ratio, 1 - ratio, 1.0);
			display->fill_rect(display, x, 0, 1, height, color);
		}
		break;

	case 22:
		width = display->xres;
		height = display->yres;

		for (x = 0; x < width; x++) {
			ratio = ((float) x) / width;
			color = display->build_color(display, ratio, 1 - ratio, 1 - ratio, 1.0);
			display->fill_rect(display, x, 0, 1, height, color);
		}
		break;

	case 23:
		width = display->xres;
		height = display->yres;

		for (x = 0; x < width; x++) {
			ratio = ((float) x) / width;
			color = display->build_color(display, 1 - ratio, ratio, 1 - ratio, 1.0);
			display->fill_rect(display, x, 0, 1, height, color);
		}
		break;

	case 24:
		width = display->xres;
		height = display->yres;

		for (x = 0; x < width; x++) {
			ratio = ((float) x) / width;
			color = display->build_color(display, 1 - ratio, 1 - ratio, ratio, 1.0);
			display->fill_rect(display, x, 0, 1, height, color);
		}
		break;

	default:
		return -EINVAL;
	}

	cavan_display_refresh_sync(display);

	return 0;
}

static int cavan_display_test_main(int argc, char *argv[])
{
	int index;
	cavan_display_color_t color;
	struct cavan_display_device *display;

	display = cavan_display_try_start();
	if (display == NULL) {
		pr_red_info("cavan_display_try_start");
		return -EFAULT;
	}

	color.value = 0x00000000;

	if (argc > 1) {
		if (strcmp("flash", argv[1]) == 0) {

			index = 0;

			while (1) {
				if ((++index & 1)) {
					color = display->build_color(display, 1.0, 1.0, 1.0, 1.0);
				} else {
					color = display->build_color(display, 0.0, 0.0, 0.0, 1.0);
				}

				display->fill_rect(display, 0, 0, display->xres, display->yres, color);
				cavan_display_refresh_sync(display);
			}
		} else if (strcmp("shadow", argv[1]) == 0) {
			while (1) {
				float red, green, blue;

				for (red = 0.0; red <= 1.0; red += 0.1) {
					for (green = 0.0; green <= 1.0; green += 0.1) {
						for (blue = 0.0; blue <= 1.0; blue += 0.1) {
							color = display->build_color(display, red, green, blue, 1.0);
							display->fill_rect(display, 0, 0, display->xres, display->yres, color);
							cavan_display_refresh_sync(display);
						}
					}
				}
			}
		} else {
			index = text2value_unsigned(argv[1], NULL, 10);

			if (argc > 2) {
				color.value = text2value_unsigned(argv[2], NULL, 16);
			}

			cavan_display_test(display, index, color);
		}
	} else {
		index = 1;

		while (cavan_display_test(display, index++, color) == 0) {
			getchar();
		}
	}

	cavan_display_stop(display);

	display->destroy(display);

	return 0;
}

static int cavan_display_wave_main(int argc, char *argv[])
{
	int fd;
	int ret;
	int zoom;
	int x, y;
	int x_old, y_old;
	int width, height;
	u32 min, max;
	u32 point = 0;
	u32 point_max;
	int point_skip;
	int point_size;
	const char *filename;
	cavan_display_color_t color_line;
	cavan_display_color_t color_point;
	struct cavan_display_device *display;
	bool draw_line, draw_point;

	if (argc < 2) {
		println("Usage: %s <filename> [bits] [skip] [zoom] [offset]", argv[0]);
		return -EINVAL;
	}

	filename = argv[1];
	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		pr_error_info("open file %s", filename);
		return fd;
	}

	display = cavan_display_try_start();
	if (display == NULL) {
		pr_red_info("cavan_display_try_start");

		ret = -EFAULT;
		goto out_close_fd;
	}

	if (argc > 2) {
		point_size = text2value_unsigned(argv[2], NULL, 10);
		if (point_size > 4) {
			point_size >>= 3;
		}
	} else {
		point_size = 2;
	}

	if (argc > 3) {
		point_skip = text2value_unsigned(argv[3], NULL, 10);
	} else {
		point_skip = 0;
	}

	if (argc > 4) {
		zoom = text2value_unsigned(argv[4], NULL, 10);
		if (zoom < 1) {
			zoom = 1;
		}
	} else {
		zoom = 1;
	}

	max = 0;
	min = (((u64) 1) << (point_size << 3)) - 1;

	while (1) {
		ssize_t rdlen;

		rdlen = read(fd, &point, point_size);
		if (rdlen < point_size) {
			break;
		}

		if (point > max) {
			max = point;
		} else if (point < min) {
			min = point;
		}
	}

	point_max = max + min;

	if (argc > 5) {
		u32 offset = text2value_unsigned(argv[5], NULL, 10) * point_size;

		if (point_skip > 0) {
			offset *= point_skip;
		}

		lseek(fd, offset, SEEK_SET);
	} else {
		lseek(fd, 0, SEEK_SET);
	}

	x_old = 0;
	y_old = 0;
	width = display->xres;
	height = display->yres;

	color_line = display->build_color(display, 1.0, 0.0, 0.0, 1.0);
	color_point = display->build_color(display, 1.0, 1.0, 0.0, 1.0);

	if (strcmp(argv[0], "wave_line") == 0) {
		draw_line = true;
		draw_point = false;
	} else if (strcmp(argv[0], "wave_point") == 0) {
		draw_line = false;
		draw_point = true;
	} else {
		draw_line = true;
		draw_point = true;
	}

	x = 0;

	while (x < width) {
		ssize_t rdlen;

		rdlen = read(fd, &point, point_size);
		if (rdlen < point_size) {
			break;
		}

		if (point_skip > 0) {
			lseek(fd, point_size * point_skip, SEEK_CUR);
		}

		y = height - ((u64) point * height) / point_max;

		if (draw_point) {
			display->fill_rect(display, x, y, 2, 2, color_point);
		}

		if (draw_line && x > 0) {
			display->draw_line(display, x_old, y_old, x, y, color_line);
		}

		x_old = x;
		y_old = y;

		x += zoom;
	}

	cavan_display_refresh_sync(display);

	while (1) {
		msleep(5000);
	}

	cavan_display_stop(display);
	display->destroy(display);

out_close_fd:
	close(fd);
	return ret;
}

static int cavan_display_wave_text_main(int argc, char *argv[])
{
	int fd;
	int ret;
	int x_old, y_old;
	int width, height;
	const char *filename;
	cavan_display_color_t color_line;
	cavan_display_color_t color_point;
	struct cavan_display_device *display;
	double xmin, xmax, xrang, xratio;
	double ymin, ymax, yrang, yratio;
	struct cavan_font *font;
	struct cavan_fifo fifo;
	double points[4096][2];
	int count = 0;
	int i;

	if (argc < 2) {
		println("Usage: %s <filename>", argv[0]);
		return -EINVAL;
	}

	filename = argv[1];
	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		pr_error_info("open file %s", filename);
		return fd;
	}

	ret = cavan_fifo_init(&fifo, 4096, &fd);
	if (ret < 0) {
		pr_red_info("cavan_fifo_init");
		goto out_close_fd;
	}

	fifo.read = file_fifo_read;

	xmin = xmax = 0;
	ymin = ymax = 0;

	while (1) {
		char line[1024];
		double x, y;

		if (cavan_fifo_read_line(&fifo, line, sizeof(line)) == NULL) {
			break;
		}

		if (sscanf(line, "%lf %lf", &x, &y) < 2) {
			y = x;
			x = count;
		}

		if (count > 0) {
			if (x < xmin) {
				xmin = x;
			} else if (x > xmax) {
				xmax = x;
			}

			if (y < ymin) {
				ymin = y;
			} else if (y > ymax) {
				ymax = y;
			}
		} else {
			xmin = xmax = x;
			ymin = ymax = y;
		}

		points[count][0] = x;
		points[count][1] = y;
		count++;
	}

	println("xmin = %lf, xmax = %lf", xmin, xmax);
	println("ymin = %lf, ymax = %lf", ymin, ymax);

	display = cavan_display_try_start();
	if (display == NULL) {
		pr_red_info("cavan_display_try_start");

		ret = -EFAULT;
		goto out_cavan_fifo_deinit;
	}

	x_old = 0;
	y_old = 0;
	width = display->xres;
	height = display->yres;

	xrang = xmax - xmin;
	xratio = ((double) width - 1) / xrang;

	yrang = ymax - ymin;
	yratio = ((double) height - 1) / yrang;

	println("xrang = %lf, xratio = %lf", xrang, xratio);
	println("yrang = %lf, yratio = %lf", yrang, yratio);

	font = cavan_font_get(CAVAN_FONT_10X18);
	if (font != NULL) {
		cavan_display_set_font(display, font);
	} else {
		font = display->font;
	}

	color_line = display->build_color(display, 1.0, 1.0, 1.0, 1.0);
	display->draw_line(display, 0, 0, width - 1, height - 1, color_line);

	color_line = display->build_color(display, 1.0, 0.0, 0.0, 1.0);
	color_point = display->build_color(display, 1.0, 1.0, 0.0, 1.0);

	for (i = 0; i < count; i++) {
		int y = height - (points[i][1] - ymin) * yratio;
		int x = (points[i][0] - xmin) * xratio;
		char buff[1024];
		int length;

		display->fill_rect(display, x, y, 2, 2, color_point);

		if (i > 0) {
			display->draw_line(display, x_old, y_old, x, y, color_line);
		}

		x_old = x;
		y_old = y;

		length = sprintf(buff, "(%.2lf,%0.2lf)", points[i][0], points[i][1]);

		if ((x + font->cwidth * length) > width) {
			x = width - font->cwidth * length;
		}

		if ((y + font->cheight) > height) {
			y = height - font->cheight;
		}

		display->draw_text(display, x, y, buff, color_point);
	}

	cavan_display_refresh_sync(display);

	while (1) {
		msleep(5000);
	}

	cavan_display_stop(display);
	display->destroy(display);

out_cavan_fifo_deinit:
	cavan_fifo_deinit(&fifo);
out_close_fd:
	close(fd);
	return ret;
}

static int cavan_display_blank_main(int argc, char *argv[])
{
	bool blank;
	struct cavan_display_device *display;

	display = cavan_fb_display_create();
	if (display == NULL) {
		pr_red_info("cavan_fb_display_create");
		return -EFAULT;
	}

	blank = argc < 2 || text2value_unsigned(argv[1], NULL, 10) > 0;
	display->blank(display, blank);

	display->destroy(display);

	return 0;
}

CAVAN_COMMAND_MAP_START {
	{ "draw_rect", cavan_display_rect_main },
	{ "fill_rect", cavan_display_rect_main },
	{ "wave", cavan_display_wave_main },
	{ "wave_line", cavan_display_wave_main },
	{ "wave_point", cavan_display_wave_main },
	{ "wave_text", cavan_display_wave_text_main },
	{ "test", cavan_display_test_main },
	{ "blank", cavan_display_blank_main },
} CAVAN_COMMAND_MAP_END;
