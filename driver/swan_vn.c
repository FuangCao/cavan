#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/crc32.h>
#include <linux/ethtool.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#define SWAN_VN_DEVICE_NAME		"swan_vn"
#define SWAN_VN_SKB_QUEUE_SIZE	10

#define pr_red_info(fmt, args ...) \
	printk(KERN_INFO "\033[31m" fmt "\033[0m\n", ##args)

#define pr_green_info(fmt, args ...) \
	printk(KERN_INFO "\033[32m" fmt "\033[0m\n", ##args)

#define pr_blue_info(fmt, args ...) \
	printk(KERN_INFO "\033[34m" fmt "\033[0m\n", ##args)

#define pr_bold_info(fmt, args ...) \
	printk(KERN_INFO "\033[1m" fmt "\033[0m\n", ##args)

#define pr_pos_info() \
	pr_green_info("%s => %s[%d]", __FILE__, __FUNCTION__, __LINE__)

struct swan_virtual_ethernet
{
	struct net_device *ndev;
	struct sk_buff *skb_queue[SWAN_VN_SKB_QUEUE_SIZE];
	int head, tail;
	struct mutex skb_queue_lock;
	spinlock_t	vn_lock;
	struct semaphore skb_queue_sem;
	atomic_t open_excl;
};

static u8 swan_vn_mac[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};

static inline int swan_virtual_ethernet_skb_queue_full(struct swan_virtual_ethernet *vn)
{
	return vn->tail + 1 == vn->head;
}

static inline int swan_virtual_ethernet_skb_queue_empty(struct swan_virtual_ethernet *vn)
{
	return vn->tail == vn->head;
}

static int swan_virtual_ethernet_skb_enqueue(struct swan_virtual_ethernet *vn, struct sk_buff *skb)
{
	if (swan_virtual_ethernet_skb_queue_full(vn))
	{
		return -ENOMEM;
	}

	mutex_lock(&vn->skb_queue_lock);

	vn->skb_queue[vn->tail++] = skb;

	if (vn->tail >= SWAN_VN_SKB_QUEUE_SIZE)
	{
		vn->tail = 0;
	}

	mutex_unlock(&vn->skb_queue_lock);

	up(&vn->skb_queue_sem);

	return 0;
}

static struct sk_buff *swan_virtual_ethernet_skb_outqueue(struct swan_virtual_ethernet *vn)
{
	struct sk_buff *skb;

	if (down_interruptible(&vn->skb_queue_sem) < 0)
	{
		pr_red_info("down_interruptible");
		return NULL;
	}

	mutex_lock(&vn->skb_queue_lock);

	skb = vn->skb_queue[vn->head++];

	if (vn->head >= SWAN_VN_SKB_QUEUE_SIZE)
	{
		vn->head = 0;
	}

	mutex_unlock(&vn->skb_queue_lock);

	return skb;
}

static int swan_virtual_ethernet_ndo_open(struct net_device *dev)
{
	pr_pos_info();

	return 0;
}

static int swan_virtual_ethernet_ndo_stop(struct net_device *dev)
{
	pr_pos_info();

	return 0;
}

static netdev_tx_t swan_virtual_ethernet_ndo_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct swan_virtual_ethernet *vn;
	unsigned long irq_flags;

	vn = netdev_priv(dev);

	if (swan_virtual_ethernet_skb_enqueue(vn, skb) < 0)
	{
		return NETDEV_TX_BUSY;
	}

	spin_lock_irqsave(&vn->vn_lock, irq_flags);

	dev->stats.tx_bytes += skb->len;
	dev->stats.tx_packets++;

	if (swan_virtual_ethernet_skb_queue_full(vn))
	{
		netif_stop_queue(dev);
	}

	spin_unlock_irqrestore(&vn->vn_lock, irq_flags);

	return NETDEV_TX_OK;
}

static void swan_virtual_ethernet_ndo_tx_timeout(struct net_device *dev)
{
	pr_pos_info();
}

static struct net_device_ops swan_virtual_ethernet_netdev_ops =
{
	.ndo_open = swan_virtual_ethernet_ndo_open,
	.ndo_stop = swan_virtual_ethernet_ndo_stop,
	.ndo_start_xmit = swan_virtual_ethernet_ndo_start_xmit,
	.ndo_tx_timeout = swan_virtual_ethernet_ndo_tx_timeout,
	.ndo_change_mtu = eth_change_mtu,
	.ndo_validate_addr = eth_validate_addr,
	.ndo_set_mac_address = eth_mac_addr,
};

static struct ethtool_ops swan_virtual_ethernet_ethtool_ops;

static ssize_t swan_virtual_ethernet_command_store(struct device *dev, struct device_attribute *attr, const char *buff, size_t count)
{
	return count;
}

static ssize_t swan_virtual_ethernet_write_data(struct swan_virtual_ethernet *vn, const void *buff, size_t count)
{
	struct sk_buff *skb;
	struct net_device *ndev = vn->ndev;
	unsigned long irq_flags;

	skb = dev_alloc_skb(count);
	if (skb == NULL)
	{
		pr_red_info("dev_alloc_skb");
		return -ENOMEM;
	}

	if (copy_from_user(skb_put(skb, count), buff, count))
	{
		pr_red_info("copy_from_user failed");
		dev_kfree_skb(skb);
		return -EFAULT;
	}

	spin_lock_irqsave(&vn->vn_lock, irq_flags);

	skb->protocol = eth_type_trans(skb, ndev);
	netif_rx(skb);
	ndev->stats.rx_bytes += count;
	ndev->stats.rx_packets++;

	spin_unlock_irqrestore(&vn->vn_lock, irq_flags);

	return count;
}

static ssize_t swan_virtual_ethernet_data_store(struct device *dev, struct device_attribute *attr, const char *buff, size_t count)
{
	return swan_virtual_ethernet_write_data(dev_get_drvdata(dev), buff, count);
}

ssize_t swan_virtual_ethernet_read_data(struct swan_virtual_ethernet *vn, void *buff)
{
	struct sk_buff *skb;
	ssize_t readlen;
	unsigned long irq_flags;

	skb = swan_virtual_ethernet_skb_outqueue(vn);
	if (skb == NULL)
	{
		return -ENOMEM;
	}

	readlen = skb->len;
	if (copy_to_user(buff, skb->data, readlen))
	{
		pr_red_info("copy_from_user");
		readlen = -1;
	}

	spin_lock_irqsave(&vn->vn_lock, irq_flags);

	dev_kfree_skb(skb);
	netif_wake_queue(vn->ndev);

	spin_unlock_irqrestore(&vn->vn_lock, irq_flags);

	return readlen;
}

ssize_t swan_virtual_ethernet_data_show(struct device *dev, struct device_attribute *attr, char *buff)
{
	return swan_virtual_ethernet_read_data(dev_get_drvdata(dev), buff);
}

const struct device_attribute vn_attrs[] =
{
	{
		.attr =
		{
			.name = "command",
			.mode = S_IWUGO,
		},

		.show = NULL,
		.store = swan_virtual_ethernet_command_store,
	},
	{
		.attr =
		{
			.name = "data",
			.mode = S_IWUGO | S_IRUGO,
		},

		.show = swan_virtual_ethernet_data_show,
		.store = swan_virtual_ethernet_data_store,
	},
};

static struct swan_virtual_ethernet *global_swan_vn;

static ssize_t swan_virtual_ethernet_read(struct file *file_desc, char __user *buff, size_t size, loff_t *offset)
{
	return swan_virtual_ethernet_read_data(global_swan_vn, buff);
}

static ssize_t swan_virtual_ethernet_write(struct file *file_desc, const char __user *buff, size_t size, loff_t *offset)
{
	return swan_virtual_ethernet_write_data(global_swan_vn, buff, size);
}

static int swan_virtual_ethernet_open(struct inode *inode_desc, struct file *file_desc)
{
	if (global_swan_vn == NULL)
	{
		return -ENOENT;
	}

	if (atomic_inc_return(&global_swan_vn->open_excl) > 1)
	{
		atomic_dec(&global_swan_vn->open_excl);
		return -EBUSY;
	}

	return 0;
}

static int swan_virtual_ethernet_release(struct inode *inode_desc, struct file *file_desc)
{
	atomic_dec(&global_swan_vn->open_excl);

	return 0;
}

static struct file_operations swan_virtual_ethernet_ops =
{
	.owner = THIS_MODULE,
	.read = swan_virtual_ethernet_read,
	.write = swan_virtual_ethernet_write,
	.open = swan_virtual_ethernet_open,
	.release = swan_virtual_ethernet_release,
};

static struct miscdevice swan_virtual_ethernet_miscdev =
{
	.minor = MISC_DYNAMIC_MINOR,
	.name = "swan_vn",
	.fops = &swan_virtual_ethernet_ops,
};

static int swan_virtual_ethernet_probe(struct platform_device *pdev)
{
	int ret;
	struct swan_virtual_ethernet *swan_vn;
	struct net_device *ndev;
	const struct device_attribute *p, *end_p;

	pr_pos_info();

	ndev = alloc_etherdev(sizeof(struct swan_virtual_ethernet));
	if (ndev == NULL)
	{
		pr_red_info("alloc_etherdev");
		return -ENOMEM;
	}

	swan_vn = netdev_priv(ndev);

	SET_NETDEV_DEV(ndev, &pdev->dev);
	ether_setup(ndev);
	ndev->netdev_ops = &swan_virtual_ethernet_netdev_ops;
	ndev->watchdog_timeo = msecs_to_jiffies(5000);
	ndev->ethtool_ops = &swan_virtual_ethernet_ethtool_ops;
	ndev->dev_addr = swan_vn_mac;

	ret = register_netdev(ndev);
	if (ret < 0)
	{
		pr_red_info("register_netdev");

		goto out_free_netdev;
	}

	global_swan_vn = swan_vn;

	ret = misc_register(&swan_virtual_ethernet_miscdev);
	if (ret < 0)
	{
		pr_red_info("misc_register");
		goto out_unregister_netdev;
	}

	for (p = vn_attrs, end_p = vn_attrs + ARRAY_SIZE(vn_attrs); p < end_p; p++)
	{
		ret = device_create_file(&pdev->dev, p);
		if (ret < 0)
		{
			pr_red_info("device_create_file");

			while (--p >= vn_attrs)
			{
				device_remove_file(&pdev->dev, p);
			}

			goto out_misc_deregister;
		}
	}

	swan_vn->ndev = ndev;
	swan_vn->head = swan_vn->tail = 0;
	mutex_init(&swan_vn->skb_queue_lock);
	sema_init(&swan_vn->skb_queue_sem, 0);
	spin_lock_init(&swan_vn->vn_lock);
	atomic_set(&swan_vn->open_excl, 0);
	platform_set_drvdata(pdev, swan_vn);

	return 0;

out_misc_deregister:
	misc_deregister(&swan_virtual_ethernet_miscdev);
out_unregister_netdev:
	unregister_netdev(ndev);
out_free_netdev:
	free_netdev(ndev);

	return 0;
}

static int swan_virtual_ethernet_remove(struct platform_device *pdev)
{
	struct swan_virtual_ethernet *swan_vn;
	const struct device_attribute *p, *end_p;
	struct net_device *ndev;

	pr_pos_info();

	swan_vn = platform_get_drvdata(pdev);
	ndev = swan_vn->ndev;

	for (p = vn_attrs, end_p = vn_attrs + ARRAY_SIZE(vn_attrs); p < end_p; p++)
	{
		device_remove_file(&pdev->dev, p);
	}

	misc_deregister(&swan_virtual_ethernet_miscdev);
	unregister_netdev(ndev);
	free_netdev(ndev);

	return 0;
}

static struct platform_driver swan_virtual_ethernet_driver =
{
	.driver =
	{
		.name = SWAN_VN_DEVICE_NAME,
	},

	.probe = swan_virtual_ethernet_probe,
	.remove = swan_virtual_ethernet_remove,
};

static struct platform_device swan_virtual_ethernet_device =
{
	.name = SWAN_VN_DEVICE_NAME,
};

static __devinit int swan_vitual_ethernet_init(void)
{
	int ret;

	pr_pos_info();

	ret = platform_device_register(&swan_virtual_ethernet_device);
	if (ret < 0)
	{
		pr_red_info("platform_device_register");
		return ret;
	}

	ret = platform_driver_register(&swan_virtual_ethernet_driver);
	if (ret < 0)
	{
		pr_red_info("platform_driver_register");
		goto out_unregister_device;
	}

	return 0;

out_unregister_device:
	platform_device_unregister(&swan_virtual_ethernet_device);

	return ret;
}

static __devexit void swan_virtual_ethernet_exit(void)
{
	pr_pos_info();

	platform_device_unregister(&swan_virtual_ethernet_device);
	platform_driver_unregister(&swan_virtual_ethernet_driver);
}

module_init(swan_vitual_ethernet_init);
module_exit(swan_virtual_ethernet_exit);

MODULE_AUTHOR("cavan <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Swan Virtual Ethernet Driver");
MODULE_LICENSE("GPL");
