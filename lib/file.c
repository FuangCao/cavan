#include <cavan.h>
#include <sys/stat.h>
#include <cavan/file.h>
#include <cavan/progress.h>
#include <cavan/text.h>
#include <cavan/parser.h>
#include <linux/loop.h>
#include <cavan/device.h>
#include <cavan/memory.h>

#define MAX_BUFF_LEN	MB(1)
#define MIN_FILE_SIZE	KB(1)

const static u32 crc16_table[256] =
{
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
	0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
	0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
	0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
	0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
	0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
	0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
	0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
	0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
	0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
	0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
	0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
	0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
	0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
	0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
	0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
	0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
	0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
	0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
	0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
	0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
	0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
	0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
	0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
	0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
	0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
	0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
	0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
	0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
	0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0,
};

const static u32 crc32_table[256] =
{
	0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
	0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
	0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
	0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
	0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
	0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
	0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
	0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
	0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
	0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
	0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
	0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
	0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
	0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
	0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
	0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
	0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
	0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
	0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
	0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
	0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
	0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
	0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
	0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
	0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
	0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
	0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
	0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
	0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
	0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
	0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
	0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
	0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
	0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
	0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
	0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
	0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
	0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
	0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
	0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
	0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
	0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
	0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
	0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
	0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
	0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
	0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
	0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
	0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
	0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
	0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
	0x2d02ef8dL
};

int file_join(const char *dest_file, char *src_files[], int count)
{
	int ret;
	int i;
	int src_fd, dest_fd;
	struct stat st;
	struct progress_bar bar;

	dest_fd = open(dest_file, O_WRONLY | O_CREAT | O_SYNC | O_TRUNC | O_BINARY, 0777);
	if (dest_fd < 0)
	{
		print_error("open \"%s\"", dest_file);
		return -1;
	}

	for (i = 0; i < count; i++)
	{
		println("\"%s\" [join]-> \"%s\"", src_files[i], dest_file);

		src_fd = open(src_files[i], O_RDONLY | O_BINARY);
		if (src_fd < 0)
		{
			ret = -1;
			print_error("open \"%s\"", src_files[i]);
			goto out_close_dest;
		}

		ret = fstat(src_fd, &st);
		if (ret < 0)
		{
			print_error("fstat \"%s\"", src_files[i]);
			goto out_close_src;
		}

		progress_bar_init(&bar, st.st_size);

		while (1)
		{
			size_t readlen, writelen;
			char buff[MAX_BUFF_LEN];

			readlen = read(src_fd, buff, sizeof(buff));
			if (readlen < 0)
			{
				ret = readlen;
				print_error("read \"%s\"", src_files[i]);
				goto out_close_src;
			}

			if (readlen == 0)
			{
				break;
			}

			writelen = write(dest_fd, buff, readlen);
			if (writelen != readlen)
			{
				ret = -errno;
				print_error("write \"%s\"", dest_file);
				goto out_close_src;
			}

			progress_bar_add(&bar, ret);
		}

		close(src_fd);

		progress_bar_finish(&bar);
	}

	ret = 0;
	goto out_close_dest;

out_close_src:
	close(src_fd);
out_close_dest:
	close(dest_fd);

	return ret;
}

int file_split(const char *file_name, const char *dest_dir, int count)
{
	int ret;
	int i;
	int src_fd, dest_fd;
	size_t dest_size, remain_size;
	struct stat st;
	struct progress_bar bar;

	mkdir(dest_dir, 0);

	src_fd = open(file_name, O_RDONLY | O_BINARY);
	if (src_fd < 0)
	{
		print_error("open \"%s\"", file_name);
		return -1;
	}

	ret = fstat(src_fd, &st);
	if (ret < 0)
	{
		print_error("fstat \"%s\"", file_name);
		goto out_close_src;
	}

	progress_bar_init(&bar, st.st_size);

	dest_size = st.st_size / count;
	dest_size = (st.st_size + dest_size - 1) / count;

	for (i = 0; i < count; i++)
	{
		remain_size = dest_size;

		dest_fd = open(format_text("%s/%d", dest_dir, i), O_WRONLY | O_CREAT | O_TRUNC | O_SYNC | O_BINARY, 0777);
		if (dest_fd < 0)
		{
			print_error("open");
			goto out_close_dest;
		}

		while (remain_size)
		{
			ssize_t readlen, writelen;
			char buff[MAX_BUFF_LEN];

			readlen = read(src_fd, buff, remain_size > sizeof(buff) ? sizeof(buff) : remain_size);
			if (ret < 0)
			{
				ret = readlen;
				print_error("read");
				goto out_close_dest;
			}

			if (readlen == 0)
			{
				break;
			}

			writelen = write(dest_fd, buff, readlen);
			if (writelen != readlen)
			{
				ret = -errno;
				print_error("write");
				goto out_close_dest;
			}

			progress_bar_add(&bar, writelen);

			remain_size -= writelen;
		}

		close(dest_fd);
	}

	progress_bar_finish(&bar);

	ret = 0;
	goto out_close_src;

out_close_dest:
	close(dest_fd);
out_close_src:
	close(src_fd);

	return ret;
}

int ffile_copy_simple(int src_fd, int dest_fd)
{
	while (1)
	{
		ssize_t readlen, writelen;
		char buff[MAX_BUFF_LEN];

		readlen = read(src_fd, buff, sizeof(buff));
		if (readlen < 0)
		{
			print_error("read");
			return readlen;
		}

		if (readlen == 0)
		{
			break;
		}

		writelen = write(dest_fd, buff, readlen);
		if (writelen != readlen)
		{
			print_error("write");
			return -errno;
		}
	}

	return 0;
}

