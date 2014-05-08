#include <dirent.h>
#include <sys/types.h>
#include <cavan.h>
#include <cavan/progress.h>
#include <cavan/copy.h>
#include <cavan/text.h>
#include <cavan/file.h>
#include <cavan/device.h>

#define MAX_PATH_LEN		KB(1)

int write_flags = O_CREAT | O_BINARY;

int symlink_copy(const char *src, const char *dest)
{
	int ret;
	char buff[MAX_PATH_LEN];

	ret = readlink(src, buff, MAX_PATH_LEN);
	if (ret < 0)
	{
		print_error("readlink");
		return ret;
	}

	buff[ret] = 0;

	ret = symlink(buff, dest);
	if (ret < 0)
	{
		print_error("symlink");
	}

	return ret;
}

int file_copy_main(const char *src, const char *dest)
{
	int ret;
	struct stat st;

	ret = file_lstat(src, &st);
	if (ret < 0)
	{
		pr_error_info("file_lstat");
		return ret;
	}

	umask(0);

	switch (st.st_mode & S_IFMT)
	{
	case S_IFCHR:
	case S_IFBLK:
		println("Device: %s [copy]-> %s", src, dest);
		return remknod(dest, st.st_mode, st.st_rdev);

	case S_IFLNK:
		println("Symlink: %s [copy]-> %s", src, dest);
		return symlink_copy(src, dest);

	case S_IFIFO:
		println("FIFO: %s [copy]-> %s", src, dest);
		return remkfifo(dest, st.st_mode);

	case S_IFDIR:
		println("Directory: %s [copy]-> %s", src, dest);
		return directory_copy_main(src, dest);

	case S_IFREG:
		println("File: %s [copy]-> %s", src, dest);
		return file_copy(src, dest, write_flags);

	default:
		pr_red_info("unknown file type");
		return 0;
	}

	return 0;
}

int directory_copy_only(const char *src, const char *dest)
{
	int ret;
	struct stat st;

	ret = file_stat2(src, &st);
	if (ret < 0)
	{
		return ret;
	}

	ret = mkdir(dest, st.st_mode);
	if (ret == 0 || errno == EEXIST)
	{
		return 0;
	}

	return ret;
}

int directory_copy_main(const char *src, const char *dest)
{
	int ret;
	DIR *src_dir;
	struct dirent *dt;
	char tmp_dirname_src[1024], *src_p;
	char tmp_dirname_dest[1024], *dest_p;

	ret = directory_copy_only(src, dest);
	if (ret < 0)
	{
		pr_red_info("create directory failed");
		return ret;
	}

	src_dir = opendir(src);
	if (src_dir == NULL)
	{
		print_error("opendir");
		return -1;
	}

	src_p = text_path_cat(tmp_dirname_src, sizeof(tmp_dirname_src), src, NULL);
	dest_p = text_path_cat(tmp_dirname_dest, sizeof(tmp_dirname_dest), dest, NULL);

	while (1)
	{
		dt = readdir(src_dir);
		if (dt == NULL)
		{
			break;
		}

#ifdef CAVAN_DEBUG
		println("filename = %s", dt->d_name);
#endif

		if (dt->d_name[0] == '.')
		{
			if (dt->d_name[1] == 0)
			{
				continue;
			}

			if (dt->d_name[1] == '.' && dt->d_name[2] == 0)
			{
				continue;
			}
		}

		text_copy(src_p, dt->d_name);
		text_copy(dest_p, dt->d_name);

		if (dt->d_type == DT_DIR)
		{
			ret = directory_copy_main(tmp_dirname_src, tmp_dirname_dest);
			if (ret < 0)
			{
				pr_red_info("directory_copy_main");
				goto out_close_dir;
			}
		}
		else
		{
			ret = file_copy_main(tmp_dirname_src, tmp_dirname_dest);
			if (ret < 0)
			{
				pr_red_info("file_copy_main");
				goto out_close_dir;
			}
		}
	}

	ret = 0;
out_close_dir:
	closedir(src_dir);

	return ret;
}

int copy_main(const char *src, const char *dest)
{
	char dest_path[1024];

	if (strcmp(src, dest) == 0)
	{
		println("source file \"%s\" and dest file \"%s\" arm same files", src, dest);
		return 0;
	}

	if (file_test(dest, "d") == 0)
	{
		text_path_cat(dest_path, sizeof(dest_path), dest, text_basename(src));
	}
	else
	{
		text_copy(dest_path, dest);
	}

	return file_copy_main(src, dest_path);
}

int move_auto(const char *srcpath, const char *destpath)
{
	int ret;
	char tmppath[1024], *p;
	struct stat st;

	ret = file_stat(destpath, &st);
	if (ret < 0)
	{
		goto label_start_move;
	}

	switch (st.st_mode & S_IFMT)
	{
	case S_IFDIR:
		p = text_path_cat(tmppath, sizeof(tmppath), destpath, NULL);
		text_basename_base(p, srcpath);
		destpath = tmppath;
		break;

	default:
		ret = remove(destpath);
		if (ret < 0)
		{
			print_error("remove %s failed", destpath);
			return ret;
		}
	}

label_start_move:
	println("Move %s => %s", srcpath, destpath);

	if (rename(srcpath, destpath) == 0)
	{
		return 0;
	}

	ret = file_copy_main(srcpath, destpath);
	if (ret < 0)
	{
		return ret;
	}

	return remove_auto(srcpath);
}
