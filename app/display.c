/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Tue Apr 10 10:36:21 CST 2012
 */

#include <cavan.h>
#include <cavan/command.h>
#include <cavan/fb.h>

static int cavan_display_rect_main(int argc, char *argv[])
{
	int ret;
	int left, top, width, height;
	struct cavan_display_device *display;

	assert(argc > 4);

	display = cavan_fb_display_start();
	if (display == NULL)
	{
		pr_red_info("cavan_fb_display_start");
		return -EFAULT;
	}

	left = text2value_unsigned(argv[1], NULL, 10);
	top = text2value_unsigned(argv[2], NULL, 10);
	width = text2value_unsigned(argv[3], NULL, 10);
	height = text2value_unsigned(argv[4], NULL, 10);

	cavan_display_set_color3f(display, 1.0, 0, 0);

	if (strcmp(argv[0], "draw_rect") == 0)
	{
		ret = display->draw_rect(display, left, top, width, height, display->pen_color);
	}
	else
	{
		ret = display->fill_rect(display, left, top, width, height, display->pen_color);
	}

	cavan_display_refresh(display);

	cavan_display_stop(display);
	display->destroy(display);

	return ret;
}

static int cavan_display_test_main(int argc, char *argv[])
{
	int i, j, width, height;
	struct cavan_display_device *display;
	cavan_display_color_t color_map[3][3];

	display = cavan_fb_display_start();
	if (display == NULL)
	{
		pr_red_info("cavan_fb_display_start");
		return -EFAULT;
	}

	color_map[0][0] = display->build_color(display, 1.0, 0.0, 0.0, 1.0);
	color_map[0][1] = display->build_color(display, 0.0, 1.0, 0.0, 1.0);
	color_map[0][2] = display->build_color(display, 0.0, 0.0, 1.0, 1.0);

	color_map[1][0] = display->build_color(display, 1.0, 1.0, 1.0, 1.0);
	color_map[1][1] = display->build_color(display, 0.0, 0.0, 0.0, 1.0);
	color_map[1][2] = display->build_color(display, 0.5, 0.5, 0.5, 1.0);

	color_map[2][0] = display->build_color(display, 0.0, 1.0, 1.0, 1.0);
	color_map[2][1] = display->build_color(display, 1.0, 0.0, 1.0, 1.0);
	color_map[2][2] = display->build_color(display, 1.0, 1.0, 0.0, 1.0);

	width = display->xres / 3;
	height = display->yres / 3;

	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			display->set_color(display, color_map[i][j]);
			display->fill_rect(display, i * width, j * height, width, height, display->pen_color);
		}
	}

	cavan_display_refresh(display);

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
	u32 point = 0;
	u32 point_max;
	int point_skip;
	int point_size;
	const char *filename;
	cavan_display_color_t color_line;
	cavan_display_color_t color_point;
	struct cavan_display_device *display;
	bool draw_line, draw_point;

	if (argc < 2)
	{
		println("Usage: %s <filename> [bits] [skip] [zoom] [offset]", argv[0]);
		return -EINVAL;
	}

	filename = argv[1];
	fd = open(filename, O_RDONLY);
	if (fd < 0)
	{
		pr_error_info("open file %s", filename);
		return fd;
	}

	display = cavan_fb_display_start();
	if (display == NULL)
	{
		pr_red_info("cavan_fb_display_start");

		ret = -EFAULT;
		goto out_close_fd;
	}

	if (argc > 2)
	{
		point_size = text2value_unsigned(argv[2], NULL, 10);
		if (point_size > 4)
		{
			point_size >>= 3;
		}
	}
	else
	{
		point_size = 2;
	}

	if (argc > 3)
	{
		point_skip = text2value_unsigned(argv[3], NULL, 10);
	}
	else
	{
		point_skip = 0;
	}

	if (argc > 4)
	{
		zoom = text2value_unsigned(argv[4], NULL, 10);
		if (zoom < 1)
		{
			zoom = 1;
		}
	}
	else
	{
		zoom = 1;
	}

	if (argc > 5)
	{
		u32 offset = text2value_unsigned(argv[5], NULL, 10) * point_size;

		if (point_skip > 0)
		{
			offset *= point_skip;
		}

		lseek(fd, offset, SEEK_SET);
	}

	x_old = 0;
	y_old = 0;
	width = display->xres;
	height = display->yres;
	point_max = (((u64) 1) << (point_size * 8)) - 1;

	color_line = display->build_color(display, 1.0, 0.0, 0.0, 1.0);
	color_point = display->build_color(display, 1.0, 1.0, 0.0, 1.0);

	if (strcmp(argv[0], "wave_line") == 0)
	{
		draw_line = true;
		draw_point = false;
	}
	else if (strcmp(argv[0], "wave_point") == 0)
	{
		draw_line = false;
		draw_point = true;
	}
	else
	{
		draw_line = true;
		draw_point = true;
	}

	x = 0;

	while (x < width)
	{
		ssize_t rdlen;

		rdlen = read(fd, &point, point_size);
		if (rdlen < point_size)
		{
			break;
		}

		if (point_skip > 0)
		{
			lseek(fd, point_size * point_skip, SEEK_CUR);
		}

		y = height - ((u64) point * height) / point_max;

		if (draw_point)
		{
			display->fill_rect(display, x, y, 2, 2, color_point);
		}

		if (draw_line && x > 0)
		{
			display->draw_line(display, x_old, y_old, x, y, color_line);
		}

		x_old = x;
		y_old = y;

		x += zoom;
	}

	cavan_display_refresh(display);

	while (1)
	{
		msleep(5000);
	}

	cavan_display_stop(display);
	display->destroy(display);

out_close_fd:
	close(fd);
	return ret;
}

static struct cavan_command_map cmd_map[] =
{
	{"draw_rect", cavan_display_rect_main},
	{"fill_rect", cavan_display_rect_main},
	{"wave", cavan_display_wave_main},
	{"wave_line", cavan_display_wave_main},
	{"wave_point", cavan_display_wave_main},
	{"test", cavan_display_test_main}
};

FIND_EXEC_COMMAND_MAIN(cmd_map);
