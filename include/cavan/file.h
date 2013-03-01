#pragma once

#include <cavan/text.h>
#include <sys/file.h>
#include <poll.h>

#define MAX_BUFFER_LEN				MB(1)

#ifndef O_BINARY
 #ifdef _O_BINARY
  #define O_BINARY					_O_BINARY
 #else
  #define O_BINARY					0
 #endif
#endif

#ifndef S_IFSOCK
#define S_IFSOCK 0140000
#endif

#define	READ_FLAGS					(O_RDONLY | O_BINARY)
#define WRITE_FLAGS					(O_CREAT | O_WRONLY | O_SYNC | O_BINARY)
#define RDWR_FLAGS					(O_CREAT | O_RDWR | O_SYNC | O_BINARY)
#define MD5SUM_LEN					32
#define MD5SUM_FILE_PATH			"/tmp/md5sum.txt"

#define RANDOM_DEVICE				"/dev/random"
#define URANDOM_DEVICE				"/dev/urandom"
#define FILE_PROC_MOUNTS			"/proc/mounts"
#define FILE_PROC_FILESYSTEMS		"/proc/filesystems"
#define FILE_PROC_CPUINFO			"/proc/cpuinfo"
#define FILE_ETC_MTAB				"/etc/mtab"

#define TEMP_DOS2UNIX_FILE			"/tmp/dos2unix_tmp_file"

#define FILE_PROC_MOUNTS_MAX_SIZE	MB(1)
#define FILE_PROC_FILESYSTEMS_SIZE	MB(1)
#define FILE_CPUINFO_SIZE			MB(1)

int file_open_rw_ro(const char *pathname, int flags);

int file_operation_ro(const char *filename, int (*handle)(int fd));
int file_noperation_ro(const char *filename, size_t size, int (*handle)(int fd, size_t size));
int file_operation_wo(const char *filename, int (*handle)(int fd));
int file_noperation_wo(const char *filename, size_t size, int (*handle)(int fd, size_t size));
int file_operation_rw(const char *filename, int (*handle)(int fd));
int file_noperation_rw(const char *filename, size_t size, int (*handle)(int fd, size_t size));

int mkdir_all(const char *pathname);
int mkdir_hierarchy(const char *pathname, mode_t mode);
int file_create_open(const char *pathname, int flags, mode_t mode);

int file_join(const char *dest_file, char *src_files[], int count);
int file_split(const char *file_name, const char *dest_dir, int count);

int ffile_copy(int src_fd, int dest_fd);
int ffile_copy_simple(int src_fd, int dest_fd);
int file_copy(const char *src_file, const char *dest_file, int flags);
int file_append(const char *file_src, const char *file_dest);

off_t ffile_get_size(int fd);
off_t file_get_size(const char *filepath);

int ffile_ncopy(int src_fd, int dest_fd, size_t size);
int ffile_ncopy_simple(int src_fd, int dest_fd, size_t size);
int file_ncopy(const char *src_file, const char *dest_file, size_t size, int flags);

int vtry_to_open(int flags, va_list ap);
int try_to_open(int flags, ...);

ssize_t ffile_writeto(int fd, const void *buff, size_t size, off_t offset);
ssize_t ffile_readfrom(int fd, void *buff, size_t size, off_t offset);

ssize_t file_writeto(const char *file_name, const void *buff, size_t size, off_t offset, int flags);
ssize_t file_readfrom(const char *file_name, void *buff, size_t size, off_t offset, int flags);

int file_test_read(const char *filename);

int ffile_show(int fd);
int ffile_nshow(int fd, size_t size);

int ffile_cat(int fd);
int ffile_ncat(int fd, size_t size);

int ffile_cmp(int fd1, int fd2, size_t size);
int file_cmp(const char *file1, const char *file2, size_t size);

