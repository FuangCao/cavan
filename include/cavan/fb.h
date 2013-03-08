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
	int fb_count;
	int fb_active;
	void *fb_base;
	void *fb_cache;

	u16 xres, yres;

	size_t bpp_byte;
	size_t line_size;
	size_t fb_size;

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
void show_fb_device_info(struct cavan_fb_device *dev);

int cavan_fb_refresh(struct cavan_fb_device *dev);
int cavan_fb_init(struct cavan_fb_device *dev, const char *fbpath);
void cavan_fb_uninit(struct cavan_fb_device *dev);
void cavan_fb_bitfield2element(struct fb_bitfield *field, struct cavan_fb_color_element *emt);

int cavan_fb_display_init(struct cavan_display_device *display, struct cavan_fb_device *fb_dev);
struct cavan_display_device *cavan_fb_display_create(void);

static inline void cavan_fb_draw_point(struct cavan_fb_device *dev, int x, int y, cavan_display_color_t color)
{
	dev->draw_point(dev, x, y, color.value);
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
