#pragma once

#include <linux/fb.h>

// Fuang.Cao <cavan.cfa@gmail.com> 2011-11-16 15:48:51

struct cavan_color_element
{
	u32 offset;
	u32 mask;
	int index;
};

struct cavan_fb_descriptor
{
	int fb;
	void *fb_base;
	int xres;
	int yres;
	int bpp;
	struct fb_fix_screeninfo fix_info;
	struct fb_var_screeninfo var_info;
	struct cavan_color_element red;
	struct cavan_color_element green;
	struct cavan_color_element blue;
	struct cavan_color_element transp;
};

void show_fb_bitfield(struct fb_bitfield *field, const char *msg);
void show_fb_var_info(struct fb_var_screeninfo *var);
void show_fb_fix_info(struct fb_fix_screeninfo *fix);

int cavan_fb_init(struct cavan_fb_descriptor *desc, const char *fbpath);
void cavan_fb_uninit(struct cavan_fb_descriptor *desc);
int cavan_fb_draw_point(struct cavan_fb_descriptor *desc, int x, int y, u32 color);
int cavan_fb_draw_line(struct cavan_fb_descriptor *desc, int x1, int y1, int x2, int y2, u32 color);
int cavan_fb_draw_rect(struct cavan_fb_descriptor *desc, int left, int top, int width, int height, u32 color);
int cavan_fb_fill_rect(struct cavan_fb_descriptor *desc, int left, int top, int width, int height, u32 color);
int cavan_fb_draw_circle(struct cavan_fb_descriptor *desc, int x, int y, int r, u32 color);
int cavan_fb_fill_circle(struct cavan_fb_descriptor *desc, int x, int y, int r, u32 color);
int cavan_fb_draw_ellipse(struct cavan_fb_descriptor *desc, int x, int y, int width, int height, u32 color);
int cavan_fb_fill_ellipse(struct cavan_fb_descriptor *desc, int x, int y, int width, int height, u32 color);

u32 cavan_fb_build_color(struct cavan_fb_descriptor *desc, u32 red, u32 green, u32 blue)
{
	return ((red << desc->red.offset) & desc->red.mask) | ((green << desc->green.offset) & desc->green.mask) | ((blue << desc->blue.offset) & desc->blue.mask);
}

