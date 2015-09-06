#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Tue Nov 20 16:09:31 CST 2012
 */

#include <cavan.h>
#include <cavan/font.h>
#include <cavan/thread.h>

typedef union {
	struct
	{
		u8 red;
		u8 green;
		u8 blue;
		u8 transp;
	};

	u32 value;
} cavan_display_color_t;

typedef struct {
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
	struct cavan_font *font;
	cavan_display_color_t pen_color;
	cavan_display_color_t bg_color;

	pthread_mutex_t lock;
	struct cavan_thread thread;

	int cx, cy;
	int cx_min, cy_min;
	int cx_max, cy_max;

	void (*destroy)(struct cavan_display_device *display);
	void (*refresh)(struct cavan_display_device *display);
	cavan_display_color_t (*build_color)(struct cavan_display_device *display, float red, float green, float blue, float transp);
	void (*set_color)(struct cavan_display_device *display, cavan_display_color_t color);

	size_t (*mesure_text)(struct cavan_display_device *display, const char *text);
	bool (*draw_char)(struct cavan_display_device *display, int x, int y, char c, cavan_display_color_t color);
	int (*draw_text)(struct cavan_display_device *display, int x, int y, const char *text, cavan_display_color_t color);

	void (*draw_point)(struct cavan_display_device *display, int x, int y, cavan_display_color_t color);
	int (*draw_line)(struct cavan_display_device *display, int x1, int y1, int x2, int y2, cavan_display_color_t color);

	int (*draw_rect)(struct cavan_display_device *display, int x, int y, int width, int height, cavan_display_color_t color);
	int (*fill_rect)(struct cavan_display_device *display, int x, int y, int width, int height, cavan_display_color_t color);

	int (*draw_circle)(struct cavan_display_device *display, int x, int y, int r, cavan_display_color_t color);
	int (*fill_circle)(struct cavan_display_device *display, int x, int y, int r, cavan_display_color_t color);

	int (*draw_ellipse)(struct cavan_display_device *display, int x, int y, int width, int height, cavan_display_color_t color);
	int (*fill_ellipse)(struct cavan_display_device *display, int x, int y, int width, int height, cavan_display_color_t color);

	int (*draw_polygon)(struct cavan_display_device *display, cavan_display_point_t *points, size_t count, cavan_display_color_t color);
	int (*fill_triangle)(struct cavan_display_device *display, cavan_display_point_t *points, cavan_display_color_t color);
	int (*fill_polygon)(struct cavan_display_device *display, cavan_display_point_t *points, size_t count, cavan_display_color_t color);

	int (*display_memory_xfer)(struct cavan_display_device *display, struct cavan_display_memory *mem, bool read);
	bool (*scroll_screen)(struct cavan_display_device *display, int width, int height, cavan_display_color_t color);
};

typedef void (*cavan_display_draw_point_handler_t)(struct cavan_display_device *display, int x, int y, cavan_display_color_t color);

