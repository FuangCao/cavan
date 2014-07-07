// Fuang.Cao <cavan.cfa@gmail.com> Wed Sep  7 09:54:06 CST 2011

#include <cavan.h>
#include <cavan/usb.h>
#include <dirent.h>

int dump_cavan_usb_descriptor(const char *buff, struct cavan_usb_descriptor *desc, size_t length)
{
	struct usb_device_descriptor *dev_desc;
	struct usb_config_descriptor *cfg_desc, *cfg_desc_end;
	const char *end_buff;

	if (length < USB_DT_DEVICE_SIZE + USB_DT_CONFIG_SIZE)
	{
		pr_red_info("length is invalid");
		return -EINVAL;
	}

	dev_desc = &desc->dev_desc;

	if (is_usb_device_descriptor((struct usb_device_descriptor *)buff))
	{
		memcpy(dev_desc, buff, USB_DT_DEVICE_SIZE);
		buff += USB_DT_DEVICE_SIZE;
	}
	else
	{
		pr_red_info("invalid usb device descriptor");
		return -EINVAL;
	}

	for (cfg_desc = desc->cfg_descs, cfg_desc_end = cfg_desc + dev_desc->bNumConfigurations; cfg_desc < cfg_desc_end; cfg_desc++)
	{
		if (is_usb_config_descriptor((struct usb_config_descriptor *)buff))
		{
			memcpy(cfg_desc, buff, USB_DT_CONFIG_SIZE);
			buff += USB_DT_CONFIG_SIZE;
		}
		else
		{
			pr_red_info("invalid usb config descriptor");
			return -EINVAL;
		}
	}

	desc->if_count = 0;
	end_buff = buff + length - USB_DT_INTERFACE_SIZE;

	while (buff <= end_buff && is_usb_interface_descriptor((struct usb_interface_descriptor *)buff))
	{
		struct usb_interface_descriptor *if_desc;
		struct cavan_usb_interface_descriptor *cavan_if_desc;
		struct usb_endpoint_descriptor *ep, *ep_end;

		cavan_if_desc = desc->if_descs + desc->if_count;
		if_desc = &cavan_if_desc->if_desc;

		memcpy(if_desc, buff, USB_DT_INTERFACE_SIZE);
		buff += USB_DT_INTERFACE_SIZE;

		for (ep = cavan_if_desc->ep_descs, ep_end = ep + if_desc->bNumEndpoints; ep < ep_end; ep++)
		{
			memcpy(ep, buff, USB_DT_ENDPOINT_SIZE);
			buff += USB_DT_ENDPOINT_SIZE;
		}

		desc->if_count++;
	}

	return 0;
}

static int usb_clam_interface(int fd, int if_num, int serial_num, char *serial)
{
	int i;
	int ret;
	struct usbdevfs_ctrltransfer ctrl;
	u16 languages[128], buff[128];
	int language_count;

	ret = ioctl(fd, USBDEVFS_CLAIMINTERFACE, &if_num);
	if (ret < 0)
	{
		return ret;
	}

	memset(&ctrl, 0, sizeof(ctrl));

	ctrl.bRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
	ctrl.bRequest = USB_REQ_GET_DESCRIPTOR;
	ctrl.wValue = (USB_DT_STRING << 8) | 0;
	ctrl.wLength = sizeof(languages);
	ctrl.data = languages;

	ret = ioctl(fd, USBDEVFS_CONTROL, &ctrl);
	if (ret < 0)
	{
		return ret;
	}

	if (ret < 2)
	{
		return 0;
	}

	language_count = (ret >> 1) - 1;

	ctrl.wLength = sizeof(buff);
	ctrl.data = buff;

	for (i = 1; i <= language_count; i++)
	{
		ctrl.wValue = (USB_DT_STRING << 8) | serial_num;
		ctrl.wIndex = languages[i];

		ret = ioctl(fd, USBDEVFS_CONTROL, &ctrl);
		if (ret > 0)
		{
			u16 *p = buff + 1, *end_p = buff + (ret >> 1);

			while (p < end_p)
			{
				*serial++ = *p++;
			}

			break;
		}
	}

	*serial = 0;

	return 0;
}

