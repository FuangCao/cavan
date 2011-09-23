#pragma once

#include <cavan/network.h>

// Fuang.Cao <cavan.cfa@gmail.com> Fri May 13 17:54:01 CST 2011

struct uevent_desc
{
	int sockfd;
};

int uevent_init(struct uevent_desc *desc);
void uevent_uninit(struct uevent_desc *desc);
int get_disk_insert_uevent(struct uevent_desc *desc, char *devname);
int get_device_uevent(struct uevent_desc *desc, const char *actions[], const char *typename, const char *type, char *devname);

static inline ssize_t uevent_recv(struct uevent_desc *desc, void *buff, size_t size, int flags)
{
	return recv(desc->sockfd, buff, size, flags);
}

static inline int get_disk_add_uevent(struct uevent_desc *desc, char *devname)
{
	const char *actions[] = {"add", NULL};

	return get_device_uevent(desc, actions, "DEVTYPE", "disk", devname);
}

static inline int get_partition_add_uevent(struct uevent_desc *desc, char *devname)
{
	const char *actions[] = {"add", NULL};

	return get_device_uevent(desc, actions, "DEVTYPE", "partition", devname);
}

static inline int get_block_device_remove_uevent(struct uevent_desc *desc, char *devname)
{
	const char *actions[] = {"remove", NULL};

	return get_device_uevent(desc, actions, "SUBSYSTEM", "block", devname);
}