int cavan_build_line_equation(int x1, int y1, int x2, int y2, double *a, double *b);
int cavan_display_draw_line_dummy(struct cavan_display_device *display, int x1, int y1, int x2, int y2, cavan_display_color_t color);
int cavan_display_draw_rect_dummy(struct cavan_display_device *display, int left, int top, int width, int height, cavan_display_color_t color);
int cavan_display_fill_rect_dummy(struct cavan_display_device *display, int left, int top, int width, int height, cavan_display_color_t color);
int cavan_display_draw_circle_dummy(struct cavan_display_device *display, int x, int y, int r, cavan_display_color_t color);
int cavan_display_fill_circle_dummy(struct cavan_display_device *display, int x, int y, int r, cavan_display_color_t color);
int cavan_display_draw_ellipse_dummy(struct cavan_display_device *display, int x, int y, int width, int height, cavan_display_color_t color);
int cavan_display_fill_ellipse_dummy(struct cavan_display_device *display, int x, int y, int width, int height, cavan_display_color_t color);
int cavan_display_draw_polygon_dummy(struct cavan_display_device *display, cavan_display_point_t *points, size_t count, cavan_display_color_t color);
int max3i(int a, int b, int c);
int min3i(int a, int b, int c);
void cavan_display_show_points(const cavan_display_point_t *points, size_t size);
void cavan_display_point_sort_x(cavan_display_point_t *start, cavan_display_point_t *end);
int cavan_display_fill_triangle_half(struct cavan_display_device *display, cavan_display_point_t *p1, cavan_display_point_t *p2, double a1, double b1, double a2, double b2, cavan_display_color_t color);
int cavan_display_fill_triangle_dummy(struct cavan_display_device *display, cavan_display_point_t *points, cavan_display_color_t color);
int cavan_display_fill_polygon_dummy(struct cavan_display_device *display, cavan_display_point_t *points, size_t count, cavan_display_color_t color);
int cavan_build_polygon_points(struct cavan_display_device *display, cavan_display_point_t *points, size_t count, int x, int y, int r, int rotation);
int cavan_display_draw_polygon_standard(struct cavan_display_device *display, size_t count, int x, int y, int r, int rotation, cavan_display_color_t color);
int cavan_display_fill_polygon_standard(struct cavan_display_device *display, size_t count, int x, int y, int r, int rotation, cavan_display_color_t color);
int cavan_display_draw_polygon_standard2(struct cavan_display_device *display, size_t count, int x, int y, int r, int rotation, cavan_display_color_t color);
int cavan_calculate_line_cross_point(int x1, int x2, double a1, double b1, double a2, double b2, cavan_display_point_t *point);
int cavan_calculate_polygo_cross_points(cavan_display_point_t *points, cavan_display_point_t *cross_points, size_t count);
int cavan_display_fill_polygon_standard2(struct cavan_display_device *display, size_t count, int x, int y, int r, int rotation, cavan_display_color_t color);
int cavan_display_draw_polygon_standard3(struct cavan_display_device *display, size_t count, int x, int y, int r, int rotation, cavan_display_color_t color);
int cavan_display_fill_polygon_standard3(struct cavan_display_device *display, size_t count, int x, int y, int r, int rotation, cavan_display_color_t color);
int cavan_display_draw_polygon_standard4(struct cavan_display_device *display, size_t count, int x, int y, int r, int rotation, cavan_display_color_t color);
int cavan_display_memory_xfer_dummy(struct cavan_display_device *display, struct cavan_display_memory *mem, bool read);
size_t cavan_display_mesure_text_dummy(struct cavan_display_device *display, const char *text);
bool cavan_display_draw_char_dummy(struct cavan_display_device *display, int x, int y, char c, cavan_display_color_t color);
int cavan_display_draw_text_dummy(struct cavan_display_device *display, int x, int y, const char *text, cavan_display_color_t color);
bool cavan_display_scroll_screen_dummy(struct cavan_display_device *display, int width, int height, cavan_display_color_t color);
void cavan_display_set_color_dummy(struct cavan_display_device *display, cavan_display_color_t color);
void cavan_display_destroy_dummy(struct cavan_display_device *display);

int cavan_display_init(struct cavan_display_device *display);
void cavan_display_deinit(struct cavan_display_device *display);
int cavan_display_start(struct cavan_display_device *display);
void cavan_display_stop(struct cavan_display_device *display);

struct cavan_display_memory *cavan_display_memory_alloc(struct cavan_display_device *display, size_t width, size_t height);
struct cavan_display_memory_rect *cavan_display_memory_rect_alloc(struct cavan_display_device *display, size_t width, size_t height, int border_width);
int cavan_display_memory_rect_backup(struct cavan_display_device *display, struct cavan_display_memory_rect *mem_rect, int x, int y);
int cavan_display_memory_rect_restore(struct cavan_display_device *display, struct cavan_display_memory_rect *mem_rect);
int cavan_display_draw_text_center(struct cavan_display_device *display, struct cavan_display_rect *rect, const char *text, cavan_display_color_t color);
void cavan_display_set_font(struct cavan_display_device *display, struct cavan_font *font);

void cavan_display_print_char(struct cavan_display_device *display, char c);
int cavan_display_print_ntext(struct cavan_display_device *display, const char *text, size_t size);
int cavan_display_print_text(struct cavan_display_device *display, const char *text);
int cavan_display_printf(struct cavan_display_device *display, const char *fmt, ...);

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
	return display->fill_rect(display, 0, 0, display->xres, display->yres, color);
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

static inline void cavan_display_lock(struct cavan_display_device *display)
{
	pthread_mutex_lock(&display->lock);
}

static inline void cavan_display_unlock(struct cavan_display_device *display)
{
	pthread_mutex_unlock(&display->lock);
}

static inline void cavan_display_refresh(struct cavan_display_device *display)
{
	cavan_thread_resume(&display->thread);
}

static inline void cavan_display_flush(struct cavan_display_device *display)
{
	cavan_display_refresh(display);
}

#if 0
static inline double cavan_display_cal_brightness(int red, int green, int blue)
{
	return 0.3 * red + 0.59 * green + 0.11 * blue;
}
#else
static inline int cavan_display_cal_brightness(int red, int green, int blue)
{
	return (77 * red + 150 * green + 29 * blue) >> 8;
}
#endif

