#include <cavan.h>
#include <linux/fb.h>
#include <cavan/bmp.h>
#include <cavan/file.h>
#include <cavan/fb.h>

void bmp_show_file_header(struct bmp_file_header *file_hdr)
{
	print_sep(60);
	println("file_hdr->type = %c%c", file_hdr->type[0], file_hdr->type[1]);
	println("file_hdr->size = %s", size2text(file_hdr->size));
	println("file_hdr->offset = %d", file_hdr->offset);
}

void bmp_show_info_header(struct bmp_info_header *info_hdr)
{
	print_sep(60);
	println("info_hdr->size = %d", info_hdr->size);
	println("info_hdr->width = %d", info_hdr->width);
	println("info_hdr->height = %d", info_hdr->height);
	println("info_hdr->planes = %d", info_hdr->planes);
	println("info_hdr->bit_count = %d", info_hdr->bit_count);
	println("info_hdr->compress = %d", info_hdr->compress);
	println("info_hdr->size_image = %s", size2text(info_hdr->size_image));
	println("info_hdr->x_pels_per_meter = %d", info_hdr->x_pels_per_meter);
	println("info_hdr->y_pels_per_meter = %d", info_hdr->y_pels_per_meter);
	println("info_hdr->clr_used = %d", info_hdr->clr_used);
	println("info_hdr->clr_important = %d", info_hdr->clr_important);
}

int bmp_read_file_header(int fd, struct bmp_file_header *file_hdr)
{
	int ret;

	ret = ffile_read(fd, file_hdr, sizeof(*file_hdr));
	if (ret < 0) {
		pr_err_info("read");
		return ret;
	}

	if (file_hdr->type[0] != 'B' || file_hdr->type[1] != 'M') {
		pr_err_info("This File Is Not A BMP File");
		return -1;
	}

	bmp_show_file_header(file_hdr);

	return ret;
}

int bmp_read_info_header(int fd, struct bmp_info_header *info_hdr)
{
	int ret;

	ret = ffile_read(fd, info_hdr, sizeof(*info_hdr));
	if (ret < 0) {
		pr_err_info("read");
		return ret;
	}

	bmp_show_info_header(info_hdr);

	return ret;
}

#define CAVAN_FB_DRAW(type) \
	int cavan_fb_bmp_draw##type(struct cavan_fb_device *fb_dev, int fd, int width, int height) { \
		int ret; \
		int x, y; \
		struct pixel##type buff[width]; \
		for (y = height - 1; y >= 0; y--) { \
			ret = read(fd, &buff, sizeof(buff)); \
			if (ret < 0) { \
				pr_err_info("read"); \
				return ret; \
			} \
			for (x = 0; x < width; x++) { \
				cavan_display_color_t color; \
				color = cavan_fb_build_color(fb_dev, buff[x].red, buff[x].green, buff[x].blue, 0xFF); \
				fb_dev->draw_point(fb_dev, x, y, color.value); \
			} \
		} \
		return 0; \
	}

static CAVAN_FB_DRAW(555);
static CAVAN_FB_DRAW(565);
static CAVAN_FB_DRAW(888);
static CAVAN_FB_DRAW(8888);

