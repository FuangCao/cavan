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

	display = cavan_fb_display_create_check();
	if (display == NULL)
	{
		pr_red_info("cavan_fb_display_create");
		return -EFAULT;
	}

	left = text2value_unsigned(argv[1], NULL, 10);
	top = text2value_unsigned(argv[2], NULL, 10);
	width = text2value_unsigned(argv[3], NULL, 10);
	height = text2value_unsigned(argv[4], NULL, 10);

	cavan_display_set_color3f(display, 1.0, 0, 0);

	if (strcmp(argv[0], "draw_rect") == 0)
	{
		ret = display->draw_rect(display, left, top, width, height);
	}
	else
	{
		ret = display->fill_rect(display, left, top, width, height);
	}

	display->refresh(display);
	display->destory(display);

	return ret;
}

static int cavan_display_test_main(int argc, char *argv[])
{
	int i, j, width, height;
	struct cavan_display_device *display;
	cavan_display_color_t color_map[3][3];

	display = cavan_fb_display_create_check();
	if (display == NULL)
	{
		pr_red_info("cavan_fb_display_create");
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
			display->fill_rect(display, i * width, j * height, width, height);
		}
	}

	display->refresh(display);
	display->destory(display);

	return 0;
}

static struct cavan_command_map cmd_map[] =
{
	{"draw_rect", cavan_display_rect_main},
	{"fill_rect", cavan_display_rect_main},
	{"test", cavan_display_test_main}
};

int main(int argc, char *argv[])
{
	return FIND_EXEC_COMMAND(cmd_map);
}
