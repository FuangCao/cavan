#include <cavan.h>
#include <cavan/fb.h>
#include <cavan/calculator.h>
#include <math.h>

// Fuang.Cao <cavan.cfa@gmail.com> 2011-11-16 15:48:51

void show_fb_bitfield(struct fb_bitfield *field, const char *msg)
{
	print_sep(60);
	if (msg)
	{
		print_string(msg);
	}
	println("field->offset = %d", field->offset);
	println("field->length = %d", field->length);
	println("field->msb_right = %d", field->msb_right);
}

void show_fb_var_info(struct fb_var_screeninfo *var)
{
	print_sep(60);
	println("var->xres = %d", var->xres);
	println("var->yres = %d", var->yres);
	println("var->bits_per_pixel = %d", var->bits_per_pixel);
	show_fb_bitfield(&var->red, "red fb_bitfield:");
	show_fb_bitfield(&var->green, "green fb_bitfield:");
	show_fb_bitfield(&var->blue, "blue fb_bitfield:");
	show_fb_bitfield(&var->transp, "transp fb_bitfield:");
}

void show_fb_fix_info(struct fb_fix_screeninfo *fix)
{
	print_sep(60);
	println("smem_start = 0x%08lx", fix->smem_start);
	println("smem_len = 0x%08x", fix->smem_len);
}

void cavan_fb_bitfield2element(struct fb_bitfield *field, struct cavan_color_element *emt)
{
	emt->offset = field->offset;
	emt->max = (1 << field->length) - 1;
	emt->mask = emt->max << emt->offset;
	emt->index = emt->offset >> 3;
}

int cavan_fb_init(struct cavan_fb_descriptor *desc, const char *fbpath)
{
	int ret;
	int fb;

	if (fbpath == NULL)
	{
		fbpath = "/dev/fb0";
	}

	fb = open(fbpath, O_RDWR);
	if (fb < 0)
	{
		print_error("open device %s failed", fbpath);
		return fb;
	}

	desc->fb = fb;

	ret = ioctl(fb, FBIOGET_VSCREENINFO, &desc->var_info);
	if (ret < 0)
	{
		print_error("get screen var info failed");
		goto out_close_fb;
	}

	show_fb_var_info(&desc->var_info);

	ret = ioctl(fb, FBIOGET_FSCREENINFO, &desc->fix_info);
	if (ret < 0)
	{
		print_error("get screen fix info failed");
		goto out_close_fb;
	}

	show_fb_fix_info(&desc->fix_info);

	desc->fb_base = mmap(NULL, desc->fix_info.smem_len, PROT_WRITE | PROT_READ, MAP_SHARED, fb, 0);
	if (desc->fb_base == NULL)
	{
		print_error("map framebuffer failed");
		ret = -1;
		goto out_close_fb;
	}

	desc->xres = desc->var_info.xres;
	desc->yres = desc->var_info.yres;
	desc->bpp = desc->var_info.bits_per_pixel >> 3;

	cavan_fb_bitfield2element(&desc->var_info.red, &desc->red);
	cavan_fb_bitfield2element(&desc->var_info.green, &desc->green);
	cavan_fb_bitfield2element(&desc->var_info.blue, &desc->blue);
	cavan_fb_bitfield2element(&desc->var_info.transp, &desc->transp);

	return 0;

out_close_fb:
	close(fb);

	return ret;
}

void cavan_fb_uninit(struct cavan_fb_descriptor *desc)
{
	munmap(desc->fb_base, desc->fix_info.smem_len);
	close(desc->fb);
}

static inline void cavan_fb_draw_point8(struct cavan_fb_descriptor * desc,int x,int y,u32 color)
{
	*(((u8 *)desc->fb_base) + y * desc->xres + x) = color;
}

static inline void cavan_fb_draw_point16(struct cavan_fb_descriptor * desc,int x,int y,u32 color)
{
	*(((u16 *)desc->fb_base) + y * desc->xres + x) = color;
}