int cavan_fb_bmp_view(struct cavan_fb_device *fb_dev, const char *pathname)
{
	int fd;
	int ret;
	struct bmp_file_header file_hdr;
	struct bmp_info_header info_hdr;

	println("pathname = %s", pathname);

	fd = open(pathname, O_RDONLY | O_BINARY);
	if (fd < 0) {
		pr_err_info("open");
		return fd;
	}

	ret = bmp_read_file_header(fd, &file_hdr);
	if (ret < 0) {
		pr_err_info("read_file_header");
		goto out_close_fd;
	}

	ret = bmp_read_info_header(fd, &info_hdr);
	if (ret < 0) {
		pr_err_info("read_info_header");
		goto out_close_fd;
	}

	ret = lseek(fd, file_hdr.offset, SEEK_SET);
	if (ret < 0) {
		pr_err_info("lseek");
		goto out_close_fd;
	}

	switch (info_hdr.bit_count) {
	case 16:
		if (info_hdr.compress) {
			println("picture format is: RGB565");
			cavan_fb_bmp_draw565(fb_dev, fd, info_hdr.width, info_hdr.height);
		} else {
			println("picture format is: RGB555");
			cavan_fb_bmp_draw555(fb_dev, fd, info_hdr.width, info_hdr.height);
		}
		break;
	case 24:
		println("picture format is: RGB888");
		cavan_fb_bmp_draw888(fb_dev, fd, info_hdr.width, info_hdr.height);
		break;
	case 32:
		println("picture format is: RGB8888");
		cavan_fb_bmp_draw8888(fb_dev, fd, info_hdr.width, info_hdr.height);
		break;
	default:
		pr_err_info("Unknown bit_count");
	}

	cavan_fb_refresh(fb_dev);

out_close_fd:
	close(fd);
	return ret;
}

int cavan_fb_bmp_view2(const char *pathname, const char *fb_path)
{
	int ret;
	struct cavan_fb_device fb_dev;

	ret = cavan_fb_init(&fb_dev, fb_path);
	if (ret < 0) {
		pr_red_info("cavan_fb_init");
		return ret;
	}

	ret = cavan_fb_bmp_view(&fb_dev, pathname);

	cavan_fb_deinit(&fb_dev);

	return ret;
}

size_t bmp_get_color_table_size(int bit_count)
{
	if (bit_count > 8) {
		return 0;
	}

	return 1 << bit_count;
}

void bmp_header_init(struct bmp_header *header, int width, int height, int bit_count)
{
	size_t table_size;
	struct bmp_file_header *file_hdr = &header->file_hdr;
	struct bmp_info_header *info_hdr = &header->info_hdr;

	table_size = bmp_get_color_table_size(bit_count) * sizeof(struct bmp_color_table_entry);

	info_hdr->size = sizeof(*info_hdr);
	info_hdr->width = width;
	info_hdr->height = height;
	info_hdr->planes = 1;
	info_hdr->bit_count = bit_count;
	info_hdr->compress = 0;
	info_hdr->size_image = width * height * bit_count / 8;
	info_hdr->x_pels_per_meter = 2834;
	info_hdr->y_pels_per_meter = 2834;
	info_hdr->clr_used = 0;
	info_hdr->clr_important = 0;

	file_hdr->type[0] = 'B';
	file_hdr->type[1] = 'M';
	file_hdr->size = info_hdr->size_image + sizeof(*header) + table_size;
	memset(file_hdr->reserved, 0, sizeof(file_hdr->reserved));
	file_hdr->offset = file_hdr->size - info_hdr->size_image;
}

int cavan_fb_bmp_capture(struct cavan_fb_device *dev, int fd)
{
	ssize_t wrlen;
	struct bmp_header header;

	bmp_header_init(&header, dev->xres, dev->yres, 24);

	wrlen = ffile_write(fd, &header, sizeof(header));
	if (wrlen < 0) {
		pr_err_info("ffile_write");
		return wrlen;
	}

	return cavan_fb_capture_file(dev, fd);
}

int cavan_fb_bmp_capture2(struct cavan_fb_device *dev, const char *pathname)
{
	int fd;
	int ret;

	fd = open(pathname, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd < 0) {
		pr_err_info("open file `%s'", pathname);
		return fd;
	}

	ret = cavan_fb_bmp_capture(dev, fd);

	close(fd);

	return ret;
}

int cavan_fb_bmp_capture3(const char *pathname)
{
	int ret;
	struct cavan_fb_device dev;

	ret = cavan_fb_init(&dev, NULL);
	if (ret < 0) {
		pr_red_info("cavan_fb_init");
		return ret;
	}

	ret = cavan_fb_bmp_capture2(&dev, pathname);

	cavan_fb_deinit(&dev);

	return ret;
}