int ffile_copy(int src_fd, int dest_fd)
{
	int ret;
	struct stat st;
	struct progress_bar bar;

	ret = fstat(src_fd, &st);
	if (ret < 0 || st.st_size < MIN_FILE_SIZE)
	{
		return ffile_copy_simple(src_fd, dest_fd);
	}

	progress_bar_init(&bar, st.st_size);

	while (1)
	{
		ssize_t readlen, writelen;
		char buff[MAX_BUFF_LEN];

		readlen = read(src_fd, buff, sizeof(buff));
		if (readlen < 0)
		{
			print_error("read");
			return readlen;
		}

		if (readlen == 0)
		{
			break;
		}

		writelen = write(dest_fd, buff, readlen);
		if (writelen != readlen)
		{
			print_error("write");
			return -errno;
		}

		progress_bar_add(&bar, writelen);
	}

	progress_bar_finish(&bar);

	return 0;
}

int file_append(const char *file_src, const char *file_dest)
{
	int fd_src, fd_dest;
	int ret;

	fd_src = open(file_src, READ_FLAGS);
	if (fd_src < 0)
	{
		print_error("open \"%s\"", file_src);
		return -1;
	}

	fd_dest = open(file_dest, WRITE_FLAGS | O_APPEND, 0777);
	if (fd_dest < 0)
	{
		ret = -1;
		print_error("open \"%s\"", file_dest);
		goto out_close_fd_src;
	}

	ret = ffile_copy(fd_src, fd_dest);

	close(fd_dest);
out_close_fd_src:
	close(fd_src);

	return ret;
}

int file_open_rw_ro(const char *pathname, int flags)
{
	int fd;

	fd = open(pathname, O_RDWR | O_BINARY | flags);
	if (fd >= 0)
	{
		return fd;
	}

#ifdef CAVAN_DEBUG
	warning_msg("rw open file \"%s\" faild, retry use ro", pathname);
#endif

	fd = open(pathname, O_RDONLY | flags);

#ifdef CAVAN_DEBUG
	if (fd < 0)
	{
		print_error("open file \"%s\"", pathname);
	}
#endif

	return fd;
}

static int open_files(const char *src_file, const char *dest_file, int *src_fd, int *dest_fd, int flags)
{
	int ret;
	struct stat st;

	*src_fd = open(src_file, O_RDONLY);
	if (*src_fd < 0)
	{
		print_error("open \"%s\"", src_file);
		return *src_fd;
	}

	ret = fstat(*src_fd, &st);
	if (ret < 0)
	{
		print_error("fstat \"%s\"", src_file);
		goto out_close_src_fd;
	}

	*dest_fd = open(dest_file, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY | flags, st.st_mode);
	if (*dest_fd < 0)
	{
		ret = *dest_fd;
		print_error("open \"%s\"", dest_file);
		goto out_close_src_fd;
	}

	return 0;

out_close_src_fd:
	close(*src_fd);

	return ret;
}

int file_copy(const char *src_file, const char *dest_file, int flags)
{
	int src_fd, dest_fd;
	int ret;

	ret = open_files(src_file, dest_file, &src_fd, &dest_fd, flags);
	if (ret < 0)
	{
		error_msg("open_files");
		return ret;
	}

	ret = ffile_copy(src_fd, dest_fd);
	if (ret < 0)
	{
		error_msg("ffile_copy");
	}

	close(src_fd);
	close(dest_fd);

	return ret;
}

off_t ffile_get_size(int fd)
{
	int ret;
	struct stat st;

	ret = fstat(fd, &st);
	if (ret < 0)
	{
		print_error("fstat");
		return 0;
	}

	return st.st_size;
}

off_t file_get_size(const char *filepath)
{
	int ret;
	struct stat st;

	ret = file_stat2(filepath, &st);
	if (ret < 0)
	{
		print_error("file_stat \"%s\"", filepath);
		return 0;
	}

	return st.st_size;
}

int mkdir_all(const char *pathname)
{
	int ret;
	char dir_name[512];

	if (file_test(pathname, "d") == 0)
	{
		return 0;
	}

	text_dirname_base(dir_name, pathname);
	ret = mkdir_all(dir_name);
	if (ret < 0)
	{
		error_msg("create directory \"%s\" failed", dir_name);
		return ret;
	}

	return mkdir(pathname, 0777);
}

int mkdir_hierarchy(const char *pathname, mode_t mode)
{
	int ret;
	char temp_path[1024], *p;

	if (mkdir(pathname, mode) == 0 || errno == EEXIST)
	{
		return 0;
	}

	p = text_copy(temp_path, pathname) - 1;
	while (p > temp_path && *p == '/')
	{
		*p-- = 0;
	}

	p = temp_path;

	while (*p && *p == '/')
	{
		p++;
	}

	while (1)
	{
		while (*p && *p != '/')
		{
			p++;
		}

		if (*p)
		{
			*p = 0;
		}
		else
		{
			break;
		}

		ret = mkdir(temp_path, mode);
		if (ret < 0 && errno != EEXIST)
		{
			return ret;
		}
		*p++ = '/';
	}

	ret = mkdir(pathname, mode);
	if (ret < 0 && errno != EEXIST)
	{
		return ret;
	}

	return 0;
}

int file_create_open(const char *pathname, int flags, mode_t mode)
{
	int ret;

	ret = open(pathname, O_CREAT | O_BINARY | flags, mode);
	if (ret >= 0)
	{
		return ret;
	}

	ret = mkdir_hierarchy(text_dirname(pathname), mode);
	if (ret < 0)
	{
		return ret;
	}

	return open(pathname, O_CREAT | O_BINARY | flags, mode);
}

int ffile_ncopy_simple(int src_fd, int dest_fd, size_t size)
{
	while (size)
	{
		ssize_t readlen, writelen;
		char buff[MAX_BUFF_LEN];

		readlen = read(src_fd, buff, size > sizeof(buff) ? sizeof(buff) : size);
		if (readlen < 0)
		{
			print_error("read");
			return readlen;
		}

		if (readlen == 0)
		{
			break;
		}

		writelen = write(dest_fd, buff, readlen);
		if (writelen != readlen)
		{
			print_error("write");
			return -errno;
		}

		size -= writelen;
	}

	return 0;
}