static inline void cavan_fb_draw_point24(struct cavan_fb_descriptor *desc, int x, int y, u32 color)
{
	u8 *base;

	base = desc->fb_base + (y * desc->xres + x) * 3;
	base[desc->red.index] = (color & desc->red.mask) >> desc->red.offset;
	base[desc->green.index] = (color & desc->green.mask) >> desc->green.offset;
	base[desc->blue.index] = (color & desc->blue.mask) >> desc->blue.offset;
}

static inline void cavan_fb_draw_point32(struct cavan_fb_descriptor * desc,int x,int y,u32 color)
{
	*(((u32 *)desc->fb_base) + y * desc->xres + x) = color;
}

void *cavan_fb_get_draw_point_function(struct cavan_fb_descriptor *desc)
{
	void (*cavan_draw_point_handle_table[])(struct cavan_fb_descriptor *, int, int, u32) =
	{
		cavan_fb_draw_point8, cavan_fb_draw_point16, cavan_fb_draw_point24, cavan_fb_draw_point32
	};

	if (desc->bpp > 0 && desc->bpp <= ARRAY_SIZE(cavan_draw_point_handle_table))
	{
		return cavan_draw_point_handle_table[desc->bpp - 1];
	}

	return NULL;
}

int cavan_fb_draw_point(struct cavan_fb_descriptor *desc, int x, int y, u32 color)
{
	switch (desc->bpp)
	{
	case 1:
		cavan_fb_draw_point8(desc, x, y, color);
		break;

	case 2:
		cavan_fb_draw_point16(desc, x, y, color);
		break;

	case 3:
		cavan_fb_draw_point24(desc, x, y, color);
		break;

	case 4:
		cavan_fb_draw_point32(desc, x, y, color);
		break;

	default:
		return -1;
	}

	return 0;
}

static void cavan_build_line_equation(int x1, int y1, int x2, int y2, double *a, double *b)
{
	if (x1 == x2)
	{
		*a = 0;
	}
	else
	{
		*a = ((double)(y2 - y1)) / (x2 - x1);
	}

	*b = y1 - *a * x1;
}

static int cavan_fb_draw_line_horizon(struct cavan_fb_descriptor *desc, int x1, int y1, int x2, int y2, u32 color)
{
	double a, b;
	void (*draw_point_handle)(struct cavan_fb_descriptor *, int, int, u32);

	draw_point_handle = cavan_fb_get_draw_point_function(desc);
	if (draw_point_handle == NULL)
	{
		return -1;
	}

	cavan_build_line_equation(x1, y1, x2, y2, &a, &b);

	if (x1 < x2)
	{
		while (x1 <= x2)
		{
			draw_point_handle(desc, x1, a * x1 + b, color);
			x1++;
		}
	}
	else
	{
		while (x1 >= x2)
		{
			draw_point_handle(desc, x1, a * x1 + b, color);
			x1--;
		}
	}

	return 0;
}

static int cavan_fb_draw_line_vertical(struct cavan_fb_descriptor *desc, int x1, int y1, int x2, int y2, u32 color)
{
	double a, b;
	void (*draw_point_handle)(struct cavan_fb_descriptor *, int, int, u32);

	draw_point_handle = cavan_fb_get_draw_point_function(desc);
	if (draw_point_handle == NULL)
	{
		return -1;
	}

	cavan_build_line_equation(y1, x1, y2, x2, &a, &b);

	if (y1 < y2)
	{
		while (y1 <= y2)
		{
			draw_point_handle(desc, a * y1 + b, y1, color);
			y1++;
		}
	}
	else
	{
		while (y1 >= y2)
		{
			draw_point_handle(desc, a * y1 + b, y1, color);
			y1--;
		}
	}

	return 0;
}

