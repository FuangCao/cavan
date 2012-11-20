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
	struct cavan_fb_device fb_dev;
	const char *fbpath;
	int ret;
	int left, top, width, height;

	assert(argc > 4);

	fbpath = argc > 5 ? argv[5] : NULL;
	ret = cavan_fb_init(&fb_dev, fbpath);
	if (ret < 0)
	{
		pr_red_info("cavan_fb_init");
		return ret;
	}

	left = text2value_unsigned(argv[1], NULL, 10);
	top = text2value_unsigned(argv[2], NULL, 10);
	width = text2value_unsigned(argv[3], NULL, 10);
	height = text2value_unsigned(argv[4], NULL, 10);

	if (strcmp(argv[0], "draw_rect") == 0)
	{
		ret = cavan_draw_rect(&fb_dev, left, top, width, height);
	}
	else
	{
		ret = cavan_fill_rect(&fb_dev, left, top, width, height);
	}

	if (ret < 0)
	{
		pr_red_info("Failed");
	}

	cavan_fb_uninit(&fb_dev);

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