u16 mem_crc16(u16 crc, const char *buff, size_t size);
u32 mem_crc32(u32 crc, const char *buff, size_t size);
int ffile_crc32(int fd, u32 *crc);
int file_crc32(const char *file_name, u32 *crc);

int ffile_crc32_seek(int fd, off_t offset, int whence, u32 *crc);
int file_crc32_seek(const char *file_name, off_t offset, int whence, u32 *crc);

int ffile_ncrc32(int fd, size_t size, u32 *crc);
int file_ncrc32(const char *file_name, size_t size, u32 *crc);

int ffile_ncrc32_seek(int fd, off_t offset, int whence, size_t size, u32 *crc);
int file_ncrc32_seek(const char *file_name, off_t offset, int whence, size_t size, u32 *crc);

int mode_test(mode_t mode, char type);

mode_t ffile_get_mode(int fd);
mode_t file_get_mode(const char *pathname);
mode_t file_get_lmode(const char *pathname);

int file_test(const char *file_name, const char *types);
int ffile_test(int fd, char type);

int file_wait(const char *filepath, const char *types, u32 sec);

int file_resize(const char *file_path, off_t length);

int get_file_pointer(int fd, off_t *fpointer);

int calculate_file_md5sum(const char *file_path, char *md5sum);
int check_file_md5sum(const char *file_path, char *md5sum);

ssize_t ffile_read(int fd, void *buff, size_t size);

int file_replace_line_simple(const char *file_path, const char *prefix, off_t prefix_size, const char *new_line, off_t new_line_size);
int file_stat(const char *file_name, struct stat *st);
int file_select_read(int fd, int timeout_ms);
bool file_poll(int fd, short events, int timeout_ms);
bool file_discard_all(int fd);

u32 mem_checksum32_simple(const char *mem, size_t count);
u16 mem_checksum16_simple(const char *mem, size_t count);
u8 mem_checksum8_simple(const char *mem, size_t count);

u32 ffile_checksum32_simple(int fd, off_t offset, size_t size);
u16 ffile_checksum16_simple(int fd, off_t offset, size_t size);
u8 ffile_checksum8_simple(int fd, off_t offset, size_t size);


u32 file_checksum32(const char *filename, off_t offset, size_t size);
u16 file_checksum16(const char *filename, off_t offset, size_t size);
u8 file_checksum8(const char *filename, off_t offset, size_t size);

int file_set_loop(const char *filename, char *loop_path, u64 offset);
int file_mount_to(const char *source, const char *target, const char *fs_type, unsigned long flags, const void *data);

int ffile_newly_cmp(int fd1, int fd2);
int file_newly_cmp(const char *file1, const char *file2);

int chdir_backup(const char *dirname);

int ffile_delete_char(int fd_in, int fd_out, char c);
int file_delete_char(const char *file_in, const char *file_out, char c);

int file_hardlink(const char *from, const char *to);

int vfile_try_open(int flags, mode_t mode, va_list ap);
int file_try_open(int flags, mode_t mode, ...);

__printf_format_20__ ssize_t ffile_vprintf(int fd, const char *fmt, va_list ap);
__printf_format_23__ ssize_t ffile_prinf(int fd, const char *fmt, ...);
__printf_format_20__ ssize_t file_vprintf(const char *filename, const char *fmt, va_list ap);
__printf_format_23__ ssize_t file_printf(const char *filename, const char *fmt, ...);
__printf_format_34__ int file_open_format(int flags, mode_t mode, const char *fmt, ...);

int file_find_and_open(const char *prefix, char *last_path, int start, int end, int flags);

char file_type_to_char(mode_t mode);
char *file_permition_tostring(mode_t mode, char *text);
const char *month_tostring(int month);
const char *week_tostring(int week);

int remove_directory(const char *pathname);
int remove_auto(const char *pathname);

int file_type_test(const char *pathname, mode_t type);
int fd_type_test(int fd, mode_t type);

