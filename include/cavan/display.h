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
	int x, y;
	int width, height;
	int width_max, height_max;

	byte data[0];
};

struct cavan_display_rect
{
	int x, y;
	int width, height;
};

struct cavan_display_memory_rect
{
	int x, y;
	int border_width;
	int width, height;

	struct cavan_display_memory *mems[4];
};

struct cavan_display_device
{
	int xres;
	int yres;
	int bpp_byte;

	void *fb_base;
	void *private_data;
	cavan_display_color_t pen_color;

	pthread_mutex_t lock;

	void (*destory)(struct cavan_display_device *display);
	void (*refresh)(struct cavan_display_device *display);
	cavan_display_color_t (*build_color)(struct cavan_display_device *display, float red, float green, float blue, float transp);
	void (*set_color)(struct cavan_display_device *display, cavan_display_color_t color);

	size_t (*mesure_text)(struct cavan_display_device *display, const char *text);
	int (*draw_text)(struct cavan_display_device *display, int x, int y, const char *text);

	void (*draw_point)(struct cavan_display_device *display, int x, int y, cavan_display_color_t color);
	int (*draw_line)(struct cavan_display_device *display, int x1, int y1, int x2, int y2);

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

typedef void (*cavan_display_draw_point_handler_t)(struct cavan_display_device *display, int x, int y, cavan_display_color_t color);

int cavan_build_line_equation(int x1, int y1, int x2, int y2, double *a, double *b);
int cavan_display_draw_line_dummy(struct cavan_display_device *display, int x1, int y1, int x2, int y2);
int cavan_display_draw_rect_dummy(struct cavan_display_device *display, int left, int top, int width, int height);
int cavan_display_fill_rect_dummy(struct cavan_display_device *display, int left, int top, int width, int height);
int cavan_display_draw_circle_dummy(struct cavan_display_device *display, int x, int y, int r);
int cavan_display_fill_circle_dummy(struct cavan_display_device *display, int x, int y, int r);
int cavan_display_draw_ellipse_dummy(struct cavan_display_device *display, int x, int y, int width, int height);
int cavan_display_fill_ellipse_dummy(struct cavan_display_device *display, int x, int y, int width, int height);
int cavan_display_draw_polygon_dummy(struct cavan_display_device *display, cavan_display_point_t *points, size_t count);
int max3i(int a, int b, int c);
int min3i(int a, int b, int c);
void cavan_display_show_points(const cavan_display_point_t *points, size_t size);
void cavan_display_point_sort_x(cavan_display_point_t *start, cavan_display_point_t *end);
int cavan_display_fill_triangle_half(struct cavan_display_device *display, cavan_display_point_t *p1, cavan_display_point_t *p2, double a1, double b1, double a2, double b2);
int cavan_display_fill_triangle_dummy(struct cavan_display_device *display, cavan_display_point_t *points);
int cavan_display_fill_polygon_dummy(struct cavan_display_device *display, cavan_display_point_t *points, size_t count);
int cavan_build_polygon_points(struct cavan_display_device *display, cavan_display_point_t *points, size_t count, int x, int y, int r, int rotation);
int cavan_display_draw_polygon_standard(struct cavan_display_device *display, size_t count, int x, int y, int r, int rotation);
int cavan_display_fill_polygon_standard(struct cavan_display_device *display, size_t count, int x, int y, int r, int rotation);
int cavan_display_draw_polygon_standard2(struct cavan_display_device *display, size_t count, int x, int y, int r, int rotation);
int cavan_calculate_line_cross_point(int x1, int x2, double a1, double b1, double a2, double b2, cavan_display_point_t *point);
int cavan_calculate_polygo_cross_points(cavan_display_point_t *points, cavan_display_point_t *cross_points, size_t count);
int cavan_display_fill_polygon_standard2(struct cavan_display_device *display, size_t count, int x, int y, int r, int rotation);
int cavan_display_draw_polygon_standard3(struct cavan_display_device *display, size_t count, int x, int y, int r, int rotation);
int cavan_display_fill_polygon_standard3(struct cavan_display_device *display, size_t count, int x, int y, int r, int rotation);
int cavan_display_draw_polygon_standard4(struct cavan_display_device *display, size_t count, int x, int y, int r, int rotation);
int cavan_display_memory_xfer_dummy(struct cavan_display_device *display, struct cavan_display_memory *mem, bool read);
size_t cavan_display_mesure_text_dummy(struct cavan_display_device *display, const char *text);
int cavan_display_draw_text_dummy(struct cavan_display_device *display, int x, int y, const char *text);
void cavan_display_set_color_dummy(struct cavan_display_device *display, cavan_display_color_t color);
void cavan_display_destory_dummy(struct cavan_display_device *display);

int cavan_display_init(struct cavan_display_device *display);
void cavan_display_uninit(struct cavan_display_device *display);
int cavan_display_check(struct cavan_display_device *display);

struct cavan_display_memory *cavan_display_memory_alloc(struct cavan_display_device *display, size_t width, size_t height);
struct cavan_display_memory_rect *cavan_display_memory_rect_alloc(struct cavan_display_device *display, size_t width, size_t height, int border_width);
int cavan_display_memory_rect_backup(struct cavan_display_device *display, struct cavan_display_memory_rect *mem_rect, int x, int y);
int cavan_display_memory_rect_restore(struct cavan_display_device *display, struct cavan_display_memory_rect *mem_rect);
int cavan_display_draw_text_centre(struct cavan_display_device *display, struct cavan_display_rect *rect, const char *text);

static inline void cavan_display_memory_free(struct cavan_display_memory *mem)
{
	if (mem)
	{
		free(mem);
	}
}

static inline void cavan_display_memory_rect_free(struct cavan_display_memory_rect *mem_rect)
{
	if (mem_rect)
	{
		free(mem_rect);
	}
}

static inline cavan_display_color_t cavan_display_build_color4f(struct cavan_display_device *display, float red, float green, float blue, float transp)
{
	return display->build_color(display, red, green, blue, transp);
}

static inline cavan_display_color_t cavan_display_build_color3f(struct cavan_display_device *display, float red, float green, float blue)
{
	return cavan_display_build_color4f(display, red, green, blue, 1.0);
}

static inline void cavan_display_set_color4f(struct cavan_display_device *display, float red, float green, float blue, float transp)
{
	display->set_color(display, cavan_display_build_color4f(display, red, green, blue, transp));
}

static inline void cavan_display_set_color3f(struct cavan_display_device *display, float red, float green, float blue)
{
	cavan_display_set_color4f(display, red, green, blue, 1.0);
}

static inline int cavan_display_clear(struct cavan_display_device *display, cavan_display_color_t color)
{
	display->set_color(display, color);

	return display->fill_rect(display, 0, 0, display->xres, display->yres);
}

static inline int cavan_display_memory_backup(struct cavan_display_device *display, struct cavan_display_memory *mem, int x, int y)
{
	mem->x = x;
	mem->y = y;
	mem->width = mem->width_max;
	mem->height = mem->height_max;

	return display->display_memory_xfer(display, mem, true);
}

static inline int cavan_display_memory_restore(struct cavan_display_device *display, struct cavan_display_memory *mem)
{
	return display->display_memory_xfer(display, mem, false);
}

static inline void cavan_display_refresh(struct cavan_display_device *display)
{
	pthread_mutex_lock(&display->lock);
	display->refresh(display);
	pthread_mutex_unlock(&display->lock);
}
