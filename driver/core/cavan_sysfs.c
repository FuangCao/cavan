#include <cavan/cavan_sysfs.h>

static ssize_t cavan_input_sysfs_show(struct kobject *kobj, struct attribute *attr, char *buff)
{
	struct cavan_input_attribute *cavan_attr = (struct cavan_input_attribute *)attr;

	if (cavan_attr->show == NULL || cavan_attr->dev == NULL)
	{
		pr_red_info("cavan_attr->show == NULL || cavan_kobj->dev == NULL");
		return -EINVAL;
	}

	return cavan_attr->show(cavan_attr->dev, cavan_attr, buff);
}

static ssize_t cavan_input_sysfs_store(struct kobject *kobj,struct attribute *attr, const char *buff, size_t size)
{
	struct cavan_input_attribute *cavan_attr = (struct cavan_input_attribute *)attr;

	if (cavan_attr->store == NULL || cavan_attr->dev == NULL)
	{
		pr_red_info("cavan_attr->store == NULL || cavan_kobj->dev == NULL");
		return -EINVAL;
	}

	return cavan_attr->store(cavan_attr->dev, cavan_attr, buff, size);
}

static struct sysfs_ops cavan_input_sysfs_ops =
{
	.show = cavan_input_sysfs_show,
	.store = cavan_input_sysfs_store
};

static struct kobj_type cavan_input_kobj_type =
{
	.sysfs_ops = &cavan_input_sysfs_ops
};

int cavan_input_add_kobject(struct kobject *kobj, const char *name)
{
	int ret;

	if (kobj->state_initialized == 0)
	{
		kobject_init(kobj, &cavan_input_kobj_type);
	}

	ret = kobject_add(kobj, NULL, name);
	if (ret < 0)
	{
		pr_red_info("kobject_add");
		return ret;
	}

	return 0;
}

EXPORT_SYMBOL_GPL(cavan_input_add_kobject);

void cavan_input_remove_kobject(struct kobject *kobj)
{
	kobject_del(kobj);
	kobject_put(kobj);
}

EXPORT_SYMBOL_GPL(cavan_input_remove_kobject);

int cavan_input_create_sysfs_files(struct cavan_input_device *dev, struct kobject *kobj, struct cavan_input_attribute *attrs, size_t count)
{
	struct cavan_input_attribute *p, *p_end;

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

EXPORT_SYMBOL_GPL(cavan_input_create_sysfs_files);

void cavan_input_remove_sysfs_files(struct kobject *kobj, struct cavan_input_attribute *attrs, size_t count)
{
	struct cavan_input_attribute *p, *p_end;

	for (p = attrs, p_end = p + count; p < p_end; p++)
	{
		sysfs_remove_file(kobj, &p->attr);
	}
}

EXPORT_SYMBOL_GPL(cavan_input_remove_sysfs_files);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Cavan Sysfs");
MODULE_LICENSE("GPL");
