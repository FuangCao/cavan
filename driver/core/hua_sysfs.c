#include <huamobile/hua_sysfs.h>

static ssize_t hua_input_sysfs_show(struct kobject *kobj, struct attribute *attr, char *buff)
{
	struct hua_input_attribute *hua_attr = (struct hua_input_attribute *)attr;

	if (hua_attr->show == NULL || hua_attr->dev == NULL)
	{
		pr_red_info("hua_attr->show == NULL || hua_kobj->dev == NULL");
		return -EINVAL;
	}

	return hua_attr->show(hua_attr->dev, hua_attr, buff);
}

static ssize_t hua_input_sysfs_store(struct kobject *kobj,struct attribute *attr, const char *buff, size_t size)
{
	struct hua_input_attribute *hua_attr = (struct hua_input_attribute *)attr;

	if (hua_attr->store == NULL || hua_attr->dev == NULL)
	{
		pr_red_info("hua_attr->store == NULL || hua_kobj->dev == NULL");
		return -EINVAL;
	}

	return hua_attr->store(hua_attr->dev, hua_attr, buff, size);
}

static struct sysfs_ops hua_input_sysfs_ops =
{
	.show = hua_input_sysfs_show,
	.store = hua_input_sysfs_store
};

static struct kobj_type hua_input_kobj_type =
{
	.sysfs_ops = &hua_input_sysfs_ops
};

int hua_input_add_kobject(struct kobject *kobj, const char *name)
{
	int ret;

	if (kobj->state_initialized == 0)
	{
		kobject_init(kobj, &hua_input_kobj_type);
	}

	ret = kobject_add(kobj, NULL, name);
	if (ret < 0)
	{
		pr_red_info("kobject_add");
		return ret;
	}

	return 0;
}

EXPORT_SYMBOL_GPL(hua_input_add_kobject);

void hua_input_remove_kobject(struct kobject *kobj)
{
	kobject_del(kobj);
	kobject_put(kobj);
}

EXPORT_SYMBOL_GPL(hua_input_remove_kobject);

int hua_input_create_sysfs_files(struct hua_input_device *dev, struct kobject *kobj, struct hua_input_attribute *attrs, size_t count)
{
	struct hua_input_attribute *p, *p_end;

	for (p = attrs, p_end = p + count; p < p_end; p++)
	{
		int ret;

		p->dev = dev;

		ret = sysfs_create_file(kobj, &p->attr);
		if (ret < 0)
		{
			pr_red_info("sysfs_create_file %s failed", p->attr.name);

			for (p--; p >= attrs; p--)
			{
				sysfs_remove_file(kobj, &p->attr);
			}

			return ret;
		}
	}

	return 0;
}

EXPORT_SYMBOL_GPL(hua_input_create_sysfs_files);

void hua_input_remove_sysfs_files(struct kobject *kobj, struct hua_input_attribute *attrs, size_t count)
{
	struct hua_input_attribute *p, *p_end;

	for (p = attrs, p_end = p + count; p < p_end; p++)
	{
		sysfs_remove_file(kobj, &p->attr);
	}
}

EXPORT_SYMBOL_GPL(hua_input_remove_sysfs_files);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Huamobile Sysfs");
MODULE_LICENSE("GPL");
