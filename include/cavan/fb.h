#pragma once

#include <cavan.h>
#include <linux/fb.h>
#include <cavan/display.h>

// Fuang.Cao <cavan.cfa@gmail.com> 2011-11-16 15:48:51

struct cavan_fb_color_element {
	u8 mask;
	u8 index;
	u8 offset;
	u8 off_big;
};

struct cavan_fb_device {
	int fd;

	int fb_count;
	void *fb_base;
	void *fb_cache;
	void *fb_dequeued;
	void *fb_acquired;

	u16 xres, yres;
	u16 xres_virtual;

	size_t bpp_byte;
	size_t line_size;
	size_t fb_size;

	struct fb_fix_screeninfo fix_info;
	struct fb_var_screeninfo var_info;
	struct cavan_fb_color_element red;
	struct cavan_fb_color_element green;
	struct cavan_fb_color_element blue;
	struct cavan_fb_color_element transp;

	void (*draw_point)(struct cavan_fb_device *dev, int x, int y, u32 color);
	struct pixel888 (*read_point)(struct cavan_fb_device *dev, int x, int y);
	int (*refresh)(struct cavan_fb_device *dev);
};

void show_fb_bitfield(struct fb_bitfield *field, const char *msg);
void show_fb_var_info(struct fb_var_screeninfo *var);
void show_fb_fix_info(struct fb_fix_screeninfo *fix);
void show_fb_device_info(struct cavan_fb_device *dev);

int cavan_fb_init(struct cavan_fb_device *dev, const char *fbpath);
void cavan_fb_deinit(struct cavan_fb_device *dev);
void cavan_fb_bitfield2element(struct fb_bitfield *field, struct cavan_fb_color_element *emt);
void *cavan_fb_get_acquired_sync2(struct cavan_fb_device *dev);
struct pixel888 *cavan_fb_read_line(struct cavan_fb_device *dev, struct pixel888 *pixel, int x, int y, int width);
int cavan_fb_capture(struct cavan_fb_device *dev, struct pixel888 *pixel, int x, int y, int width, int height);
int cavan_fb_capture_file(struct cavan_fb_device *dev, int fd);

int cavan_fb_display_init(struct cavan_display_device *display, struct cavan_fb_device *fb_dev);
struct cavan_display_device *cavan_fb_display_create(void);
struct cavan_display_device *cavan_fb_display_start(void);

static inline int cavan_fb_refresh(struct cavan_fb_device *dev)
{
	return dev->refresh(dev);
}

static inline void cavan_fb_draw_point(struct cavan_fb_device *dev, int x, int y, cavan_display_color_t color)
{
	dev->draw_point(dev, x, y, color.value);
}

static inline void *cavan_fb_get_acquired(struct cavan_fb_device *dev)
{
	return dev->fb_acquired;
}

static inline void *cavan_fb_get_dequeued(struct cavan_fb_device *dev)
{
	return dev->fb_dequeued;
}

static inline int cavan_fb_get_index(struct cavan_fb_device *dev, void *fb)
{
	return ADDR_SUB2(fb, dev->fb_base) / dev->fb_size;
}

static inline int cavan_fb_get_yoffset(struct cavan_fb_device *dev, void *fb)
{
	return ADDR_SUB2(fb, dev->fb_base) / dev->line_size;
}

static inline void *cavan_fb_get_address(struct cavan_fb_device *dev, int index)
{
	return ADDR_ADD(dev->fb_base, index * dev->fb_size);
}

static inline void *cavan_fb_get_acquired_sync(struct cavan_fb_device *dev, struct fb_var_screeninfo *var)
{
	return cavan_fb_get_address(dev, var->yoffset / var->yres);
}

static inline cavan_display_color_t cavan_fb_build_color(struct cavan_fb_device *dev, u32 red, u32 green, u32 blue, u32 transp)
{
	cavan_display_color_t color;

	color.value = ((red & dev->red.mask) << dev->red.offset) | \
		((green & dev->green.mask) << dev->green.offset) | \
		((blue & dev->blue.mask) << dev->blue.offset) | \
		((transp & dev->transp.mask) << dev->transp.offset);

	return color;
}

static inline cavan_display_color_t cavan_fb_build_color4f(struct cavan_fb_device *dev, float red, float green, float blue, float transp)
{
	return cavan_fb_build_color(dev, red * dev->red.mask, green * dev->green.mask, blue * dev->blue.mask, transp * dev->transp.mask);
}
