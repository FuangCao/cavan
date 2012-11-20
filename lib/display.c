/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Tue Nov 20 16:09:31 CST 2012
 */

#include <cavan.h>
#include <cavan/display.h>

static void cavan_display_update_data(struct cavan_display_device *dev)
{
	pr_pos_info();
}

static void cavan_display_set_color(struct cavan_display_device *dev, struct cavan_display_color3f *color)
{
	cavan_fb_set_pen_color3f(dev->private_data, color->red, color->green, color->blue);
}

static void cavan_display_draw_point(struct cavan_display_device *dev, int x, int y)
{
	cavan_fb_draw_point2(dev->private_data, x, y);
}

static int cavan_display_draw_line(struct cavan_display_device *dev, int x1, int y1, int x2, int y2)
{
	return cavan_fb_draw_line(dev->private_data, x1, y1, x2, y2);
}

static size_t cavan_display_mesure_text(struct cavan_display_device *dev, const char *text)
{
	return text_len(text);
}

static int cavan_display_draw_text(struct cavan_display_device *dev, int x, int y, const char *text)
{
	pr_pos_info();

	return 0;
}

static int cavan_display_draw_rect(struct cavan_display_device *dev, int x, int y, int width, int height)
{
	return cavan_fb_draw_rect(dev->private_data, x, y, width, height);
}

static int cavan_display_fill_rect(struct cavan_display_device *dev, int x, int y, int width, int height)
{
	return cavan_fb_fill_rect(dev->private_data, x, y, width, height);
}

static int cavan_display_draw_circle(struct cavan_display_device *dev, int x, int y, int r)
{
	return cavan_fb_draw_circle(dev->private_data, x, y, r);
}

static int cavan_display_fill_circle(struct cavan_display_device *dev, int x, int y, int r)
{
	return cavan_fb_fill_circle(dev->private_data, x, y, r);
}

static int cavan_display_draw_ellipse(struct cavan_display_device *dev, int x, int y, int width, int height)
{
	return cavan_fb_draw_ellipse(dev->private_data, x, y, width, height);
}

static int cavan_display_fill_ellipse(struct cavan_display_device *dev, int x, int y, int width, int height)
{
	return cavan_fb_fill_ellipse(dev->private_data, x, y, width, height);
}

static int cavan_display_draw_polygon(struct cavan_display_device *dev, struct cavan_fb_point *points, size_t count)
{
	return cavan_fb_draw_polygon(dev->private_data, points, count);
}

static int cavan_display_fill_triangle(struct cavan_display_device *dev, struct cavan_fb_point *points)
{
	return cavan_fb_fill_triangle(dev->private_data, points);
}

static int cavan_display_fill_polygon(struct cavan_display_device *dev, struct cavan_fb_point *points, size_t count)
{
	return cavan_fb_fill_polygon(dev->private_data, points, count);
}

int cavan_display_init(struct cavan_display_device *dev)
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

	dev->private_data = fb_dev;
	dev->width = fb_dev->xres;
	dev->height = fb_dev->yres;

	dev->update_data = cavan_display_update_data;
	dev->set_color = cavan_display_set_color;
	dev->draw_point = cavan_display_draw_point;
	dev->draw_line = cavan_display_draw_line;
	dev->mesure_text = cavan_display_mesure_text;
	dev->draw_text = cavan_display_draw_text;
	dev->draw_rect = cavan_display_draw_rect;
	dev->fill_rect = cavan_display_fill_rect;
	dev->draw_circle = cavan_display_draw_circle;
	dev->fill_circle = cavan_display_fill_circle;
	dev->draw_ellipse = cavan_display_draw_ellipse;
	dev->fill_ellipse = cavan_display_fill_ellipse;
	dev->draw_polygon = cavan_display_draw_polygon;
	dev->fill_triangle = cavan_display_fill_triangle;
	dev->fill_polygon = cavan_display_fill_polygon;

	return 0;
}

void cavan_display_uninit(struct cavan_display_device *dev)
{
	cavan_fb_uninit(dev->private_data);
	free(dev->private_data);
}
