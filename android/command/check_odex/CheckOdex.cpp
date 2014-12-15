/*
 * File:		CheckOdex.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2014-11-25 11:55:09
 *
 * Copyright (c) 2014 Fuang.Cao <cavan.cfa@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <zlib.h>
#include <errno.h>
#include <DexFile.h>
#include <utils/Log.h>

#define MIN_DEP_SIZE	(4 * 4)
#define MAX_DEP_SIZE	(4 * 4 + 2048)
#define LOCAL_DEBUG		0

#define println(fmt, args ...) \
	printf(fmt "\n", ##args)

#define pr_pos_info() \
	println("%s => %s[%d]", __FILE__, __FUNCTION__, __LINE__)

#define pr_bold_info(fmt, args ...) \
	println("\033[1m" fmt "\033[0m", ##args)

#define pr_red_info(fmt, args ...) \
	println("\033[31m" fmt "\033[0m", ##args)

#define pr_green_info(fmt, args ...) \
	println("\033[32m" fmt "\033[0m", ##args)

#if LOCAL_DEBUG
static void showDexOptHeader(DexOptHeader *header)
{
	pr_bold_info("DexOptHeader[%p]:", header);
	println("magic = %s", header->magic);
	println("dexOffset = %d", header->dexOffset);
	println("dexLength = %d", header->dexLength);
	println("depsOffset = %d", header->depsOffset);
	println("depsLength = %d", header->depsLength);
	println("optOffset = %d", header->optOffset);
	println("optLength = %d", header->optLength);
	println("flags = 0x%08x", header->flags);
	println("checksum = 0x%08x", header->checksum);
}

static void showDexHeader(DexHeader *header)
{
	pr_bold_info("DexHeader[%p]:", header);
	println("magic = %s", header->magic);
	// println("signature = %s", header->signature);
	println("fileSize = %d", header->fileSize);
	println("headerSize = %d", header->headerSize);
	println("endianTag = %d", header->endianTag);
	println("linkSize = %d", header->linkSize);
	println("linkOff = %d", header->linkOff);
	println("mapOff = %d", header->mapOff);
	println("stringIdsSize = %d", header->stringIdsSize);
	println("stringIdsOff = %d", header->stringIdsOff);
	println("typeIdsSize = %d", header->typeIdsSize);
	println("typeIdsOff = %d", header->typeIdsOff);
	println("protoIdsSize = %d", header->protoIdsSize);
	println("protoIdsOff = %d", header->protoIdsOff);
	println("fieldIdsSize = %d", header->fieldIdsSize);
	println("fieldIdsOff = %d", header->fieldIdsOff);
	println("methodIdsSize = %d", header->methodIdsSize);
	println("methodIdsOff = %d", header->methodIdsOff);
	println("classDefsSize = %d", header->classDefsSize);
	println("classDefsOff = %d", header->classDefsOff);
	println("dataSize = %d", header->dataSize);
	println("dataOff = %d", header->dataOff);
	println("checksum = 0x%08x", header->checksum);
}
#endif

static u4 fileAdler32(int fd, u4 adler, off_t start, size_t size)
{
    unsigned char buff[8192];

    if (lseek(fd, start, SEEK_SET) != start) {
        pr_red_info("Unable to seek to start of checksum area (%ld): %s", (long) start, strerror(errno));
		return 0;
    }

    while (size != 0) {
        ssize_t rdLen = read(fd, buff, size < sizeof(buff) ? size : sizeof(buff));
        if (rdLen <= 0) {
            pr_red_info("Read failed (%d) while computing checksum (len=%zu): %s", (int) rdLen, size, strerror(errno));
			return 0;
        }

        adler = adler32(adler, buff, rdLen);
        size -= rdLen;
    }

	return adler;
}

static ssize_t fileReadFrom(int fd, off_t start, void *buff, size_t size)
{
	if (lseek(fd, start, SEEK_SET) != start) {
		pr_red_info("lseek");
		return -EFAULT;
	}

	return read(fd, buff, size);
}

int odexCheck(int fd)
{
	u4 checksum;
	ssize_t rdLen;
    DexOptHeader dexOptHeader;
	DexHeader dexHeader;

	rdLen = fileReadFrom(fd, 0, &dexOptHeader, sizeof(dexOptHeader));
	if (rdLen < (ssize_t) sizeof(dexOptHeader)) {
		pr_red_info("read DexOptHeader failed");
		return -EFAULT;
	}

	if (memcmp(dexOptHeader.magic, DEX_OPT_MAGIC, 4)) {
        pr_red_info("DexOpt: incorrect opt magic number");
		return -EFAULT;
    }

    if (memcmp(dexOptHeader.magic + 4, DEX_OPT_MAGIC_VERS, 4)) {
        pr_red_info("DexOpt: stale opt version");
		return -EFAULT;
    }

    if (dexOptHeader.depsLength < MIN_DEP_SIZE || dexOptHeader.depsLength > MAX_DEP_SIZE) {
        pr_red_info("DexOpt: weird deps length %d, bailing", dexOptHeader.depsLength);
		return -EFAULT;
    }

#if __BYTE_ORDER == __LITTLE_ENDIAN
	if ((dexOptHeader.flags & DEX_OPT_FLAG_BIG))
#else
	if ((dexOptHeader.flags & DEX_OPT_FLAG_BIG) != DEX_OPT_FLAG_BIG)
#endif
	{
		pr_red_info("DexOpt: header flag mismatch");
		return -EFAULT;
	}

#if LOCAL_DEBUG
	showDexOptHeader(&dexOptHeader);
	println("checksum = 0x%08x", checksum);
#endif

	checksum = fileAdler32(fd, adler32(0L, Z_NULL, 0), dexOptHeader.depsOffset, dexOptHeader.optOffset + dexOptHeader.optLength - dexOptHeader.depsOffset);
	if (dexOptHeader.checksum != checksum) {
		pr_red_info("DexOptHeader checksum is not match");
		return -EFAULT;
	}

#if LOCAL_DEBUG
	pr_green_info("DexOptHeader checksum is match");
#endif

	rdLen = fileReadFrom(fd, dexOptHeader.dexOffset, &dexHeader, sizeof(dexHeader));
	if (rdLen < (ssize_t) sizeof(dexHeader)) {
		return -EFAULT;
	}

	if (memcmp(dexHeader.magic, DEX_MAGIC, 4)) {
        pr_red_info("Dex: incorrect opt magic number");
		return -EFAULT;
    }

#if 0
    if (memcmp(dexHeader.magic + 4, DEX_MAGIC_VERS, 4)) {
        pr_red_info("Dex: stale opt version");
		return -EFAULT;
    }
#endif

#if LOCAL_DEBUG
	showDexHeader(&dexHeader);
	println("checksum = 0x%08x", checksum);
#endif

	rdLen = sizeof(dexHeader.magic) + sizeof(dexHeader.checksum);
	checksum = adler32(adler32(0L, Z_NULL, 0), dexHeader.signature, sizeof(dexHeader) - rdLen);
	checksum = fileAdler32(fd, checksum, dexOptHeader.dexOffset + sizeof(dexHeader), dexOptHeader.dexLength - sizeof(dexHeader));
	if (dexHeader.checksum != checksum) {
		pr_red_info("DexHeader checksum is not match");
		return -EFAULT;
	}

#if LOCAL_DEBUG
	pr_green_info("DexHeader checksum is match");
#endif

	return 0;
}

int odexCheck2(const char *pathname)
{
	int fd;
	int ret;

	fd = open(pathname, O_RDONLY);
	if (fd < 0) {
		pr_red_info("open file %s failed", pathname);
		return fd;
	}

	ret = odexCheck(fd);
	if (ret < 0) {
		pr_red_info("%s failed", pathname);
	} else {
		pr_green_info("%s passed", pathname);
	}

	close(fd);

	return ret;
}

int main(int argc, char *argv[])
{
	int i;
	int failed;

	for (failed = 0, i = 1; i < argc; i++) {
		int ret = odexCheck2(argv[i]);
		if (ret < 0) {
			failed++;
		}
	}

	println("passed: %d, failed: %d", argc - failed - 1, failed);

	return failed > 0 ? -EFAULT : 0;
}
