#pragma once

#include <linux/fb.h>
#include <cavan/display.h>

// Fuang.Cao <cavan.cfa@gmail.com> 2011-11-16 15:48:51

struct cavan_fb_color_element
{
	u32 offset;
	u32 mask;
	u16 max;
	u16 index;
};

struct cavan_fb_device
{
	int fb;
	void *fb_base;

	u16 xres, yres;

	size_t byte_per_pixel;
	size_t line_size;
	size_t fb_size;

	cavan_display_color_t pen_color;

	struct fb_fix_screeninfo fix_info;
	struct fb_var_screeninfo var_info;
	struct cavan_fb_color_element red;
	struct cavan_fb_color_element green;
	struct cavan_fb_color_element blue;
	struct cavan_fb_color_element transp;

	void (*draw_point)(struct cavan_fb_device *, int x, int y, u32 color);
};

void show_fb_bitfield(struct fb_bitfield *field, const char *msg);
void show_fb_var_info(struct fb_var_screeninfo *var);
void show_fb_fix_info(struct fb_fix_screeninfo *fix);

int cavan_fb_init(struct cavan_fb_device *dev, const char *fbpath);
void cavan_fb_uninit(struct cavan_fb_device *dev);
void cavan_fb_clear(struct cavan_fb_device *dev);
int cavan_fb_draw_line(struct cavan_fb_device *dev, int x1, int y1, int x2, int y2);
int cavan_fb_draw_rect(struct cavan_fb_device *dev, int left, int top, int width, int height);
int cavan_fb_fill_rect(struct cavan_fb_device *dev, int left, int top, int width, int height);
int cavan_fb_draw_circle(struct cavan_fb_device *dev, int x, int y, int r);
int cavan_fb_fill_circle(struct cavan_fb_device *dev, int x, int y, int r);
int cavan_fb_draw_ellipse(struct cavan_fb_device *dev, int x, int y, int width, int height);
int cavan_fb_fill_ellipse(struct cavan_fb_device *dev, int x, int y, int width, int height);
int cavan_fb_draw_polygon(struct cavan_fb_device *dev, cavan_display_point_t *points, size_t count);
int cavan_fb_fill_triangle(struct cavan_fb_device *dev, cavan_display_point_t *points);
int cavan_fb_fill_polygon(struct cavan_fb_device *dev, cavan_display_point_t *points, size_t count);
int cavan_fb_draw_polygon_standard(struct cavan_fb_device *dev, size_t count, int x, int y, int r, int rotation);
int cavan_fb_fill_polygon_standard(struct cavan_fb_device *dev, size_t count, int x, int y, int r, int rotation);
int cavan_fb_draw_polygon_standard2(struct cavan_fb_device *dev, size_t count, int x, int y, int r, int rotation);
int cavan_fb_fill_polygon_standard2(struct cavan_fb_device *dev, size_t count, int x, int y, int r, int rotation);
int cavan_fb_draw_polygon_standard3(struct cavan_fb_device *dev, size_t count, int x, int y, int r, int rotation);
int cavan_fb_fill_polygon_standard3(struct cavan_fb_device *dev, size_t count, int x, int y, int r, int rotation);
int cavan_fb_draw_polygon_standard4(struct cavan_fb_device *dev, size_t count, int x, int y, int r, int rotation);

int cavan_fb_display_memory_xfer(struct cavan_fb_device *dev, struct cavan_display_memory *mem, bool read);

int cavan_build_line_equation(int x1, int y1, int x2, int y2, double *a, double *b);
void cavan_fb_point_sort_x(cavan_display_point_t *start, cavan_display_point_t *end);
void cavan_fb_show_points(const cavan_display_point_t *points, size_t size);

static inline void cavan_fb_draw_point(struct cavan_fb_device *dev, int x, int y)
{
	dev->draw_point(dev, x, y, dev->pen_color.value);
}

static inline cavan_display_color_t cavan_fb_build_color(struct cavan_fb_device *dev, u32 red, u32 green, u32 blue, u32 transp)
{
	cavan_display_color_t color;

	color.value = ((red << dev->red.offset) & dev->red.mask) | \
		((green << dev->green.offset) & dev->green.mask) | \
		((blue << dev->blue.offset) & dev->blue.mask) | \
		((transp << dev->transp.offset) & dev->transp.mask);

	return color;
}

static inline cavan_display_color_t cavan_fb_build_color4f(struct cavan_fb_device *dev, float red, float green, float blue, float transp)
{
	return cavan_fb_build_color(dev, red * dev->red.max, green * dev->green.max, blue * dev->blue.max, transp * dev->transp.max);
}

static inline void cavan_fb_set_pen_color(struct cavan_fb_device *dev, u32 red, u32 green, u32 blue)
{
	dev->pen_color = cavan_fb_build_color(dev, red, green, blue, dev->transp.max);
}

static inline void cavan_fb_set_pen_color4f(struct cavan_fb_device *dev, float red, float green, float blue, float transp)
{
	dev->pen_color = cavan_fb_build_color4f(dev, red, green, blue, transp);
}

static inline void cavan_fb_set_pen_color3f(struct cavan_fb_device *dev, float red, float green, float blue)
{
	cavan_fb_set_pen_color4f(dev, red, green, blue, 1.0);
}