int ffile_ncopy(int src_fd, int dest_fd, size_t size)
{
	struct progress_bar bar;

	if (size < MIN_FILE_SIZE)
	{
		return ffile_ncopy_simple(src_fd, dest_fd, size);
	}

	progress_bar_init(&bar, size);

	while (size)
	{
		ssize_t readlen, writelen;
		char buff[MAX_BUFF_LEN];

		readlen = read(src_fd, buff, size < sizeof(buff) ? size : sizeof(buff));
		if (readlen < 0)
		{
			print_error("read");
			return readlen;
		}

		if (readlen == 0)
		{
			break;
		}

		writelen = write(dest_fd, buff, readlen);
		if (writelen != readlen)
		{
			print_error("write");
			return -errno;
		}

		progress_bar_add(&bar, writelen);
		size -= writelen;
	}

	progress_bar_finish(&bar);

	if (size)
	{
		warning_msg("size != 0");
	}

	return 0;
}

int file_ncopy(const char *src_file, const char *dest_file, size_t size, int flags)
{
	int src_fd, dest_fd;
	int ret;

	ret = open_files(src_file, dest_file, &src_fd, &dest_fd, flags);
	if (ret < 0)
	{
		error_msg("open_files");
		return ret;
	}

	ret = ffile_ncopy(src_fd, dest_fd, size);
	if (ret < 0)
	{
		error_msg("ffile_ncopy");
	}

	close(src_fd);
	close(dest_fd);

	return ret;
}

int vtry_to_open(int flags, va_list ap)
{
	while (1)
	{
		int fd;
		const char *filename;

		filename = va_arg(ap, const char *);
		if (filename == NULL)
		{
			return -ENOENT;
		}

		println("open file \"%s\"", filename);

		fd = open(filename, flags);
		if (fd >= 0)
		{
			return fd;
		}
	}

	return -1;
}

int try_to_open(int flags, ...)
{
	int ret;
	va_list ap;

	va_start(ap, flags);
	ret = vtry_to_open(flags, ap);
	va_end(ap);

	return ret;
}

ssize_t ffile_read(int fd, void *buff, size_t size)
{
	void *buff_bak = buff, *buff_end = buff + size;

	while (buff < buff_end)
	{
		ssize_t readlen = read(fd, buff, buff_end - buff);

		if (readlen < 0)
		{
			return readlen;
		}

		if (readlen == 0)
		{
			return buff - buff_bak;
		}

		buff += readlen;
	}

	return buff - buff_bak;
}

ssize_t ffile_write(int fd, const void *buff, size_t size)
{
	const void *buff_bak = buff, *buff_end = buff + size;

	while (buff < buff_end)
	{
		ssize_t writelen = write(fd, buff, buff_end - buff);

		if (writelen < 0)
		{
			return writelen;
		}

		if (writelen == 0)
		{
			return buff - buff_bak;
		}

		buff += writelen;
	}

	return buff - buff_bak;
}

ssize_t ffile_writeto(int fd, const void *buff, size_t size, off_t offset)
{
	if (offset > 0)
	{
		offset = lseek(fd, offset, SEEK_SET);
		if (offset < 0)
		{
#ifdef CAVAN_DEBUG
			print_error("lseek");
#endif
			return offset;
		}
	}

	return ffile_write(fd, buff, size);
}

ssize_t file_writeto(const char *file_name, const void *buff, size_t size, off_t offset, int flags)
{
	ssize_t writelen;
	int fd;

#ifdef CAVAN_DEBUG
	println("write file = %s", file_name);
	println("size = %s", size2text(size));
	println("offset = %s", size2text(offset));
#endif

	fd = file_create_open(file_name, O_WRONLY | O_SYNC | flags, 0777);
	if (fd < 0)
	{
#ifdef CAVAN_DEBUG
		print_error("open \"%s\"", file_name);
#endif
		return -1;
	}

	writelen = ffile_writeto(fd, buff, size, offset);

	close(fd);

	return writelen;
}

ssize_t ffile_readfrom(int fd, void *buff, size_t size, off_t offset)
{
	if (offset > 0)
	{
		offset = lseek(fd, offset, SEEK_SET);
		if (offset < 0)
		{
			print_error("lseek");
			return offset;
		}
	}

	return ffile_read(fd, buff, size);
}

ssize_t file_readfrom(const char *file_name, void *buff, size_t size, off_t offset, int flags)
{
	ssize_t readlen;
	int fd;

#ifdef CAVAN_DEBUG
	println("read file = %s", file_name);
	println("size = %s", size2text(size));
	println("offset = %s", size2text(offset));
#endif

	fd = open(file_name, O_RDONLY | O_BINARY | flags);
	if (fd < 0)
	{
#ifdef CAVAN_DEBUG
		print_error("open \"%s\"", file_name);
#endif
		return -1;
	}

	readlen = ffile_readfrom(fd, buff, size, offset);

	close(fd);

	return readlen;
}

int file_test_read(const char *filename)
{
	char buff[1024];

	return file_readfrom(filename, buff, sizeof(buff), 0, 0);
}

int ffile_show(int fd)
{
	while (1)
	{
		ssize_t readlen;
		char buff[MAX_BUFF_LEN];

		readlen = read(fd, buff, sizeof(buff));
		if (readlen < 0)
		{
			print_error("read");
			return readlen;
		}

		if (readlen == 0)
		{
			break;
		}

		text_show(buff, readlen);
	}

	print_char('\n');

	return 0;
}

int ffile_nshow(int fd, size_t size)
{
	if (size == 0)
	{
		return ffile_show(fd);
	}

	while (size)
	{
		ssize_t readlen;
		char buff[MAX_BUFF_LEN];

		readlen = read(fd, buff, size > sizeof(buff) ? sizeof(buff) : size);
		if (readlen < 0)
		{
			print_error("read");
			return readlen;
		}

		if (readlen == 0)
		{
			break;
		}

		text_show(buff, readlen);

		size -= readlen;
	}

	print_char('\n');

	return 0;
}

int file_operation_ro(const char *filename, int (*handle)(int fd))
{
	int fd;
	int ret;

	fd = file_open_ro(filename);
	if (fd < 0)
	{
		return fd;
	}

	ret = handle(fd);

	close(fd);

	return ret;
}

int file_noperation_ro(const char *filename, size_t size, int (*handle)(int fd, size_t size))
{
	int fd;
	int ret;

	fd = file_open_ro(filename);
	if (fd < 0)
	{
		return fd;
	}

	ret = handle(fd, size);

	close(fd);

	return ret;
}

