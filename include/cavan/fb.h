#pragma once

#include <linux/fb.h>

// Fuang.Cao <cavan.cfa@gmail.com> 2011-11-16 15:48:51

struct cavan_point
{
	u32 x;
	u32 y;
};

struct cavan_color_element
{
	u32 offset;
	u32 mask;
	u16 max;
	u16 index;
};

struct cavan_screen_descriptor
{
	int fb;
	void *fb_base;
	int xres;
	int yres;
	size_t line_size;
	size_t fb_size;
	u32 pen_color;
	struct fb_fix_screeninfo fix_info;
	struct fb_var_screeninfo var_info;
	struct cavan_color_element red;
	struct cavan_color_element green;
	struct cavan_color_element blue;
	struct cavan_color_element transp;

	void (*draw_point)(struct cavan_screen_descriptor *, int x, int y, u32 color);
};

void show_fb_bitfield(struct fb_bitfield *field, const char *msg);
void show_fb_var_info(struct fb_var_screeninfo *var);
void show_fb_fix_info(struct fb_fix_screeninfo *fix);

int cavan_fb_init(struct cavan_screen_descriptor *desc, const char *fbpath);
void cavan_fb_uninit(struct cavan_screen_descriptor *desc);
void cavan_fb_clear(struct cavan_screen_descriptor *desc);
int cavan_draw_line(struct cavan_screen_descriptor *desc, int x1, int y1, int x2, int y2);
int cavan_draw_rect(struct cavan_screen_descriptor *desc, int left, int top, int width, int height);
int cavan_fill_rect(struct cavan_screen_descriptor *desc, int left, int top, int width, int height);
int cavan_draw_circle(struct cavan_screen_descriptor *desc, int x, int y, int r);
int cavan_fill_circle(struct cavan_screen_descriptor *desc, int x, int y, int r);
int cavan_draw_ellipse(struct cavan_screen_descriptor *desc, int x, int y, int width, int height);
int cavan_fill_ellipse(struct cavan_screen_descriptor *desc, int x, int y, int width, int height);
int cavan_draw_polygon(struct cavan_screen_descriptor *desc, struct cavan_point *points, size_t count);
int cavan_fill_triangle(struct cavan_screen_descriptor *desc, struct cavan_point *points);
int cavan_fill_polygon(struct cavan_screen_descriptor *desc, struct cavan_point *points, size_t count);
int cavan_draw_polygon_standard(struct cavan_screen_descriptor *desc, size_t count, int x, int y, int r, int rotation);
int cavan_fill_polygon_standard(struct cavan_screen_descriptor *desc, size_t count, int x, int y, int r, int rotation);
int cavan_draw_polygon_standard2(struct cavan_screen_descriptor *desc, size_t count, int x, int y, int r, int rotation);
int cavan_fill_polygon_standard2(struct cavan_screen_descriptor *desc, size_t count, int x, int y, int r, int rotation);
int cavan_draw_polygon_standard3(struct cavan_screen_descriptor *desc, size_t count, int x, int y, int r, int rotation);
int cavan_fill_polygon_standard3(struct cavan_screen_descriptor *desc, size_t count, int x, int y, int r, int rotation);
int cavan_draw_polygon_standard4(struct cavan_screen_descriptor *desc, size_t count, int x, int y, int r, int rotation);

int cavan_build_line_equation(int x1, int y1, int x2, int y2, double *a, double *b);
void cavan_point_sort_x(struct cavan_point *start, struct cavan_point *end);
void show_cavan_points(const struct cavan_point *points, size_t size);

static inline void cavan_draw_point(struct cavan_screen_descriptor *desc, int x, int y, u32 color)
{
	return desc->draw_point(desc, x, y, color);
}

static inline u32 cavan_build_color(struct cavan_screen_descriptor *desc, u32 red, u32 green, u32 blue, u32 transp)
{
	return ((red << desc->red.offset) & desc->red.mask) | \
		((green << desc->green.offset) & desc->green.mask) | \
		((blue << desc->blue.offset) & desc->blue.mask) | \
		((transp << desc->transp.offset) & desc->transp.mask);
}

static inline u32 cavan_build_color3f(struct cavan_screen_descriptor *desc, float red, float green, float blue)
{
	return cavan_build_color(desc, red * desc->red.max, green * desc->green.max, blue * desc->blue.max, desc->transp.max);
}

static inline void cavan_set_pen_color(struct cavan_screen_descriptor *desc, u32 red, u32 green, u32 blue)
{
	desc->pen_color = cavan_build_color(desc, red, green, blue, desc->transp.max);
}

static inline void cavan_set_pen_color3f(struct cavan_screen_descriptor *desc, float red, float green, float blue)
{
	desc->pen_color = cavan_build_color3f(desc, red, green, blue);
}
