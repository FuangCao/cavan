#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/idr.h>
#include <linux/slab.h>
#include <cavan/cavan_debugfs.h>

static struct dentry *root_dir;

struct dentry *cavan_debugfs_create_file(const char *name, umode_t mode, struct dentry *parent, void *data, const struct file_operations *fops)
{
	if (root_dir == NULL) {
		return NULL;
	}

	return debugfs_create_file(name, mode, root_dir, data, fops);
}
EXPORT_SYMBOL_GPL(cavan_debugfs_create_file);

struct dentry *cavan_debugfs_create_dir(const char *name, struct dentry *parent)
{
	if (root_dir == NULL) {
		return NULL;
	}

	return debugfs_create_dir(name, root_dir);
}
EXPORT_SYMBOL_GPL(cavan_debugfs_create_dir);

struct dentry *cavan_debugfs_create_symlink(const char *name, struct dentry *parent, const char *dest)
{
	if (root_dir == NULL) {
		return NULL;
	}

	return debugfs_create_symlink(name, root_dir, dest);
}
EXPORT_SYMBOL_GPL(cavan_debugfs_create_symlink);

static int __init cavan_debugfs_init(void)
{
	root_dir = debugfs_create_dir("cavan", NULL);
	if (root_dir == NULL) {
		pr_red_info("debugfs_create_dir");
		return -EFAULT;
	}

	return 0;
}

subsys_initcall(cavan_debugfs_init);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Cavan Debugfs");
MODULE_LICENSE("GPL");