size_t fscan_directory1(DIR *dp, void *buff, size_t size);
size_t fscan_directory2(DIR *dp, void *buff, size_t size1, size_t size2);
int scan_directory(const char *dirpath, void *buff, size_t size1, size_t size2);

size_t ffile_line_count(int fd);
size_t file_line_count(const char *filename);

int file_mmap(const char *pathname, void **addr, size_t *size, int flags);
void file_unmap(int fd, void *map, size_t size);
void *file_read_all(const char *pathname, size_t extra, size_t *size);

// ============================================================

static inline int file_open_ro(const char *file_path)
{
	return open(file_path, READ_FLAGS);
}

static inline int file_open_wo(const char *file_path)
{
	return open(file_path, WRITE_FLAGS, 0777);
}

static inline int file_open_rw(const char *file_path)
{
	return open(file_path, RDWR_FLAGS, 0777);
}

static inline int file_trunc_open_wot(const char *file_path)
{
	return open(file_path, WRITE_FLAGS | O_TRUNC, 0777);
}

static inline int file_open_rwt(const char *file_path)
{
	return open(file_path, RDWR_FLAGS | O_TRUNC, 0777);
}

static inline int file_show(const char *filename, size_t size)
{
	return file_noperation_ro(filename, size, ffile_nshow);
}

static inline int file_cat(const char *filename, size_t size)
{
	return file_noperation_ro(filename, size, ffile_ncat);
}

static inline ssize_t file_read(const char *filename, void *buff, size_t size)
{
	return file_readfrom(filename, buff, size, 0, 0);
}

ssize_t ffile_write(int fd, const void *buff, size_t size);

static inline ssize_t file_write(const char *filename, const void *buff, size_t size)
{
	return file_writeto(filename, buff, size, 0, 0);
}

static inline int ffile_crc32_back(int fd, u32 *crc)
{
	return ffile_crc32_seek(fd, 0, SEEK_CUR, crc);
}

static inline int file_crc32_back(const char *file_name, u32 *crc)
{
	return file_crc32_seek(file_name, 0, SEEK_CUR, crc);
}

static inline int ffile_ncrc32_back(int fd, size_t size, u32 *crc)
{
	return ffile_ncrc32_seek(fd, 0, SEEK_CUR, size, crc);
}

static inline int file_ncrc32_back(const char *file_name, size_t size, u32 *crc)
{
	return file_ncrc32_seek(file_name, 0, SEEK_CUR, size, crc);
}

static inline u8 mem_checksum8(const void *mem, size_t count)
{
	return ~(mem_checksum8_simple(mem, count));
}

static inline u16 mem_checksum16(const void *mem, size_t count)
{
	return ~(mem_checksum16_simple(mem, count));
}

static inline u32 mem_checksum32(const void *mem, size_t count)
{
	return ~(mem_checksum32_simple(mem, count));
}

static inline u32 ffile_checksum32(int fd, off_t offset, size_t size)
{
	return ~(ffile_checksum32_simple(fd, offset, size));
}

static inline u16 ffile_checksum16(int fd, off_t offset, size_t size)
{
	return ~(ffile_checksum16_simple(fd, offset, size));
}

static inline u8 ffile_checksum8(int fd, off_t offset, size_t size)
{
	return ~(ffile_checksum8_simple(fd, offset, size));
}

static inline ssize_t ffile_puts(int fd, const char *text)
{
	return ffile_write(fd, text, text_len(text));
}

static inline ssize_t file_puts(const char *filename, const char *text)
{
	return file_writeto(filename, text, text_len(text), 0, 0);
}

static inline ssize_t file_read_random(void *buff, size_t size)
{
	return file_readfrom(RANDOM_DEVICE, buff, size, 0, 0);
}

static inline ssize_t file_read_urandom(void *buff, size_t size)
{
	return file_readfrom(URANDOM_DEVICE, buff, size, 0, 0);
}

static inline ssize_t file_read_mounts(void *buff, size_t size)
{
	return file_read(FILE_PROC_MOUNTS, buff, size);
}

