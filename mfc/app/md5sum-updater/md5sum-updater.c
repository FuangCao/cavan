#include <stdio.h>
#include <stdlib.h>
#include <io.h>

#define NELEM(a) \
	(sizeof(a) / sizeof(a[0]))

const char *sVideoFileList[] = { "rm", "rmvb", "wmv", "avi", "mp4", "3gp", "mkv", "mpeg", "mpg", "vm" };

static const char *getFileExtension(const char *pathname)
{
	const char *extension = NULL;

	while (*pathname) {
		if (*pathname == '.') {
			extension = pathname + 1;
		}

		pathname++;
	}

	if (extension) {
		return extension;
	}

	return pathname;
}

static int strcasecmp(const char *src, const char *dest)
{
	while (((*src) | 0x20) == ((*dest) | 0x20)) {
		if (*src == 0) {
			return 0;
		}

		src++;
		dest++;
	}

	return *src - *dest;
}

static int isVideoFile(const char *pathname)
{
	int i;
	const char *extension;

	extension = getFileExtension(pathname);
	if (extension[0] == 0) {
		return 0;
	}

	// printf("extension = %s\n", extension);

	for (i = 0; i < NELEM(sVideoFileList); i++) {
		if (strcasecmp(extension, sVideoFileList[i]) == 0) {
			return 1;
		}
	}

	printf("文件格式不支持：%s\n", pathname);

	return 0;
}

static int doUpdateMd5sum(const char *pathname)
{
	FILE *fp;

	if (!isVideoFile(pathname)) {
		return -1;
	}

	fp = fopen(pathname, "a+");
	if (fp == NULL) {
		return -1;
	}

	fwrite("12345678", 8, 1, fp);

	fclose(fp);

	printf("=> 成功更改文件：%s\n", pathname);

	return 0;
}

int main(int argc, char *argv[])
{
	int i;

	printf("支持的文件格式有：");

	for (i = 0; i < NELEM(sVideoFileList); i++) {
		printf(i > 0 ? ", %s" : "%s", sVideoFileList[i]);
	}

	putchar('\n');

	if (argc > 1) {
		int i;

		for (i = 1; i < argc; i++) {
			doUpdateMd5sum(argv[i]);
		}
	} else {
		struct _finddata_t data;
		long handle = _findfirst("*.*", &data);
		if (handle < 0) {
			return handle;
		}

		do {
			doUpdateMd5sum(data.name);
		} while (_findnext(handle, &data) == 0);

		_findclose(handle);
	}

	printf("所有的文件都已经改好了！");
	getchar();

    return 0;
}