int cavan_fb_draw_line(struct cavan_fb_descriptor *desc, int x1, int y1, int x2, int y2, u32 color)
{
	int ret;

	if (x1 < 0 || x1 >= desc->xres || x2 < 0 || x2 >= desc->xres)
	{
		return -EINVAL;
	}

	if (y1 < 0 || y1 >= desc->yres || y2 < 0 || y2 >= desc->yres)
	{
		return -EINVAL;
	}

	ret = cavan_fb_draw_line_horizon(desc, x1, y1, x2, y2, color);

	return ret < 0 ? ret : cavan_fb_draw_line_vertical(desc, x1, y1, x2, y2, color);
}

int cavan_fb_draw_rect(struct cavan_fb_descriptor *desc, int left, int top, int width, int height, u32 color)
{
	int right, bottom;
	int i;
	void (*draw_point_handle)(struct cavan_fb_descriptor *, int, int, u32);

	if (left < 0 || top < 0)
	{
		return -EINVAL;
	}

	right = left + width - 1;
	bottom = top + height - 1;

	if (right >= desc->xres || bottom >= desc->yres)
	{
		return -EINVAL;
	}

	draw_point_handle = cavan_fb_get_draw_point_function(desc);
	if (draw_point_handle == NULL)
	{
		return -1;
	}

	for (i = left; i <= right; i++)
	{
		draw_point_handle(desc, i, top, color);
		draw_point_handle(desc, i, bottom, color);
	}

	for (i = top; i <= bottom; i++)
	{
		draw_point_handle(desc, left, i, color);
		draw_point_handle(desc, right, i, color);
	}

	return 0;
}

int cavan_fb_fill_rect(struct cavan_fb_descriptor *desc, int left, int top, int width, int height, u32 color)
{
	int right, bottom;
	int x, y;
	void (*draw_point_handle)(struct cavan_fb_descriptor *, int, int, u32);

	if (left < 0 || top < 0)
	{
		return -EINVAL;
	}

	right = left + width - 1;
	bottom = top + height - 1;

	if (right >= desc->xres || bottom >= desc->yres)
	{
		return -EINVAL;
	}

	draw_point_handle = cavan_fb_get_draw_point_function(desc);
	if (draw_point_handle == NULL)
	{
		return -1;
	}

	for (y = top; y <= bottom; y++)
	{
		for (x = left; x <= right; x++)
		{
			draw_point_handle(desc, x, y, color);
		}
	}

	return 0;
}

int cavan_fb_draw_circle(struct cavan_fb_descriptor *desc, int x, int y, int r, u32 color)
{
	int rr;
	int i;
	int tmp;
	void (*draw_point_handle)(struct cavan_fb_descriptor *, int, int, u32);

	if (x - r < 0 || x + r >= desc->xres || y - r < 0 || y + r >= desc->yres)
	{
		return -EINVAL;
	}

	draw_point_handle = cavan_fb_get_draw_point_function(desc);
	if (draw_point_handle == NULL)
	{
		return -1;
	}

	rr = r * r;

	for (i = 0; i < r; i++)
	{
		tmp = sqrt(rr - i * i);

		draw_point_handle(desc, x + i, y + tmp, color);
		draw_point_handle(desc, x + i, y - tmp, color);
		draw_point_handle(desc, x - i, y + tmp, color);
		draw_point_handle(desc, x - i, y - tmp, color);

		draw_point_handle(desc, x + tmp, y + i, color);
		draw_point_handle(desc, x + tmp, y - i, color);
		draw_point_handle(desc, x - tmp, y + i, color);
		draw_point_handle(desc, x - tmp, y - i, color);
	}

	return 0;
}

