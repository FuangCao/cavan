#pragma once

#include <cavan/cavan_input.h>

struct cavan_input_attribute {
	struct attribute attr;
	struct cavan_input_device *dev;
	ssize_t (*show)(struct cavan_input_device *dev, struct cavan_input_attribute *attr, char *buff);
	ssize_t (*store)(struct cavan_input_device *dev, struct cavan_input_attribute *attr, const char *buff, size_t count);
};

struct cavan_sysfs_attribute {
	struct attribute attr;
	void *private_data;
	ssize_t (*show)(struct cavan_sysfs_attribute *attr, char *buff);
	ssize_t (*store)(struct cavan_sysfs_attribute *attr, const char *buff, size_t count);
};

int cavan_input_add_kobject(struct kobject *kobj, const char *name);
void cavan_input_remove_kobject(struct kobject *kobj);
int cavan_input_create_sysfs_files(struct cavan_input_device *dev, struct kobject *kobj, struct cavan_input_attribute *attrs, size_t count);
void cavan_input_remove_sysfs_files(struct kobject *kobj, struct cavan_input_attribute *attrs, size_t count);

int cavan_sysfs_create_file(const struct attribute *attr);
int cavan_sysfs_create_files(const struct attribute **attr);

static inline void cavan_sysfs_attribute_set_data(struct cavan_sysfs_attribute *attr, void *data)
{
	attr->private_data = data;
}

static inline void *cavan_sysfs_attribute_get_data(struct cavan_sysfs_attribute *attr)
{
	return attr->private_data;
}