int file_operation_wo(const char *filename, int (*handle)(int fd))
{
	int fd;
	int ret;

	fd = file_open_wo(filename);
	if (fd < 0)
	{
		return fd;
	}

	ret = handle(fd);

	close(fd);

	return ret;
}

int file_noperation_wo(const char *filename, size_t size, int (*handle)(int fd, size_t size))
{
	int fd;
	int ret;

	fd = file_open_wo(filename);
	if (fd < 0)
	{
		return fd;
	}

	ret = handle(fd, size);

	close(fd);

	return ret;
}

int file_operation_rw(const char *filename, int (*handle)(int fd))
{
	int fd;
	int ret;

	fd = file_open_ro(filename);
	if (fd < 0)
	{
		return fd;
	}

	ret = handle(fd);

	close(fd);

	return ret;
}

int file_noperation_rw(const char *filename, size_t size, int (*handle)(int fd, size_t size))
{
	int fd;
	int ret;

	fd = file_open_ro(filename);
	if (fd < 0)
	{
		return fd;
	}

	ret = handle(fd, size);

	close(fd);

	return ret;
}

int ffile_cat(int fd)
{
	while (1)
	{
		ssize_t readlen;
		char buff[MAX_BUFFER_LEN];

		readlen = read(fd, buff, sizeof(buff));
		if (readlen < 0)
		{
			print_error("read");
			return readlen;
		}

		if (readlen == 0)
		{
			break;
		}

		print_ntext(buff, readlen);
	}

	print_char('\n');

	return 0;
}

int ffile_ncat(int fd, size_t size)
{
	if (size == 0)
	{
		return ffile_cat(fd);
	}

	while (size)
	{
		ssize_t readlen;
		char buff[MAX_BUFFER_LEN];

		readlen = read(fd, buff, size > sizeof(buff) ? sizeof(buff) : size);
		if (readlen < 0)
		{
			print_error("read");
			return readlen;
		}

		if (readlen == 0)
		{
			break;
		}

		print_ntext(buff, readlen);
		size -= readlen;
	}

	print_char('\n');

	return 0;
}

int ffile_cmp(int fd1, int fd2, size_t size)
{
	if (ffile_get_size(fd1) != ffile_get_size(fd2))
	{
		return 1;
	}

	while (size)
	{
		ssize_t readlen;
		char buff1[MAX_BUFF_LEN];
		char buff2[MAX_BUFF_LEN];

		readlen = read(fd1, buff1, size > sizeof(buff1) ? sizeof(buff1) : size);
		if (readlen < 0)
		{
			print_error("read");
			return readlen;
		}

		if (readlen == 0)
		{
			break;
		}

		readlen = read(fd2, buff2, readlen);
		if (readlen < 0)
		{
			print_error("read");
			return readlen;
		}

		if (memcmp(buff1, buff2, readlen) != 0)
		{
			return 1;
		}

		size -= readlen;
	}

	return 0;
}

int file_cmp(const char *file1, const char *file2, size_t size)
{
	int ret;
	int fd1, fd2;

	fd1 = open(file1, READ_FLAGS);
	if (fd1 < 0)
	{
		print_error("open \"%s\"", file1);
		return -1;
	}

	fd2 = open(file2, READ_FLAGS);
	if (fd2 < 0)
	{
		ret = -1;
		print_error("open \"%s\"", file2);
		goto out_close_file1;
	}

	ret = ffile_cmp(fd1, fd2, size);
	if (ret < 0)
	{
		error_msg("ffile_cmp");
	}

	close(fd2);
out_close_file1:
	close(fd1);

	return ret;
}

u16 mem_crc16(u16 crc, const void *buff, size_t size)
{
	const void *buff_end = buff + size;

	while (buff < buff_end)
	{
		crc = crc16_table[((crc >> 8) ^ (*(char *)buff++)) & 0xFF] ^ (crc << 8);
	}

	return crc;
}

u32 mem_crc32(u32 crc, const void *buff, size_t size)
{
	const void *buff_end = buff + size;

	crc ^= 0xFFFFFFFFL;

	while (buff < buff_end)
	{
		crc = crc32_table[(crc ^ (*(char *)buff++)) & 0xFF] ^ (crc >> 8);
	}

	return crc ^ 0xFFFFFFFFL;
}

int ffile_crc32(int fd, u32 *crc)
{
	print("check file crc32 checksum ...");

	while (1)
	{
		ssize_t readlen;
		char buff[MAX_BUFF_LEN];

		readlen = read(fd, buff, sizeof(buff));
		if (readlen < 0)
		{
			print_error("read");
			return readlen;
		}

		if (readlen == 0)
		{
			break;
		}

		crc[0] = mem_crc32(crc[0], buff, readlen);

		print_char('.');
	}

	println(" OK");

	return 0;
}

int file_crc32(const char *file_name, u32 *crc)
{
	int fd;
	int ret;

	fd = open(file_name, READ_FLAGS);
	if (fd < 0)
	{
		print_error("open \"%s\"", file_name);
		return -1;
	}

	ret = ffile_crc32(fd, crc);
	if (ret < 0)
	{
		error_msg("ffile_crc32");
	}

	close(fd);

	return ret;
}

int ffile_ncrc32(int fd, size_t size, u32 *crc)
{
	struct progress_bar bar;

	progress_bar_init(&bar, size);

	while (size)
	{
		ssize_t readlen;
		char buff[MAX_BUFF_LEN];

		readlen = read(fd, buff, size > sizeof(buff) ? sizeof(buff) : size);
		if (readlen < 0)
		{
			print_error("read");
			return readlen;
		}

		if (readlen == 0)
		{
			break;
		}

		crc[0] = mem_crc32(crc[0], buff, readlen);
		size -= readlen;
		progress_bar_add(&bar, readlen);
	}

	progress_bar_finish(&bar);

	return 0;
}