int cavan_fb_fill_circle(struct cavan_fb_descriptor *desc, int x, int y, int r, u32 color)
{
	int rr;
	int i;
	int tmp, left, right, top, bottom;
	void (*draw_point_handle)(struct cavan_fb_descriptor *, int, int, u32);

	if (x - r < 0 || x + r >= desc->xres || y - r < 0 || y + r >= desc->yres)
	{
		return -EINVAL;
	}

	draw_point_handle = cavan_fb_get_draw_point_function(desc);
	if (draw_point_handle == NULL)
	{
		return -1;
	}

	rr = r * r;

	for (i = 0; i < r; i++)
	{
		tmp = sqrt(rr - i * i);

		for (left = x - i, right = x + i, top = y + tmp, bottom = y - tmp; top <= bottom; top++)
		{
			draw_point_handle(desc, left, top, color);
			draw_point_handle(desc, right, top, color);
		}

		for (left = x - tmp, right = x + tmp, top = y - i, bottom = y + i; left <= right; left++)
		{
			draw_point_handle(desc, left, top, color);
			draw_point_handle(desc, left, bottom, color);
		}
	}

	return 0;
}

int cavan_fb_draw_ellipse(struct cavan_fb_descriptor *desc, int x, int y, int width, int height, u32 color)
{
	double aa, bb;
	int tmp;
	int i;
	void (*draw_point_handle)(struct cavan_fb_descriptor *, int, int, u32);

	aa = ((double)width) / 2;
	bb = ((double)height) / 2;

	if (x - aa < 0 || x + aa >= desc->xres || y - bb < 0 || y + bb >= desc->yres)
	{
		return -EINVAL;
	}

	aa *= aa;
	bb *= bb;

	draw_point_handle = cavan_fb_get_draw_point_function(desc);
	if (draw_point_handle == NULL)
	{
		return -1;
	}

	for (i = width >> 1; i >= 0; i--)
	{
		tmp = sqrt(bb - (bb * i * i / aa));

		draw_point_handle(desc, x + i, y + tmp, color);
		draw_point_handle(desc, x + i, y - tmp, color);
		draw_point_handle(desc, x - i, y + tmp, color);
		draw_point_handle(desc, x - i, y - tmp, color);
	}

	for (i = height >> 1; i >= 0; i--)
	{
		tmp = sqrt(aa - (aa * i * i / bb));

		draw_point_handle(desc, x + tmp, y + i, color);
		draw_point_handle(desc, x + tmp, y - i, color);
		draw_point_handle(desc, x - tmp, y + i, color);
		draw_point_handle(desc, x - tmp, y - i, color);
	}

	return 0;
}

int cavan_fb_fill_ellipse(struct cavan_fb_descriptor *desc, int x, int y, int width, int height, u32 color)
{
	double aa, bb;
	int tmp, left, right, top, bottom;
	int i;
	void (*draw_point_handle)(struct cavan_fb_descriptor *, int, int, u32);

	aa = ((double)width) / 2;
	bb = ((double)height) / 2;

	if (x - aa < 0 || x + aa >= desc->xres || y - bb < 0 || y + bb >= desc->yres)
	{
		return -EINVAL;
	}

	aa *= aa;
	bb *= bb;

	draw_point_handle = cavan_fb_get_draw_point_function(desc);
	if (draw_point_handle == NULL)
	{
		return -1;
	}

	for (i = width >> 1; i >= 0; i--)
	{
		tmp = sqrt(bb - (bb * i * i / aa));

		for (left = x - i, right = x + i, top = y - tmp, bottom = y + tmp; top <= bottom; top++)
		{
			draw_point_handle(desc, left, y + tmp, color);
			draw_point_handle(desc, right, y + tmp, color);
		}
	}

	for (i = height >> 1; i >= 0; i--)
	{
		tmp = sqrt(aa - (aa * i * i / bb));

		for (left = x - tmp, right = x + tmp, top = y - i, bottom = y + i; left <= right; left++)
		{
			draw_point_handle(desc, left, top, color);
			draw_point_handle(desc, left, bottom, color);
		}
	}

	return 0;
}

int cavan_fb_draw_polygon(struct cavan_fb_descriptor *desc, struct cavan_point *points, size_t count, u32 color)
{
	int i;
	int ret;

	if (count < 3)
	{
		return -EINVAL;
	}

	for (i = 0, count--; i < count; i++)
	{
		ret = cavan_fb_draw_line(desc, points[i].x, points[i].y, points[i + 1].x, points[i + 1].y, color);
		if (ret < 0)
		{
			return ret;
		}
	}

	return cavan_fb_draw_line(desc, points[0].x, points[0].y, points[count].x, points[count].y, color);
}

