#pragma once

#include <linux/kernel.h>

#define CAVAN_FW_FLAG_USER	(1 << 0)

struct cavan_firmware
{
	int closed;
	struct mutex lock;
	void *private_data;
	struct task_struct *rdtask;
	struct task_struct *wrtask;
	int status;
	char *head, *tail;
	char *mem, *mem_end;

	int (*upgrade)(struct cavan_firmware *fw);
};

struct cavan_firmware *cavan_firmware_create(size_t size, int (*upgrade)(struct cavan_firmware *));
int cavan_firmware_destroy(struct cavan_firmware *fw);
ssize_t cavan_firmware_free_space(struct cavan_firmware *fw);
ssize_t cavan_firmware_used_space(struct cavan_firmware *fw);
ssize_t cavan_firmware_write(struct cavan_firmware *fw, const char *buff, size_t size, int flags);
ssize_t cavan_firmware_read(struct cavan_firmware *fw, char *buff, size_t size, size_t reserved, u32 timeout);
ssize_t cavan_firmware_fill(struct cavan_firmware *fw, char *buff, size_t size, size_t reserved, u32 timeout);
ssize_t cavan_firmware_read_line(struct cavan_firmware *fw, char *buff, size_t size, size_t reserved, u32 timeout);

static inline int cavan_firmware_read_char(struct cavan_firmware *fw, size_t reserved, long timeout)
{
	char c;

	return cavan_firmware_read(fw, &c, 1, reserved, timeout) == 1 ? c : -EFAULT;
}

static inline void *cavan_firmware_get_data(const struct cavan_firmware *fw)
{
	return fw->private_data;
}

static inline void cavan_firmware_set_data(struct cavan_firmware *fw, void *data)
{
	fw->private_data = data;
}