int file_ncrc32(const char *file_name, size_t size, u32 *crc)
{
	int fd;
	int ret;

	fd = open(file_name, READ_FLAGS);
	if (fd < 0)
	{
		print_error("open \"%s\"", file_name);
		return -1;
	}

	ret = ffile_ncrc32(fd, size, crc);
	if (ret < 0)
	{
		error_msg("ffile_ncrc32");
	}

	close(fd);

	return ret;
}

int ffile_crc32_seek(int fd, off_t offset, int whence, u32 *crc)
{
	int ret;
	s64 offset_bak;

	offset_bak = lseek(fd, offset, whence);
	if (offset_bak < 0)
	{
		print_error("lseek");
		return -1;
	}

	ret = ffile_crc32(fd, crc);
	if (ret < 0)
	{
		error_msg("ffile_crc32");
		return ret;
	}

	ret = lseek(fd, offset_bak, SEEK_SET);
	if (ret < 0)
	{
		print_error("lseek");
		return ret;
	}

	return 0;
}

int file_crc32_seek(const char *file_name, off_t offset, int whence, u32 *crc)
{
	int fd;
	int ret;

	fd = open(file_name, READ_FLAGS);
	if (fd < 0)
	{
		print_error("open \"%s\"", file_name);
		return -1;
	}

	ret = ffile_crc32_seek(fd, offset, whence, crc);
	if (ret < 0)
	{
		error_msg("ffile_crc32_seek");
	}

	close(fd);

	return ret;
}

int ffile_ncrc32_seek(int fd, off_t offset, int whence, size_t size, u32 *crc)
{
	int ret;
	s64 offset_bak;

	offset_bak = lseek(fd, offset, whence);
	if (offset_bak < 0)
	{
		print_error("lseek");
		return -1;
	}

	ret = ffile_ncrc32(fd, size, crc);
	if (ret < 0)
	{
		error_msg("ffile_crc32");
		return ret;
	}

	ret = lseek(fd, offset_bak, SEEK_SET);
	if (ret < 0)
	{
		print_error("lseek");
		return ret;
	}

	return 0;
}

int file_ncrc32_seek(const char *file_name, off_t offset, int whence, size_t size, u32 *crc)
{
	int fd;
	int ret;

	fd = open(file_name, READ_FLAGS);
	if (fd < 0)
	{
		print_error("open \"%s\"", file_name);
		return -1;
	}

	ret = ffile_ncrc32_seek(fd, offset, whence, size, crc);
	if (ret < 0)
	{
		error_msg("ffile_ncrc32_seek");
	}

	close(fd);

	return ret;
}

int file_stat(const char *file_name, struct stat *st)
{
	int ret;
	int fd;

	fd = open(file_name, 0);
	if (fd < 0)
	{
		return fd;
	}

	ret = fstat(fd, st);

	close(fd);

	return ret;
}

int mode_test(mode_t mode, char type)
{
	switch (mode & S_IFMT)
	{
	case S_IFBLK:
		if (type == 'b' || type == 'B')
		{
			return 0;
		}
		break;
	case S_IFCHR:
		if (type == 'c' || type == 'C')
		{
			return 0;
		}
		break;
	case S_IFDIR:
		if (type == 'd' || type == 'D')
		{
			return 0;
		}
		break;
	case S_IFIFO:
		if (type == 'p' || type == 'P')
		{
			return 0;
		}
		break;
	case S_IFLNK:
		if (type == 'l' || type == 'L')
		{
			return 0;
		}
		break;
	case S_IFREG:
		if (type == 'f' || type == 'F')
		{
			return 0;
		}
		break;
	case S_IFSOCK:
		if (type == 's' || type == 'S')
		{
			return 0;
		}
		break;
	default:
		if (type == 'e' || type == 'E')
		{
			return 0;
		}
		warning_msg("unknown file type");
	}

	return -EINVAL;
}

mode_t ffile_get_mode(int fd)
{
	int ret;
	struct stat st;

	ret = fstat(fd, &st);
	if (ret < 0)
	{
		return 0;
	}

	return st.st_mode;
}

mode_t file_get_mode(const char *pathname)
{
	int ret;
	int fd;
	struct stat st;
	mode_t mode;

	ret = stat(pathname, &st);
	if (ret == 0)
	{
		return st.st_mode;
	}

	fd = open(pathname, 0);
	if (fd < 0)
	{
		return 0;
	}

	mode = ffile_get_mode(fd);

	close(fd);

	return mode;
}

mode_t file_get_lmode(const char *pathname)
{
	int ret;
	struct stat st;

	ret = lstat(pathname, &st);
	if (ret == 0)
	{
		return st.st_mode;
	}

	return file_get_mode(pathname);
}

int file_test(const char *file_name, const char *types)
{
	int ret;
	char buff[8], *p = buff;
	int access_mode = 0;
	mode_t st_mode;

	while (1)
	{
		switch (*types)
		{
		case 'w':
		case 'W':
			access_mode |= W_OK;
			break;

		case 'r':
		case 'R':
			access_mode |= R_OK;
			break;

		case 'e':
		case 'E':
			access_mode |= F_OK;
			break;

		case 'x':
		case 'X':
			access_mode |= X_OK;
			break;

		case 0:
			goto label_access;

		default:
			*p++ = *types;
		}

		types++;
	}

label_access:
	ret = access(file_name, access_mode);
	if (ret < 0)
	{
		return ret;
	}

	if (p == buff)
	{
		return 0;
	}

	st_mode = file_get_lmode(file_name);

	for (*p = 0, p = buff; *p; p++)
	{
		if (mode_test(st_mode, *p) == 0)
		{
			return 0;
		}
	}

	return -EINVAL;
}

int ffile_test(int fd, char type)
{
	int ret;
	struct stat st;

	ret = fstat(fd, &st);
	if (ret < 0)
	{
		return ret;
	}

	return mode_test(st.st_mode, type);
}

int file_wait(const char *filepath, const char *types, u32 sec)
{
	while (sec && file_test(filepath, types) < 0)
	{
		print("Wait remain time %d(s)  \r", sec);
		sleep(1);
		sec--;
	}

	return sec ? 0 : -ETIMEDOUT;
}

