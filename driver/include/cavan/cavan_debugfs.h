#pragma once

#include <cavan/cavan_input.h>

struct dentry *cavan_debugfs_create_file(const char *name, umode_t mode, struct dentry *parent, void *data, const struct file_operations *fops);
struct dentry *cavan_debugfs_create_dir(const char *name, struct dentry *parent);
struct dentry *cavan_debugfs_create_symlink(const char *name, struct dentry *parent, const char *dest);
