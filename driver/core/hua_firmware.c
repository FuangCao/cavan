#include <huamobile/hua_input.h>
#include <huamobile/hua_firmware.h>

#define HUA_FW_DEBUG	0

static int hua_firmware_init(struct hua_firmware *fw, void *mem, size_t size)
{
	mutex_init(&fw->lock);

	fw->rdtask = NULL;
	fw->wrtask = NULL;

	fw->mem = mem;
	fw->mem_end = fw->mem + size;

	return 0;
}

static void hua_firmware_deinit(struct hua_firmware *fw)
{
	mutex_destroy(&fw->lock);
}

static int hua_firmware_thread_handler(void *data)
{
	int ret;
	struct hua_firmware *fw = data;

	pr_pos_info();

	ret = fw->upgrade(fw);

	mutex_lock(&fw->lock);

	fw->closed = 1;
	fw->status = ret;

	if (fw->wrtask) {
		wake_up_process(fw->wrtask);
	}

	mutex_unlock(&fw->lock);

	pr_green_info("Firmware thread complete with %d", ret);

	return 0;
}

static int hua_firmware_open(struct hua_firmware *fw, int (*upgrade)(struct hua_firmware *))
{
	int ret = 0;

	mutex_lock(&fw->lock);

	fw->closed = 0;
	fw->status = -EFAULT;
	fw->head = fw->tail = fw->mem;

	fw->upgrade = upgrade;
	fw->rdtask = kthread_create(hua_firmware_thread_handler, fw, "HUA-FIRMWARE");
	if (fw->rdtask == NULL) {
		ret = -EFAULT;
		pr_red_info("kthread_create");
	}

	mutex_unlock(&fw->lock);

	return ret;
}

static int hua_firmware_close(struct hua_firmware *fw)
{
	int i;
	int ret;

	mutex_lock(&fw->lock);

	if (fw->closed == 0) {
		fw->closed = -1;

		for (i = 0; i < 1000; i++) {
			if (fw->rdtask) {
				wake_up_process(fw->rdtask);
			}

			if (fw->wrtask) {
				wake_up_process(fw->wrtask);
			}

			mutex_unlock(&fw->lock);
			msleep(200);
			mutex_lock(&fw->lock);

			if (fw->closed > 0) {
				break;
			}

#if HUA_FW_DEBUG
			pr_bold_info("Wait process terminal");
#endif
		}

		if (fw->closed < 0) {
			if (fw->rdtask) {
				pr_red_info("kill read process");
				send_sig(SIGKILL, fw->rdtask, 0);
			}

			if (fw->wrtask) {
				pr_red_info("kill write process");
				send_sig(SIGKILL, fw->wrtask, 0);
			}
		}
	}

	ret = fw->status;

	mutex_unlock(&fw->lock);

	return ret;
}

struct hua_firmware *hua_firmware_create(size_t size, int (*upgrade)(struct hua_firmware *))
{
	struct hua_firmware *fw;

	if (upgrade == NULL) {
		pr_red_info("Please implement upgrade method");
		return NULL;
	}

	fw = kmalloc(sizeof(struct hua_firmware) + size, GFP_KERNEL);
	if (fw == NULL)
	{
		pr_red_info("malloc");
		return NULL;
	}

	if (hua_firmware_init(fw, (void *)(fw + 1), size) < 0)
	{
		pr_red_info("hua_firmware_init");
		goto out_kfree_fw;
	}

	if (hua_firmware_open(fw, upgrade) < 0) {
		pr_red_info("hua_firmware_open");
		goto out_hua_firmware_deinit;
	}

	return fw;

out_hua_firmware_deinit:
	hua_firmware_deinit(fw);
out_kfree_fw:
	kfree(fw);
	return NULL;
}

EXPORT_SYMBOL_GPL(hua_firmware_create);

int hua_firmware_destroy(struct hua_firmware *fw)
{
	int ret;

	ret = hua_firmware_close(fw);
	hua_firmware_deinit(fw);
	kfree(fw);

	return ret;
}

EXPORT_SYMBOL_GPL(hua_firmware_destroy);

ssize_t hua_firmware_free_space(struct hua_firmware *fw)
{
	if (fw->tail < fw->head)
	{
		return fw->head - fw->tail - 1;
	}
	else
	{
		return (fw->mem_end - fw->tail) + (fw->head - fw->mem) - 1;
	}
}

EXPORT_SYMBOL_GPL(hua_firmware_free_space);

ssize_t hua_firmware_used_space(struct hua_firmware *fw)
{
	if (fw->head > fw->tail)
	{
		return (fw->mem_end - fw->head) + (fw->tail - fw->mem);
	}
	else
	{
		return fw->tail - fw->head;
	}
}

EXPORT_SYMBOL_GPL(hua_firmware_used_space);

static void hua_firmware_wait(struct hua_firmware *fw)
{
	set_current_state(TASK_UNINTERRUPTIBLE);

	mutex_unlock(&fw->lock);
	schedule();
	mutex_lock(&fw->lock);
}

static long hua_firmware_timedwait(struct hua_firmware *fw, long timeout)
{
	long remain;

	set_current_state(TASK_UNINTERRUPTIBLE);

	mutex_unlock(&fw->lock);
	remain = schedule_timeout(timeout);
	mutex_lock(&fw->lock);

	return remain;
}