int file_resize(const char *file_path, off_t length)
{
	int ret;
	int fd;

	fd = open(file_path, O_CREAT | O_WRONLY | O_BINARY, 0777);
	if (fd < 0)
	{
		print_error("open file \"%s\"", file_path);
		return fd;
	}

	ret = ftruncate(fd, length);

	close(fd);

	return ret;
}

int get_file_pointer(int fd, off_t *fpointer)
{
	off_t ret;

	ret = lseek(fd, 0, SEEK_CUR);
	if (ret < 0)
	{
		print_error("lseek");
		return ret;
	}

	fpointer[0] = ret;

	return 0;
}

int calculate_file_md5sum(const char *file_path, char *md5sum)
{
#if 0
	int ret;

	ret = system_command("md5sum -b %s > " MD5SUM_FILE_PATH, file_path);
	if (ret < 0)
	{
		print_error("system_command");
		return ret;
	}

	return read_from(MD5SUM_FILE_PATH, md5sum, MD5SUM_LEN, 0, 0);
#else
	return buff_command2(md5sum, MD5SUM_LEN, "md5sum %s", file_path);
#endif
}

int check_file_md5sum(const char *file_path, char *md5sum)
{
	int ret;
	char buff[1024];

	memcpy(buff, md5sum, MD5SUM_LEN);
	buff[MD5SUM_LEN] = buff[MD5SUM_LEN + 1] = ' ';
	strcpy(buff + MD5SUM_LEN + 2, file_path);

	ret = file_writeto(MD5SUM_FILE_PATH, buff, strlen(buff), 0, O_TRUNC);
	if (ret < 0)
	{
		error_msg("write_to");
		return ret;
	}

	return system_command("md5sum -c " MD5SUM_FILE_PATH);
}

int file_replace_line_simple(const char *file_path, const char *prefix, off_t prefix_size, const char *new_line, off_t new_line_size)
{
	int ret;
	struct buffer *old_buff, *new_buff;

	old_buff = read_lines(file_path);
	if (old_buff == NULL)
	{
		return -1;
	}

	new_buff = replace_prefix_line(old_buff, prefix, prefix_size, new_line, new_line_size);
	if (new_buff == NULL)
	{
		ret = -1;
		goto out_free_old_buff;
	}

	ret = write_lines(file_path, new_buff);

	free_buffer(new_buff);
out_free_old_buff:
	free_buffer(old_buff);

	return ret;
}

int file_select_read(int fd, long timeout)
{
	fd_set set_read;
	struct timeval time;

	FD_ZERO(&set_read);
	FD_SET(fd, &set_read);

	time.tv_sec = timeout;
	time.tv_usec = 0;

	return select(fd + 1, &set_read, NULL, NULL, &time);
}

u32 mem_checksum32_simple(const void *mem, size_t count)
{
	const void *mem_end = mem + count;
	u64 checksum = 0;

	while (mem < mem_end)
	{
		checksum += *(char *)mem++;
	}

	checksum = (checksum >> 32) + (checksum & 0xFFFFFFFF);

	return (u32)((checksum >> 32) + checksum);
}

u16 mem_checksum16_simple(const void *mem, size_t count)
{
	u32 checksum = mem_checksum32_simple(mem, count);

	checksum = (checksum >> 16) + (checksum & 0xFFFF);

	return (u16)((checksum >> 16) + checksum);
}

u8 mem_checksum8_simple(const void *mem, size_t count)
{
	u16 checksum = mem_checksum16_simple(mem, count);

	checksum = (checksum >> 8) + (checksum & 0xFF);

	return (u8)((checksum >> 8) + checksum);
}

u32 ffile_checksum32_simple(int fd, off_t offset, size_t size)
{
	int ret;
	struct progress_bar bar;
	u64 checksum = 0;

	if (size == 0)
	{
		size = ffile_get_size(fd);
		if (size < 0)
		{
			error_msg("get file size failed");
			return size;
		}
	}

	if (offset >= size)
	{
		goto out_return;
	}

	if (offset > 0)
	{
		ret = lseek(fd, offset, SEEK_SET);
		if (ret < 0)
		{
			goto out_return;
		}

		size -= offset;
	}

	progress_bar_init(&bar, size);

	while (1)
	{
		ssize_t readlen;
		char buff[MAX_BUFF_LEN];

		readlen = read(fd, buff, sizeof(buff));
		if (readlen < 0)
		{
			ret = readlen;
			goto out_return;
		}

		if (readlen == 0)
		{
			break;
		}

		checksum += mem_checksum32_simple(buff, readlen);
		progress_bar_add(&bar, readlen);
	}

	progress_bar_finish(&bar);

out_return:
	checksum = (checksum >> 32) + (checksum & 0xFFFFFFFF);
	return (u32)((checksum >> 32) + checksum);
}

u16 ffile_checksum16_simple(int fd, off_t offset, size_t size)
{
	u32 checksum = ffile_checksum32_simple(fd, offset, size);

	checksum = (checksum >> 16) + (checksum & 0xFFFF);

	return (u16)((checksum >> 16) + checksum);
}

u8 ffile_checksum8_simple(int fd, off_t offset, size_t size)
{
	u16 checksum = ffile_checksum16_simple(fd, offset, size);

	checksum = (checksum >> 8) + (checksum & 0xFF);

	return (u8)((checksum >> 8) + checksum);
}

u32 file_checksum32(const char *filename, off_t offset, size_t size)
{
	int fd;
	u32 checksum;

	fd = file_open_ro(filename);
	if (fd < 0)
	{
		print_error("open file \"%s\"", filename);
		return 0;
	}

	checksum = ffile_checksum32(fd, offset, size);

	close(fd);

	return checksum;
}

u16 file_checksum16(const char *filename, off_t offset, size_t size)
{
	int fd;
	u16 checksum;

	fd = file_open_ro(filename);
	if (fd < 0)
	{
		print_error("open file \"%s\"", filename);
		return 0;
	}

	checksum = ffile_checksum16(fd, offset, size);

	close(fd);

	return checksum;
}

