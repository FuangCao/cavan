#pragma once

#include <cavan.h>
#include <cavan/network.h>

// Fuang.Cao <cavan.cfa@gmail.com> Fri May 13 17:54:01 CST 2011

struct uevent_filter
{
	int count;
	char *props[100];
};

struct uevent_desc
{
	int sockfd;
	char buff[1024];
	char *props[100];
	size_t prop_count;
};

int uevent_init(struct uevent_desc *desc);
void uevent_deinit(struct uevent_desc *desc);
int get_disk_insert_uevent(struct uevent_desc *desc, char *devname);
size_t uevent_split_base(const char *event, size_t event_len, char *props[], size_t size);
char *uevent_get_property_base(char *props[], int prop_count, const char *prefix, char *buff);
int uevent_match_base(char *props[], int prop_count, struct uevent_filter *filter);
int get_device_uevent(struct uevent_desc *desc, struct uevent_filter *filters, size_t count);

int get_disk_add_uevent(struct uevent_desc *desc);
int get_partition_add_uevent(struct uevent_desc *desc);
int get_block_device_remove_uevent(struct uevent_desc *desc);

static inline ssize_t uevent_recv(struct uevent_desc *desc, void *buff, size_t size, int flags)
{
	return recv(desc->sockfd, buff, size, flags);
}

static inline char *uevent_get_property(struct uevent_desc *desc, const char *prefix, char *buff)
{
	return uevent_get_property_base(desc->props, desc->prop_count, prefix, buff);
}

static inline char *uevent_get_propery_devname(struct uevent_desc *desc, char *devname)
{
	return uevent_get_property(desc, "DEVNAME=", devname);
}

static inline char *uevent_get_propery_devtype(struct uevent_desc *desc, char *devtype)
{
	return uevent_get_property(desc, "DEVTYPE=", devtype);
}

static inline char *uevent_get_propery_action(struct uevent_desc *desc, char *action)
{
	return uevent_get_property(desc, "ACTION=", action);
}

static inline char *uevent_get_propery_major(struct uevent_desc *desc, char *major)
{
	return uevent_get_property(desc, "MAJOR=", major);
}

static inline char *uevent_get_propery_minor(struct uevent_desc *desc, char *minor)
{
	return uevent_get_property(desc, "MINOR=", minor);
}