ssize_t hua_firmware_write(struct hua_firmware *fw, const char *buff, size_t size, int flags)
{
	const char *buff_bak, *buff_end;

	pr_bold_info("Write: size = %d, flags = %d", size, flags);

	buff_bak = buff;
	buff_end = buff + size;

	mutex_lock(&fw->lock);

	while (1)
	{
		size_t length, rcount;
		size_t remain;

		remain = buff_end - buff;
		if (remain == 0)
		{
			break;
		}

		while (1)
		{
			if (fw->tail < fw->head)
			{
				length = rcount = fw->head - fw->tail - 1;
			}
			else
			{
				rcount = fw->mem_end - fw->tail;
				length = rcount + (fw->head - fw->mem) - 1;
			}

#if HUA_FW_DEBUG
			pr_bold_info("Write: head = %p, tail = %p, rcount = %d, length = %d, free_space = %d",
				fw->head, fw->tail, rcount, length, hua_firmware_free_space(fw));
#endif

			if (length > 0)
			{
				break;
			}

			if (fw->closed)
			{
				mutex_unlock(&fw->lock);
				return fw->status;
			}

			fw->wrtask = current;
			hua_firmware_wait(fw);
			fw->wrtask = NULL;
		}

		if (length > remain)
		{
			length = remain;
		}

		if (length > rcount)
		{
			size_t lcount = length - rcount;

			if ((flags & HUA_FW_FLAG_USER)) {
				if (copy_from_user(fw->tail, buff, rcount)) {
					pr_red_info("copy_from_user");
					return -EFAULT;
				}

				if (copy_from_user(fw->mem, buff + rcount, lcount)) {
					pr_red_info("copy_from_user");
					return -EFAULT;
				}
			} else {
				memcpy(fw->tail, buff, rcount);
				memcpy(fw->mem, buff + rcount, lcount);
			}

			fw->tail = fw->mem + lcount;
		}
		else
		{
			if ((flags & HUA_FW_FLAG_USER)) {
				if (copy_from_user(fw->tail, buff, length)) {
					pr_red_info("copy_from_user");
					return -EFAULT;
				}
			} else {
				memcpy(fw->tail, buff, length);
			}

			fw->tail += length;
			if (fw->tail >= fw->mem_end)
			{
				fw->tail = fw->mem;
			}
		}

		buff += length;

		if (fw->rdtask) {
			wake_up_process(fw->rdtask);
		}
	}

	mutex_unlock(&fw->lock);

	return size;
}

EXPORT_SYMBOL_GPL(hua_firmware_write);

ssize_t hua_firmware_read(struct hua_firmware *fw, char *buff, size_t size, size_t reserved, u32 timeout)
{
	ssize_t length, rcount;

	mutex_lock(&fw->lock);

	while (1)
	{
		if (fw->head > fw->tail)
		{
			rcount = fw->mem_end - fw->head;
			length = rcount + (fw->tail - fw->mem);
		}
		else
		{
			length = rcount = fw->tail - fw->head;
		}

#if HUA_FW_DEBUG
		pr_bold_info("Read: head = %p, tail = %p, rcount = %d, length = %d, used_space = %d",
			fw->head, fw->tail, rcount, length, hua_firmware_used_space(fw));
#endif

		if ((size_t) length > reserved)
		{
			length -= reserved;
			break;
		}

		if (fw->closed)
		{
			length = 0;
			goto out_mutex_unlock;
		}

		fw->rdtask = current;

		if (timeout > 0) {
			if (hua_firmware_timedwait(fw, timeout) == 0) {
				length = -ETIMEDOUT;
				goto out_mutex_unlock;
			}
		} else {
			hua_firmware_wait(fw);
		}

		fw->rdtask = NULL;
	}

	if ((size_t) length > size)
	{
		length = size;
	}

	if (length > rcount)
	{
		size_t lcount = length - rcount;

		memcpy(buff, fw->head, rcount);
		memcpy(buff + rcount, fw->mem, lcount);
		fw->head = fw->mem + lcount;
	}
	else
	{
		memcpy(buff, fw->head, length);

		fw->head += length;
		if (fw->head >= fw->mem_end)
		{
			fw->head = fw->mem;
		}
	}

	if (fw->wrtask) {
		wake_up_process(fw->wrtask);
	}

out_mutex_unlock:
	mutex_unlock(&fw->lock);
	return length;
}

EXPORT_SYMBOL_GPL(hua_firmware_read);

ssize_t hua_firmware_fill(struct hua_firmware *fw, char *buff, size_t size, size_t reserved, u32 timeout)
{
	char *buff_bak = buff;
	char *buff_end = buff + size;

	while (buff < buff_end)
	{
		ssize_t rdlen = hua_firmware_read(fw, buff, buff_end - buff, reserved, timeout);
		if (rdlen < 0)
		{
			pr_red_info("hua_firmware_read");
			return rdlen;
		}

		if (rdlen == 0)
		{
			size = buff - buff_bak;
			break;
		}

		buff += rdlen;
	}

	return size;
}

EXPORT_SYMBOL_GPL(hua_firmware_fill);

ssize_t hua_firmware_read_line(struct hua_firmware *fw, char *buff, size_t size, size_t reserved, u32 timeout)
{
	char *buff_bak = buff;
	char *buff_end = buff + size - 1;

	while (buff < buff_end)
	{
		char c;
		ssize_t rdlen;

		rdlen = hua_firmware_read(fw, &c, 1, reserved, timeout);
		if (rdlen < 0)
		{
			pr_red_info("hua_firmware_read");
			return rdlen;
		}

		if (rdlen == 0)
		{
			break;
		}

		switch (c)
		{
		case '\n':
			if (buff > buff_bak)
			{
				goto out_return;
			}
		case '\r':
			break;

		default:
			*buff++ = c;
		}
	}

out_return:
	*buff = 0;
	return buff - buff_bak;
}

EXPORT_SYMBOL_GPL(hua_firmware_read_line);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Huamobile Firmware");
MODULE_LICENSE("GPL");
