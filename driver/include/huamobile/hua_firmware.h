#pragma once

#include <linux/kernel.h>

#define HUA_FW_FLAG_USER	(1 << 0)

struct hua_firmware
{
	int closed;
	struct mutex lock;
	void *private_data;
	struct task_struct *rdtask;
	struct task_struct *wrtask;
	int status;
	char *head, *tail;
	char *mem, *mem_end;

	int (*upgrade)(struct hua_firmware *fw);
};

struct hua_firmware *hua_firmware_create(size_t size, int (*upgrade)(struct hua_firmware *));
int hua_firmware_destroy(struct hua_firmware *fw);
ssize_t hua_firmware_free_space(struct hua_firmware *fw);
ssize_t hua_firmware_used_space(struct hua_firmware *fw);
ssize_t hua_firmware_write(struct hua_firmware *fw, const char *buff, size_t size, int flags);
ssize_t hua_firmware_read(struct hua_firmware *fw, char *buff, size_t size, size_t reserved, u32 timeout);
ssize_t hua_firmware_fill(struct hua_firmware *fw, char *buff, size_t size, size_t reserved, u32 timeout);
ssize_t hua_firmware_read_line(struct hua_firmware *fw, char *buff, size_t size, size_t reserved, u32 timeout);

static inline int hua_firmware_read_char(struct hua_firmware *fw, size_t reserved, long timeout)
{
	char c;

	return hua_firmware_read(fw, &c, 1, reserved, timeout) == 1 ? c : -EFAULT;
}

static inline void *hua_firmware_get_data(const struct hua_firmware *fw)
{
	return fw->private_data;
}

static inline void hua_firmware_set_data(struct hua_firmware *fw, void *data)
{
	fw->private_data = data;
}