u8 file_checksum8(const char *filename, off_t offset, size_t size)
{
	int fd;
	u8 checksum;

	fd = file_open_ro(filename);
	if (fd < 0)
	{
		print_error("open file \"%s\"", filename);
		return 0;
	}

	checksum = ffile_checksum8(fd, offset, size);

	close(fd);

	return checksum;
}

ssize_t ffile_vprintf(int fd, const char *fmt, va_list ap)
{
	char buff[1024];
	ssize_t writelen;

	writelen = vsprintf(buff, fmt, ap);
	writelen = ffile_write(fd, buff, writelen);

	return writelen;
}

ssize_t ffile_prinf(int fd, const char *fmt, ...)
{
	ssize_t writelen;
	va_list ap;

	va_start(ap, fmt);
	writelen = ffile_vprintf(fd, fmt, ap);
	va_end(ap);

	return writelen;
}

ssize_t file_vprintf(const char *filename, const char *fmt, va_list ap)
{
	char buff[1024];
	ssize_t writelen;

	writelen = vsprintf(buff, fmt, ap);
	writelen = file_writeto(filename, buff, writelen, 0, 0);

	return writelen;
}

ssize_t file_printf(const char *filename, const char *fmt, ...)
{
	ssize_t writelen;
	va_list ap;

	va_start(ap, fmt);
	writelen = file_vprintf(filename, fmt, ap);
	va_end(ap);

	return writelen;
}

int file_set_loop(const char *filename, char *loop_path, u64 offset)
{
	int ret;
	int fd;
	int loop_fd;
	struct loop_info64 loopinfo;

	fd = file_open_rw_ro(filename, 0);
	if (fd < 0)
	{
		return fd;
	}

	loop_fd = loop_get_fd(filename, loop_path, offset);
	if (loop_fd < 0)
	{
		ret = loop_fd;
		error_msg("loop_get_fd");
		goto out_close_file;
	}

	ret = ioctl(loop_fd, LOOP_SET_FD, fd);
	if (ret < 0)
	{
		error_msg("LOOP_SET_FD");
		goto out_close_loop;
	}

	mem_set8(&loopinfo, 0, sizeof(loopinfo));
	if (to_abs_path2_base(filename, (char *)loopinfo.lo_file_name, sizeof(loopinfo.lo_file_name)) == NULL)
	{
		ret = -ENOENT;
		goto out_close_loop;
	}
	loopinfo.lo_offset = offset;

	ret = ioctl(loop_fd, LOOP_SET_STATUS64, &loopinfo);
	if (ret < 0)
	{
		error_msg("LOOP_SET_STATUS64");
		goto out_clr_fd;
	}

	goto out_close_loop;

out_clr_fd:
	ioctl(loop_fd, LOOP_CLR_FD, 0);
out_close_loop:
	close(loop_fd);
out_close_file:
	close(fd);

	return ret;
}

int file_mount_to(const char *source, const char *target, const char *fs_type, unsigned long flags, const void *data)
{
	int ret;
	char loop_path[64];

	ret = file_set_loop(source, loop_path, 0);
	if (ret < 0)
	{
#ifdef CAVAN_DEBUG
		error_msg("file set loop");
#endif
		return ret;
	}

	ret = libc_mount_to(loop_path, target, fs_type, flags, data);
	if (ret < 0)
	{
#ifdef CAVAN_DEBUG
		print_error("libc_mount_to");
#endif
		loop_clr_fd(loop_path);
		return ret;
	}

	return 0;
}

int ffile_newly_cmp(int fd1, int fd2)
{
	struct stat st1, st2;

	if (fstat(fd1, &st1) < 0)
	{
		return -1;
	}

	if (fstat(fd2, &st2) < 0)
	{
		return 1;
	}

	return st2.st_mtime - st1.st_mtime;
}

int file_newly_cmp(const char *file1, const char *file2)
{
	struct stat st1, st2;

	if (file_stat2(file1, &st1) < 0)
	{
		return -1;
	}

	if (file_stat2(file2, &st2) < 0)
	{
		return 1;
	}

	return st2.st_mtime - st1.st_mtime;
}

int chdir_backup(const char *dirname)
{
	static char dir_bak[1024];

	if (dirname == NULL)
	{
		if (dir_bak[0])
		{
			return chdir(dir_bak);
		}
		else
		{
			return -ENOENT;
		}
	}

	if (getcwd(dir_bak, sizeof(dir_bak)) == NULL)
	{
		dir_bak[0] = 0;
	}

	return chdir(dirname);
}

int ffile_delete_char(int fd_in, int fd_out, char c)
{
	while (1)
	{
		char buff[MAX_BUFFER_LEN];
		ssize_t readlen, writelen;

		readlen = read(fd_in, buff, sizeof(buff));
		if (readlen < 0)
		{
			print_error("read");
			return readlen;
		}

		if (readlen == 0)
		{
			return 0;
		}

		readlen = mem_delete_char(buff, readlen, c);
		writelen = write(fd_out, buff, readlen);
		if (writelen != readlen)
		{
			print_error("write");
			if (writelen < 0)
			{
				return writelen;
			}

			ERROR_RETURN(ENOMEDIUM);
		}
	}

	return 0;
}

int file_delete_char(const char *file_in, const char *file_out, char c)
{
	int ret;
	int fd_in, fd_out;

	fd_in = open(file_in, O_RDONLY | O_BINARY);
	if (fd_in < 0)
	{
		print_error("open input file \"%s\"", file_in);
		return fd_in;
	}

	fd_out = open(file_out, O_WRONLY | O_CREAT | O_SYNC | O_TRUNC | O_BINARY, 0777);
	if (fd_out < 0)
	{
		ret = fd_out;
		print_error("open output file \"%s\"", file_out);
		goto out_close_fd_in;
	}

	ret = ffile_delete_char(fd_in, fd_out, c);

	close(fd_out);
out_close_fd_in:
	close(fd_in);

	return ret;
}

int file_hardlink(const char *from, const char *to)
{
	int ret;

	remove(from);

	ret = mkdir_hierarchy(text_dirname(from), 0777);
	if (ret < 0)
	{
		return ret;
	}

	return link(to, from);
}