static inline ssize_t file_read_filesystems(void *buff, size_t size)
{
	return file_read(FILE_PROC_FILESYSTEMS, buff, size);
}

static inline int update_mount_table(void)
{
	return file_copy(FILE_PROC_MOUNTS, FILE_ETC_MTAB, 0);
}

static inline ssize_t file_read_cpuinfo(void *buff, size_t size)
{
	return file_read(FILE_PROC_CPUINFO, buff, size);
}

static inline int file_dos2unix(const char *filename)
{
	int ret;

	ret = file_delete_char(filename, TEMP_DOS2UNIX_FILE, '\r');
	if (ret < 0)
	{
		return ret;
	}

	return file_copy(TEMP_DOS2UNIX_FILE, filename, 0);
}

static inline bool file_access_e(const char *filename)
{
	return access(filename, 0) == 0;
}

static inline bool file_access_r(const char *filename)
{
	return access(filename, R_OK) == 0;
}

static inline bool file_access_w(const char *filename)
{
	return access(filename, W_OK) == 0;
}

static inline bool file_access_x(const char *filename)
{
	return access(filename, X_OK) == 0;
}

static inline bool file_access_rw(const char *filename)
{
	return access(filename, R_OK | W_OK) == 0;
}

static inline bool file_access_rwx(const char *filename)
{
	return access(filename, R_OK | W_OK | X_OK) == 0;
}

static inline bool file_poll_input(int fd, int timeout_ms)
{
	return file_poll(fd, POLLIN, timeout_ms);
}

static inline bool file_poll_output(int fd, int timeout_ms)
{
	return file_poll(fd, POLLOUT, timeout_ms);
}

static inline ssize_t file_read_timeout(int fd, void *buff, size_t size, int timeout_ms)
{
	if (file_poll_input(fd, timeout_ms))
	{
		return read(fd, buff, size);
	}

	ERROR_RETURN(ETIMEDOUT);
}

static inline int file_is_file(const char *pathname)
{
	return file_type_test(pathname, S_IFREG);
}

static inline int file_is_directory(const char *pathname)
{
	return file_type_test(pathname, S_IFDIR);
}

static inline int file_is_symlink(const char *pathname)
{
	return file_type_test(pathname, S_IFLNK);
}

static inline int file_is_char_dev(const char *pathname)
{
	return file_type_test(pathname, S_IFCHR);
}

static inline int file_is_block_dev(const char *pathname)
{
	return file_type_test(pathname, S_IFBLK);
}

static inline int file_is_socket(const char *pathname)
{
	return file_type_test(pathname, S_IFSOCK);
}

static inline int file_is_fifo(const char *pathname)
{
	return file_type_test(pathname, S_IFIFO);
}

static inline int fd_is_file(int fd)
{
	return fd_type_test(fd, S_IFREG);
}

static inline int fd_is_directory(int fd)
{
	return fd_type_test(fd, S_IFDIR);
}

static inline int fd_is_symlink(int fd)
{
	return fd_type_test(fd, S_IFLNK);
}

static inline int fd_is_char_dev(int fd)
{
	return fd_type_test(fd, S_IFCHR);
}

static inline int fd_is_block_dev(int fd)
{
	return fd_type_test(fd, S_IFBLK);
}

static inline int fd_is_socket(int fd)
{
	return fd_type_test(fd, S_IFSOCK);
}

static inline int fd_is_fifo(int fd)
{
	return fd_type_test(fd, S_IFIFO);
}

static inline int file_stat2(const char *filename, struct stat *st)
{
	return stat(filename, st) < 0 ? file_stat(filename, st) : 0;
}

static inline int file_lstat(const char *filename, struct stat *st)
{
	return lstat(filename, st) < 0 ? file_stat(filename, st) : 0;
}

static inline ssize_t file_read_byte(int fd, void *buff)
{
	return read(fd, buff, 1);
}