int fusb_read_cavan_descriptor(int fd, struct cavan_usb_descriptor *desc)
{
	ssize_t readlen;
	char buff[512];

	readlen = read(fd, buff, sizeof(buff));
	if (readlen < 0)
	{
		error_msg("read");
		return readlen;
	}

	return dump_cavan_usb_descriptor(buff, desc, readlen);
}

int usb_read_cavan_descriptor(const char *dev_path, struct cavan_usb_descriptor *desc)
{
	int ret;
	int fd;

	fd = open(dev_path, O_RDONLY);
	if (fd < 0)
	{
		error_msg("open device \"%s\" failed", dev_path);
		return fd;
	}

	ret = fusb_read_cavan_descriptor(fd, desc);

	close(fd);

	return ret;
}

const char *usb_endpoint_xfertype_tostring(const struct usb_endpoint_descriptor *desc)
{
	switch (desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
	{
	case USB_ENDPOINT_XFER_CONTROL:
		return "control";

	case USB_ENDPOINT_XFER_ISOC:
		return "isoc";

	case USB_ENDPOINT_XFER_BULK:
		return "bluk";

	case USB_ENDPOINT_XFER_INT:
		return "int";

	default:
		return "unknown";
	}
}

void show_usb_device_descriptor(const struct usb_device_descriptor *desc)
{
	println("bLength = 0x%02x", desc->bLength);
	println("bDescriptorType = 0x%02x", desc->bDescriptorType);
	println("bcdUSB = 0x%04x", desc->bcdUSB);
	println("bDeviceClass = 0x%02x", desc->bDeviceClass);
	println("bDeviceSubClass = 0x%02x", desc->bDeviceSubClass);
	println("bDeviceProtocol = 0x%02x", desc->bDeviceProtocol);
	println("bMaxPacketSize0 = 0x%02x", desc->bMaxPacketSize0);
	println("idVendor = 0x%04x", desc->idVendor);
	println("idProduct = 0x%04x", desc->idProduct);
	println("bcdDevice = 0x%04x", desc->bcdDevice);
	println("iManufacturer = 0x%02x", desc->iManufacturer);
	println("iProduct = 0x%02x", desc->iProduct);
	println("iSerialNumber = 0x%02x", desc->iSerialNumber);
	pr_bold_info("bNumConfigurations = 0x%02x", desc->bNumConfigurations);
}

void show_usb_config_descriptor(const struct usb_config_descriptor *desc)
{
	println("bLength = 0x%02x", desc->bLength);
	println("bDescriptorType = 0x%02x", desc->bDescriptorType);
	println("wTotalLength = 0x%04x", desc->wTotalLength);
	println("bNumInterfaces = 0x%02x", desc->bNumInterfaces);
	println("bConfigurationValue = 0x%02x", desc->bConfigurationValue);
	println("iConfiguration = 0x%02x", desc->iConfiguration);
	println("bmAttributes = 0x%02x", desc->bmAttributes);
	println("bMaxPower = 0x%02x", desc->bMaxPower);
}

void show_usb_interface_descriptor(const struct usb_interface_descriptor *desc)
{
	println("bLength = 0x%02x", desc->bLength);
	println("bDescriptorType = 0x%02x", desc->bDescriptorType);
	println("bInterfaceNumber = 0x%02x", desc->bInterfaceNumber);
	println("bAlternateSetting = 0x%02x", desc->bAlternateSetting);
	pr_bold_info("bNumEndpoints = 0x%02x", desc->bNumEndpoints);
	println("bInterfaceClass = 0x%02x", desc->bInterfaceClass);
	println("bInterfaceSubClass = 0x%02x", desc->bInterfaceSubClass);
	println("bInterfaceProtocol = 0x%02x", desc->bInterfaceProtocol);
	println("iInterface = 0x%02x", desc->iInterface);
}

void show_usb_endpoint_descriptor(const struct usb_endpoint_descriptor *desc)
{
	pr_bold_info("xfertype = %s", usb_endpoint_xfertype_tostring(desc));
	pr_bold_info("Direction = %s", desc->bEndpointAddress & USB_ENDPOINT_DIR_MASK ? "input" : "output");
	println("bLength = 0x%02x", desc->bLength);
	println("bDescriptorType = 0x%02x", desc->bDescriptorType);
	println("bEndpointAddress = 0x%02x", desc->bEndpointAddress);
	println("bmAttributes = 0x%02x", desc->bmAttributes);
	println("wMaxPacketSize = 0x%04x", desc->wMaxPacketSize);
	println("bInterval = 0x%02x", desc->bInterval);
	println("bRefresh = 0x%02x", desc->bRefresh);
	println("bSynchAddress = 0x%02x", desc->bSynchAddress);
}

void show_cavan_usb_descriptor(const struct cavan_usb_descriptor *desc)
{
	int i, j, count;

	print_sep(60);
	pr_bold_info("usb deivce descriptor:");
	show_usb_device_descriptor(&desc->dev_desc);
	print_sep(60);

	for (i = 0, count = desc->dev_desc.bNumConfigurations; i < count; i++)
	{
		pr_bold_info("usb config descriptor[%d]:", i);
		show_usb_config_descriptor(desc->cfg_descs + i);
	}

	print_sep(60);

	pr_bold_info("interface descriptor count = %d", desc->if_count);

	for (i = 0, count = desc->if_count; i < count; i++)
	{
		pr_bold_info("usb interface descriptor[%d]:", i);
		show_usb_interface_descriptor(&desc->if_descs[i].if_desc);

		for (j = 0; j < desc->if_descs[i].if_desc.bNumEndpoints; j++)
		{
			pr_bold_info("usb endpoint[%d]:", j);
			show_usb_endpoint_descriptor(desc->if_descs[i].ep_descs + j);
		}
	}

	print_sep(60);
}

static void *cavan_usb_notify_handle(void *data)
{
	int ret;
	struct cavan_usb_descriptor *desc = data;
	struct usbdevfs_urb *urb_out, *urb_read, *urb_write;
	pthread_mutex_t *lock;
	pthread_cond_t *notify_read, *notify_write;

	pr_green_pos();

	urb_read = &desc->urb_read;
	urb_write = &desc->urb_write;
	lock = &desc->lock;
	notify_read = &desc->notify_read;
	notify_write = &desc->notify_write;

	while (1)
	{
		while (1)
		{
			ret = ioctl(desc->fd, USBDEVFS_REAPURB, &urb_out);
			if (ret < 0 && errno == EINTR)
			{
				msleep(100);
			}
			else
			{
				break;
			}
		}

		if (ret < 0 && errno == ENOENT)
		{
			break;
		}

		if (urb_out == urb_read)
		{
			cavan_cond_signal(notify_read, lock);
		}
		else if (urb_out == urb_write)
		{
			cavan_cond_signal(notify_write, lock);
		}
		else
		{
			pr_red_info("invalid urb");
		}
	}

	pr_red_info("cavan usb notify thread exit");

	return NULL;
}

int cavan_usb_init(const char *dev_path, struct cavan_usb_descriptor *desc)
{
	int i, j;
	int fd;
	int ret;

	fd = open(dev_path, O_RDWR);
	if (fd < 0)
	{
		error_msg("open device \"%s\" failed", dev_path);
		return fd;
	}

	ret = fusb_read_cavan_descriptor(fd, desc);
	if (ret < 0)
	{
		pr_red_info("fusb_read_cavan_descriptor");
		goto out_close_fd;
	}

#if CAVAN_USB_DEBUG
	show_cavan_usb_descriptor(desc);
#endif

	for (i = desc->if_count - 1; i >= 0; i--)
	{
		if (usb_clam_interface(fd, desc->if_descs[i].if_desc.bInterfaceNumber, desc->dev_desc.iSerialNumber, desc->serial) < 0)
		{
			continue;
		}

		desc->epin_curr = -1;
		desc->epout_curr = -1;

		for (j = desc->if_descs[i].if_desc.bNumEndpoints - 1; j >= 0; j--)
		{
			if (desc->if_descs[i].ep_descs[j].bEndpointAddress & USB_ENDPOINT_DIR_MASK)
			{
				desc->epin_curr = desc->if_descs[i].ep_descs[j].bEndpointAddress;
			}
			else
			{
				desc->epout_curr = desc->if_descs[i].ep_descs[j].bEndpointAddress;
			}
		}

		if (desc->epin_curr > 0 && desc->epout_curr > 0)
		{
			break;
		}
	}

	if (i < 0)
	{
		return -ENOENT;
	}

	memset(&desc->urb_read, 0, sizeof(desc->urb_read));
	memset(&desc->urb_write, 0, sizeof(desc->urb_write));
	desc->urb_read.type = desc->urb_write.type = USBDEVFS_URB_TYPE_BULK;
	desc->urb_read.endpoint = desc->epin_curr;
	desc->urb_write.endpoint = desc->epout_curr;

	desc->fd = fd;
	text_copy(desc->dev_path, dev_path);
	pthread_mutex_init(&desc->lock, 0);
	pthread_cond_init(&desc->notify_read, 0);
	pthread_cond_init(&desc->notify_write, 0);

	ret = pthread_create(&desc->thread_notify, NULL, cavan_usb_notify_handle, desc);
	if (ret < 0)
	{
		print_error("pthread_create");
		goto out_close_fd;
	}

	return 0;

out_close_fd:
	close(fd);

	return ret;
}

void cavan_usb_deinit(struct cavan_usb_descriptor *desc)
{
#ifndef CONFIG_BUILD_FOR_ANDROID
	pthread_cancel(desc->thread_notify);
#endif

	close(desc->fd);
}

int cavan_usb_bluk_rw(struct cavan_usb_descriptor *desc, void *buff, size_t length, int read)
{
	int ret;
	struct usbdevfs_urb *urb;
	pthread_cond_t *notify;
	int fd = desc->fd;
	pthread_mutex_t *lock = &desc->lock;

	if (read)
	{
		urb = &desc->urb_read;
		notify = &desc->notify_read;
	}
	else
	{
		urb = &desc->urb_write;
		notify = &desc->notify_write;
	}

	urb->status = -1;
	urb->buffer = buff;
	urb->buffer_length = length;

	pthread_mutex_lock(lock);

	while (1)
	{
		ret = ioctl(fd, USBDEVFS_SUBMITURB, urb);
		if (ret < 0 && errno == EINTR)
		{
			msleep(100);
		}
		else
		{
			break;
		}
	}

	pthread_mutex_unlock(lock);

	if (ret < 0)
	{
		print_error("ioctl");
		return ret;
	}

	cavan_cond_wait(notify, lock);

	return urb->actual_length;
}

int cavan_usb_bluk_read(struct cavan_usb_descriptor *desc, void *buff, size_t length)
{
	if (desc == NULL || desc->epin_curr < 0)
	{
		return -EINVAL;
	}

	return cavan_usb_bluk_rw(desc, buff, length, 1);
}

int cavan_usb_bluk_write(struct cavan_usb_descriptor *desc, const void *buff, size_t length)
{
	if (desc == NULL || desc->epout_curr < 0)
	{
		return -EINVAL;
	}

	return cavan_usb_bluk_rw(desc, (void *) buff, length, 0);
}

int cavan_usb_bluk_xfer(struct cavan_usb_descriptor *desc, void *buff, size_t length, int ep)
{
	int fd = desc->fd;
	struct usbdevfs_bulktransfer bulk =
	{
		.ep = ep,
		.len = length,
		.timeout = 0,
		.data = buff
	};

	return ioctl(fd, USBDEVFS_BULK, &bulk);
}

int cavan_usb_bluk_read2(struct cavan_usb_descriptor *desc, void *buff, size_t length)
{
	if (desc == NULL || desc->epin_curr < 0)
	{
		return -EINVAL;
	}

	return cavan_usb_bluk_xfer(desc, buff, length, desc->epin_curr);
}

int cavan_usb_bluk_write2(struct cavan_usb_descriptor *desc, const void *buff, size_t length)
{
	if (desc == NULL || desc->epout_curr < 0)
	{
		return -EINVAL;
	}

	return cavan_usb_bluk_xfer(desc, (void *) buff, length, desc->epout_curr);
}

int cavan_find_usb_device(const char *dev_path, struct cavan_usb_descriptor *desc)
{
	int ret;
	char *p1, *p2;
	DIR *dir1, *dir2;
	char tmp_path[1024];
	struct dirent *dt;

	if (dev_path && cavan_usb_init(dev_path, desc) >= 0)
	{
		return 0;
	}

	p1 = text_path_cat(tmp_path, sizeof(tmp_path), USB_DEVICE_DIR, NULL);

	dir1 = opendir(tmp_path);
	if (dir1 == NULL)
	{
		print_error("open directory \"%s\" failed", tmp_path);
		return -ENOENT;
	}

	while ((dt = readdir(dir1)))
	{
		if (text_is_dot_name(dt->d_name))
		{
			continue;
		}

		p2 = text_copy(p1, dt->d_name);

		dir2 = opendir(tmp_path);
		if (dir2 == NULL)
		{
			warning_msg("open directory \"%s\" failed", tmp_path);
			continue;
		}

		p2 = text_copy(p2, "/");

		while ((dt = readdir(dir2)))
		{
			if (text_is_dot_name(dt->d_name))
			{
				continue;
			}

			text_copy(p2, dt->d_name);

			ret = cavan_usb_init(tmp_path, desc);
			if (ret >= 0)
			{
				goto out_close_dir2;
			}
		}

		closedir(dir2);
	}

	ret = -ENOENT;
	goto out_close_dir1;

out_close_dir2:
	closedir(dir2);
out_close_dir1:
	closedir(dir1);

	return ret;
}

ssize_t cavan_usb_read_data(struct cavan_usb_descriptor *desc, void *buff, size_t size)
{
	ssize_t readlen;
	struct cavan_usb_data_header hdr;

	readlen = cavan_usb_bluk_read(desc, &hdr, sizeof(hdr));
	if (readlen < (ssize_t) sizeof(hdr))
	{
		pr_red_pos();
		return readlen < 0 ? readlen : -ENOMEDIUM;
	}

	if ((hdr.data_length ^ hdr.data_check) != 0xFFFF)
	{
		pr_bold_pos();
		return -EINVAL;
	}

	return cavan_usb_bluk_read(desc, buff, hdr.data_length);
}

ssize_t cavan_usb_write_data(struct cavan_usb_descriptor *desc, const void *buff, size_t size)
{
	ssize_t writelen;
	struct cavan_usb_data_header hdr =
	{
		.data_length = size,
		.data_check = ~size
	};

	writelen = cavan_usb_bluk_write(desc, &hdr, sizeof(hdr));
	if (writelen < (ssize_t) sizeof(hdr))
	{
		pr_red_pos();
		return writelen < 0 ? writelen : -ENOMEDIUM;
	}

	return cavan_usb_bluk_write(desc, buff, size);
}

ssize_t cavan_adb_read_data(int fd_adb, void *buff, size_t size)
{
	ssize_t readlen;
	struct cavan_usb_data_header hdr;

	readlen = read(fd_adb, &hdr, sizeof(hdr));
	if (readlen < (ssize_t) sizeof(hdr))
	{
		pr_red_pos();
		return readlen < 0 ? readlen : -ENOMEDIUM;
	}

	if ((hdr.data_length ^ hdr.data_check) != 0xFFFF)
	{
		pr_red_pos();
		return -EINVAL;
	}

	return read(fd_adb, buff, hdr.data_length);
}

ssize_t cavan_adb_write_data(int fd_adb, const void *buff, size_t size)
{
	ssize_t writelen;
	struct cavan_usb_data_header hdr =
	{
		.data_length = size,
		.data_check = ~size
	};

	writelen = write(fd_adb, &hdr, sizeof(hdr));
	if (writelen < (ssize_t) sizeof(hdr))
	{
		print_error("write");
		return writelen < 0 ? writelen : -ENOMEDIUM;
	}

	return write(fd_adb, buff, size);
}
