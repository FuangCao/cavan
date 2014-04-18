#pragma once

#include <cavan.h>
#include <linux/fb.h>

#pragma pack(1)
struct bmp_file_header
{
	u8 type[2];
	u32 size;
	u8 reserved[4];
	u32 offset;
};

struct bmp_info_header
{
	u32 size;
	u32 width;
	u32 height;
	u16 planes;
	u16 bit_count;
	u32 compress;
	u32 size_image;
	u32 x_pels_per_meter;
	u32 y_pels_per_meter;
	u32 clr_used;
	u32 clr_important;
};

struct bmp_color_table_entry
{
	u8 blue;
	u8 green;
	u8 red;
	u8 reserved;
};

struct bmp_header
{
	struct bmp_file_header file_hdr;
	struct bmp_info_header info_hdr;
	struct bmp_color_table_entry color_table[0];
};

struct pixel555
{
	u16 red		:5;
	u16 green	:5;
	u16 blue	:5;
};

struct pixel565
{
	u16 red		:5;
	u16 green	:6;
	u16 blue	:5;
};

struct pixel888
{
	u32 red		:8;
	u32 green	:8;
	u32 blue	:8;
};

struct pixel8888
{
	u32 red		:8;
	u32 green	:8;
	u32 blue	:8;
	u32 transp	:8;
};
#pragma pack()

int bmp_view(const char *file_name, const char *fb_name);
void bmp_show_file_header(struct bmp_file_header *file_hdr);
void bmp_show_info_header(struct bmp_info_header *info_hdr);
int bmp_read_file_header(int fd, struct bmp_file_header *file_hdr);
int bmp_read_info_header(int fd, struct bmp_info_header *info_hdr);
size_t bmp_get_color_table_size(int bit_count);
void bmp_header_init(struct bmp_header *header, int width, int height, int bit_count);