int vfile_try_open(int flags, mode_t mode, va_list ap)
{
	int fd;
	const char *filename;

	while ((filename = va_arg(ap, const char *)))
	{
		fd = open(filename, flags, mode);
		if (fd >= 0)
		{
			println("filename = %s", filename);
			return fd;
		}
	}

	return -ENOENT;
}

int file_try_open(int flags, mode_t mode, ...)
{
	int fd;
	va_list ap;

	va_start(ap, mode);
	fd = vfile_try_open(flags, mode, ap);
	va_end(ap);

	return fd;
}

int file_find_and_open(const char *prefix, char *last_path, int start, int end, int flags)
{
	int fd;
	char tmp_path[1024], *p, *p_bak;

	if (start > end)
	{
		NUMBER_SWAP(start, end);
	}

	if (last_path)
	{
		p = last_path;
	}
	else
	{
		p = tmp_path;
	}

	p_bak = p;

	p = text_copy(p, prefix);

	while (start <= end)
	{
		simple_value2text_unsigned(start, p, sizeof(tmp_path), 10);

		fd = open(p_bak, flags | O_NONBLOCK);
		if (fd >= 0)
		{
			return fd;
		}

		start++;
	}

	return -ENOENT;
}

int file_poll(int fd, short events, int timeout)
{
	int ret;
	struct pollfd pfd =
	{
		.fd = fd,
		.events = events,
	};

	ret = poll(&pfd, 1, timeout);
	if (ret < 0)
	{
		print_error("poll");
		return ret;
	}

	return pfd.revents;
}

char file_type_to_char(mode_t mode)
{
	switch (mode & S_IFMT)
	{
	case S_IFBLK:
		return 'b';

	case S_IFCHR:
		return 'c';

	case S_IFDIR:
		return 'd';

	case S_IFIFO:
		return 'f';

	case S_IFLNK:
		return 'l';

	default:
		return '-';
	}
}

char *file_permition_tostring(mode_t mode, char *text)
{
	int i;
	u32 shift;
	const char *permition_table = "rwx";

	shift = 1 << 8;

	while (shift)
	{
		for (i = 0; i < 3; i++, shift >>= 1, text++)
		{
			if (mode & shift)
			{
				*text = permition_table[i];
			}
			else
			{
				*text = '-';
			}
		}
	}

	return text;
}

const char *month_tostring(int month)
{
	const char *month_table[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

	if (month >= 0 && month < ARRAY_SIZE(month_table))
	{
		return month_table[month];
	}

	return "Unknown";
}

const char *week_tostring(int week)
{
	const char *week_table[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

	if (week >= 0 && week < ARRAY_SIZE(week_table))
	{
		return week_table[week];
	}

	return "Unknown";
}

int remove_directory(const char *pathname)
{
	int ret;
	char tmppath[1024], *name_p;
	DIR *dp;
	struct dirent *en;

	dp = opendir(pathname);
	if (dp == NULL)
	{
		print_error("opendir %s failed", pathname);
		return -ENOENT;
	}

	name_p = text_path_cat(tmppath, pathname, NULL);

	while ((en = readdir(dp)))
	{
		if (text_is_dot_name(en->d_name))
		{
			continue;
		}

		text_copy(name_p, en->d_name);

		if (en->d_type == DT_DIR)
		{
			ret = remove_directory(tmppath);
		}
		else
		{
			println("remove file \"%s\"", tmppath);
			ret = remove(tmppath);
		}

		if (ret < 0)
		{
			print_error("delete %s failed", tmppath);
			goto out_close_dp;
		}
	}

	println("remove directory \"%s\"", pathname);
	ret = rmdir(pathname);

out_close_dp:
	closedir(dp);

	return ret;
}

int remove_auto(const char *pathname)
{
	int ret;
	struct stat st;

	ret = lstat(pathname, &st);
	if (ret < 0)
	{
		print_error("get file %s stat failed", pathname);
		return ret;
	}

	return S_ISDIR(st.st_mode) ? remove_directory(pathname) : remove(pathname);
}

int file_type_test(const char *pathname, mode_t type)
{
	int ret;
	struct stat st;

	ret = file_lstat(pathname, &st);
	if (ret < 0)
	{
		return 0;
	}

	return (st.st_mode & S_IFMT) == type;
}

int fd_type_test(int fd, mode_t type)
{
	int ret;
	struct stat st;

	ret = fstat(fd, &st);
	if (ret < 0)
	{
		return 0;
	}

	return (st.st_mode & S_IFMT) == type;
}

size_t fscan_directory1(DIR *dp, void *buff, size_t size)
{
	void *buff_end;
	struct dirent *en;

	buff_end = buff + size;
	size = 0;

	while (buff < buff_end && (en = readdir(dp)))
	{
		if (text_is_dot_name(en->d_name))
		{
			continue;
		}

		buff = text_copy(buff, en->d_name) + 1;
		size++;
	}

	return size;
}

size_t fscan_directory2(DIR *dp, void *buff, size_t size1, size_t size2)
{
	void *buff_end;
	struct dirent *en;

	buff_end = buff + (size1 * size2);
	size1 = 0;

	while (buff < buff_end && (en = readdir(dp)))
	{
		if (text_is_dot_name(en->d_name))
		{
			continue;
		}

		text_ncopy(buff, en->d_name, size2);

		buff += size2;
		size1++;
	}

	return size1;
}

int scan_directory(const char *dirpath, void *buff, size_t size1, size_t size2)
{
	DIR *dp;

	dp = opendir(dirpath);
	if (dp == NULL)
	{
		print_error("opendir %s failed", dirpath);
		return -1;
	}

	size1 = size2 ? fscan_directory2(dp, buff, size1, size2) : fscan_directory1(dp, buff, size1);

	closedir(dp);

	return size1;
}

int file_open_format(int flags, mode_t mode, const char *fmt, ...)
{
	int fd;
	char buff[1024];
	va_list ap;

	va_start(ap, fmt);
	vsprintf(buff, fmt, ap);
	va_end(ap);

	fd = open(buff, flags, mode);
	if (fd < 0)
	{
		print_error("Open file `%s' failed", buff);
	}

	return fd;
}
