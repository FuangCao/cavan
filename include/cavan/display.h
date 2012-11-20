#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Tue Nov 20 16:09:31 CST 2012
 */

#include <cavan.h>
#include <cavan/fb.h>

struct cavan_display_color3f
{
	float red;
	float green;
	float blue;
	float transp;
};

struct cavan_display_device
{
	int width;
	int height;

	void *private_data;

	void (*update_data)(struct cavan_display_device *dev);
	void (*set_color)(struct cavan_display_device *dev, struct cavan_display_color3f *color);
	void (*draw_point)(struct cavan_display_device *dev, int x, int y);
	int (*draw_line)(struct cavan_display_device *dev, int x1, int y1, int x2, int y2);
	size_t (*mesure_text)(struct cavan_display_device *dev, const char *text);
	int (*draw_text)(struct cavan_display_device *dev, int x, int y, const char *text);
	int (*draw_rect)(struct cavan_display_device *dev, int x, int y, int width, int height);
	int (*fill_rect)(struct cavan_display_device *dev, int x, int y, int width, int height);
	int (*draw_circle)(struct cavan_display_device *dev, int x, int y, int r);
	int (*fill_circle)(struct cavan_display_device *dev, int x, int y, int r);
	int (*draw_ellipse)(struct cavan_display_device *dev, int x, int y, int width, int height);
	int (*fill_ellipse)(struct cavan_display_device *dev, int x, int y, int width, int height);
	int (*draw_polygon)(struct cavan_display_device *dev, struct cavan_fb_point *points, size_t count);
	int (*fill_triangle)(struct cavan_display_device *dev, struct cavan_fb_point *points);
	int (*fill_polygon)(struct cavan_display_device *dev, struct cavan_fb_point *points, size_t count);
};

int cavan_display_init(struct cavan_display_device *dev);
void cavan_display_uninit(struct cavan_display_device *dev);

static inline void cavan_display_build_color3f(struct cavan_display_color3f *color, float red, float green, float blue)
{
	color->red = red;
	color->green = green;
	color->blue = blue;
	color->transp = 1.0;
}
