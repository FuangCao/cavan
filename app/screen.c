/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Tue Apr 10 10:36:21 CST 2012
 */

#include <cavan.h>
#include <cavan/command.h>
#include <cavan/fb.h>

static int cavan_draw_rect_main(int argc, char *argv[])
{
	struct cavan_display_device *display;
	int ret;
	int left, top, width, height;

	assert(argc > 4);

	display = cavan_fb_display_create();
	if (display == NULL)
	{
		pr_red_info("cavan_fb_display_create");
		return -EFAULT;
	}

	ret = cavan_display_check(display);
	if (ret < 0)
	{
		pr_red_info("cavan_display_check");
		display->destory(display);
		return ret;
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

	if (ret < 0)
	{
		pr_red_info("Failed");
	}

	display->destory(display);

	return ret;
}

static struct cavan_command_map cmd_map[] =
{
	{"draw_rect", cavan_draw_rect_main},
	{"fill_rect", cavan_draw_rect_main},
};

int main(int argc, char *argv[])
{
	return FIND_EXEC_COMMAND(cmd_map);
}
