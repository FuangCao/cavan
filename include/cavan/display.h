#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Tue Nov 20 16:09:31 CST 2012
 */

#include <cavan.h>

typedef union
{
	struct
	{
		u8 red;
		u8 green;
		u8 blue;
		u8 transp;
	};

	u32 value;
} cavan_display_color_t;

typedef struct
{
	u16 x;
	u16 y;
} cavan_display_point_t;

struct cavan_display_memory
{
	int x;
	int y;

	size_t width;
	size_t height;

	size_t width_max;
	size_t height_max;

	char data[0];
};

struct cavan_display_device
{
	int width;
	int height;

	void *private_data;

	void (*update_data)(struct cavan_display_device *display);
	void (*set_color)(struct cavan_display_device *display, cavan_display_color_t color);
	int (*clear)(struct cavan_display_device *display, cavan_display_color_t color);
	void (*draw_point)(struct cavan_display_device *display, int x, int y);
	int (*draw_line)(struct cavan_display_device *display, int x1, int y1, int x2, int y2);
	size_t (*mesure_text)(struct cavan_display_device *display, const char *text);
	int (*draw_text)(struct cavan_display_device *display, int x, int y, const char *text);
	int (*draw_rect)(struct cavan_display_device *display, int x, int y, int width, int height);
	int (*fill_rect)(struct cavan_display_device *display, int x, int y, int width, int height);
	int (*draw_circle)(struct cavan_display_device *display, int x, int y, int r);
	int (*fill_circle)(struct cavan_display_device *display, int x, int y, int r);
	int (*draw_ellipse)(struct cavan_display_device *display, int x, int y, int width, int height);
	int (*fill_ellipse)(struct cavan_display_device *display, int x, int y, int width, int height);
	int (*draw_polygon)(struct cavan_display_device *display, cavan_display_point_t *points, size_t count);
	int (*fill_triangle)(struct cavan_display_device *display, cavan_display_point_t *points);
	int (*fill_polygon)(struct cavan_display_device *display, cavan_display_point_t *points, size_t count);
	int (*display_memory_xfer)(struct cavan_display_device *display, struct cavan_display_memory *mem, bool read);
};

int cavan_display_init(struct cavan_display_device *display);
void cavan_display_uninit(struct cavan_display_device *display);
cavan_display_color_t cavan_display_build_color3f(struct cavan_display_device *display, float red, float green, float blue, float transp);
struct cavan_display_memory *cavan_display_memory_alloc(struct cavan_display_device *display, size_t width, size_t height);
void cavan_display_memory_free(struct cavan_display_memory *mem);
