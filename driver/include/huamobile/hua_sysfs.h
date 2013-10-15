#pragma once

#include <huamobile/hua_input.h>

struct hua_input_attribute
{
	struct attribute attr;
	struct hua_input_device *dev;
	ssize_t (*show)(struct hua_input_device *dev, struct hua_input_attribute *attr, char *buff);
	ssize_t (*store)(struct hua_input_device *dev, struct hua_input_attribute *attr, const char *buff, size_t count);
};

int hua_input_add_kobject(struct kobject *kobj, const char *name);
void hua_input_remove_kobject(struct kobject *kobj);
int hua_input_create_sysfs_files(struct hua_input_device *dev, struct kobject *kobj, struct hua_input_attribute *attrs, size_t count);
void hua_input_remove_sysfs_files(struct kobject *kobj, struct hua_input_attribute *attrs, size_t count);
