/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Tue Nov 20 16:09:31 CST 2012
 */

#include <cavan.h>
#include <cavan/fb.h>
#include <cavan/display.h>

cavan_display_color_t cavan_display_build_color4f(struct cavan_display_device *display, float red, float green, float blue, float transp)
{
	return cavan_fb_build_color4f(display->private_data, red, green, blue, transp);
}

static void cavan_display_update_data(struct cavan_display_device *display)
{
}

static void cavan_display_set_color(struct cavan_display_device *display, cavan_display_color_t color)
{
	struct cavan_fb_device *fb_dev = display->private_data;

	fb_dev->pen_color = color;
}

static void cavan_display_draw_point(struct cavan_display_device *display, int x, int y)
{
	cavan_fb_draw_point(display->private_data, x, y);
}

static int cavan_display_draw_line(struct cavan_display_device *display, int x1, int y1, int x2, int y2)
{
	return cavan_fb_draw_line(display->private_data, x1, y1, x2, y2);
}

static size_t cavan_display_mesure_text(struct cavan_display_device *display, const char *text)
{
	return text_len(text);
}

static int cavan_display_draw_text(struct cavan_display_device *display, int x, int y, const char *text)
{
	return 0;
}

static int cavan_display_draw_rect(struct cavan_display_device *display, int x, int y, int width, int height)
{
	return cavan_fb_draw_rect(display->private_data, x, y, width, height);
}

static int cavan_display_fill_rect(struct cavan_display_device *display, int x, int y, int width, int height)
{
	return cavan_fb_fill_rect(display->private_data, x, y, width, height);
}

static int cavan_display_draw_circle(struct cavan_display_device *display, int x, int y, int r)
{
	return cavan_fb_draw_circle(display->private_data, x, y, r);
}

static int cavan_display_fill_circle(struct cavan_display_device *display, int x, int y, int r)
{
	return cavan_fb_fill_circle(display->private_data, x, y, r);
}

static int cavan_display_draw_ellipse(struct cavan_display_device *display, int x, int y, int width, int height)
{
	return cavan_fb_draw_ellipse(display->private_data, x, y, width, height);
}

static int cavan_display_fill_ellipse(struct cavan_display_device *display, int x, int y, int width, int height)
{
	return cavan_fb_fill_ellipse(display->private_data, x, y, width, height);
}

static int cavan_display_draw_polygon(struct cavan_display_device *display, cavan_display_point_t *points, size_t count)
{
	return cavan_fb_draw_polygon(display->private_data, points, count);
}

static int cavan_display_fill_triangle(struct cavan_display_device *display, cavan_display_point_t *points)
{
	return cavan_fb_fill_triangle(display->private_data, points);
}

static int cavan_display_fill_polygon(struct cavan_display_device *display, cavan_display_point_t *points, size_t count)
{
	return cavan_fb_fill_polygon(display->private_data, points, count);
}

static int cavan_display_memory_xfer(struct cavan_display_device *display, struct cavan_display_memory *mem, bool read)
{
	return cavan_fb_display_memory_xfer(display->private_data, mem, read);
}

static int cavan_display_clear(struct cavan_display_device *display, cavan_display_color_t color)
{
	struct cavan_fb_device *fb_dev = display->private_data;

	fb_dev->pen_color = color;

	return cavan_fb_fill_rect(fb_dev, 0, 0, display->width, display->height);
}

int cavan_display_init(struct cavan_display_device *display)
{
	int ret;
	struct cavan_fb_device *fb_dev;

	fb_dev = malloc(sizeof(*fb_dev));
	if (fb_dev == NULL)
	{
		pr_error_info("malloc");
		return -ENOMEM;
	}

	ret = cavan_fb_init(fb_dev, NULL);
	if (ret < 0)
	{
		pr_red_info("cavan_fb_init");
		free(fb_dev);
		return ret;
	}

	display->private_data = fb_dev;
	display->width = fb_dev->xres;
	display->height = fb_dev->yres;

	display->update_data = cavan_display_update_data;
	display->set_color = cavan_display_set_color;
	display->clear = cavan_display_clear;
	display->draw_point = cavan_display_draw_point;
	display->draw_line = cavan_display_draw_line;
	display->mesure_text = cavan_display_mesure_text;
	display->draw_text = cavan_display_draw_text;
	display->draw_rect = cavan_display_draw_rect;
	display->fill_rect = cavan_display_fill_rect;
	display->draw_circle = cavan_display_draw_circle;
	display->fill_circle = cavan_display_fill_circle;
	display->draw_ellipse = cavan_display_draw_ellipse;
	display->fill_ellipse = cavan_display_fill_ellipse;
	display->draw_polygon = cavan_display_draw_polygon;
	display->fill_triangle = cavan_display_fill_triangle;
	display->fill_polygon = cavan_display_fill_polygon;
	display->display_memory_xfer = cavan_display_memory_xfer;

	return 0;
}

void cavan_display_uninit(struct cavan_display_device *display)
{
	cavan_fb_uninit(display->private_data);
	free(display->private_data);
}

struct cavan_display_memory *cavan_display_memory_alloc(struct cavan_display_device *display, size_t width, size_t height)
{
	struct cavan_display_memory *mem;
	struct cavan_fb_device *fb_dev = display->private_data;
	size_t size;

	size = width * height * fb_dev->byte_per_pixel;
	mem = malloc(sizeof(*mem) + size);
	if (mem == NULL)
	{
		pr_error_info("malloc");
		return NULL;
	}

	mem->x = 0;
	mem->y = 0;
	mem->width = 0;
	mem->height = 0;

	mem->width_max = width;
	mem->height_max = height;

	return mem;
}

void cavan_display_memory_free(struct cavan_display_memory *mem)
{
	free(mem);
}