int max3i(int a, int b, int c)
{
	if (b > a)
	{
		a = b;
	}

	return c > a ? c : a;
}

int min3i(int a, int b, int c)
{
	if (b < a)
	{
		a = b;
	}

	return c < a ? c : a;
}

void show_cavan_points(const struct cavan_point *points, size_t size)
{
	const struct cavan_point *end;

	for (end = points + size; points < end; points++)
	{
		println("[%d, %d]", points->x, points->y);
	}
}

void cavan_point_sort_x(struct cavan_point *start, struct cavan_point *end)
{
	struct cavan_point mid, *start_bak, *end_bak;

	if (start >= end)
	{
		return;
	}

	mid = *start;
	start_bak = start;
	end_bak = end;

	while (1)
	{
		for (; start < end && mid.x < end->x; end--);
		if (start < end)
		{
			*start++ = *end;
		}
		else
		{
			break;
		}

		for (; start < end && mid.x > start->x; start++);
		if (start < 0)
		{
			*end-- = *start;
		}
		else
		{
			break;
		}
	}

	*start = mid;

	if (start_bak < start)
	{
		cavan_point_sort_x(start_bak, start - 1);
	}

	if (end_bak > end)
	{
		cavan_point_sort_x(end + 1, end_bak);
	}
}

static int cavan_fb_fill_triangle_half(struct cavan_fb_descriptor *desc, int left, int right, double a1, double b1, double a2, double b2, u32 color)
{
	int min, max;
	void (*draw_point_handle)(struct cavan_fb_descriptor *, int, int, u32);

	draw_point_handle = cavan_fb_get_draw_point_function(desc);
	if (draw_point_handle == NULL)
	{
		return -1;
	}

	while (left <= right)
	{
		min = a1 * left + b1;
		max = a2 * left + b2;

		while (min <= max)
		{
			draw_point_handle(desc, left, min, color);
			min++;
		}

		left++;
	}

	return 0;
}

int cavan_fb_fill_triangle(struct cavan_fb_descriptor *desc, struct cavan_point *points, u32 color)
{
	double a[3], b[3];

	cavan_point_sort_x(points, points + 2);
	cavan_build_line_equation(points[0].x, points[0].y, points[1].x, points[1].y, a, b);
	cavan_build_line_equation(points[2].x, points[2].y, points[1].x, points[1].y, a + 1, b + 1);
	cavan_build_line_equation(points[0].x, points[0].y, points[2].x, points[2].y, a + 2, b + 2);

	if (points[1].y < points[2].y)
	{
		cavan_fb_fill_triangle_half(desc, points[0].x, points[1].x, a[0], b[0], a[2], b[2], color);
		cavan_fb_fill_triangle_half(desc, points[1].x, points[2].x, a[1], b[1], a[2], b[2], color);
	}
	else
	{
		cavan_fb_fill_triangle_half(desc, points[0].x, points[1].x, a[2], b[2], a[0], b[0], color);
		cavan_fb_fill_triangle_half(desc, points[1].x, points[2].x, a[2], b[2], a[1], b[1], color);
	}

	return 0;
}

int cavan_fb_fill_polygon(struct cavan_fb_descriptor *desc, struct cavan_point *points, size_t count, u32 color)
{
	int ret;
	struct cavan_point *p, *p_end;
	struct cavan_point point_buff[3];

	if (count < 3)
	{
		return -EINVAL;
	}


	for (p = points + 1, p_end = p + count - 2; p < p_end; p++)
	{
		point_buff[0] = points[0];
		point_buff[1] = p[0];
		point_buff[2] = p[1];

		ret = cavan_fb_fill_triangle(desc, point_buff, color);
		if (ret < 0)
		{
			return ret;
		}
	}

	return 0;
}
