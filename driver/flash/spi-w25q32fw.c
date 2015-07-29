/* drivers/spi/spi-w25q32fw.c - spi flash driver
 * author: cavan.cfa@gmail.com
 * create date: 2015-05-25
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/fs.h>
#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/spi/spi.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/miscdevice.h>
#include <linux/platform_data/spi-rockchip.h>
#include <asm/uaccess.h>
#include "spi-rockchip-core.h"

#define W25Q32FW_DEBUG				0
#define W25Q32FW_DUMP				0

#define W25Q32FW_RD_MAX				256
#define W25Q32FW_WR_MAX				256
#define W25Q32FW_SPEED_HZ			(2 * 1000 * 1000)

#define w25q32fw_pr_info(fmt, args ...) \
	pr_err("%s[%d]: " fmt "\n", __FUNCTION__, __LINE__, ##args)

#define w25q32fw_pr_pos_info() \
	pr_err("%s => %s[%d]\n", __FILE__, __FUNCTION__, __LINE__)

typedef enum {
	W25Q32FW_ERASE_SECTOR,
	W25Q32FW_ERASE_BLOCK_32K,
	W25Q32FW_ERASE_BLOCK_64K,
	W25Q32FW_ERASE_CHIP,
} w25q32fw_erase_type_t;

struct w25q32fw_device {
	struct spi_device *spi;
	struct dw_spi_chip chip_data;

	char misc_name[16];
	struct miscdevice misc;

	int gpio_rst;
	int gpio_pwr;
	bool enabled;
	u32 capacity;
};

struct w25q32fw_command {
	const void *command;
	size_t cmdlen;

	const void *data;
	size_t datalen;

	void *response;
	size_t rsplen;
};

// ================================================================================

static int w25q32fw_spi_send_command(struct spi_device *spi, struct w25q32fw_command *command)
{
	struct spi_message message;
	struct spi_transfer transfer[3];

#if W25Q32FW_DEBUG
	w25q32fw_pr_info("command = %p, cmdlen = %d", command->command, command->cmdlen);
	w25q32fw_pr_info("data = %p, datalen = %d", command->data, command->datalen);
	w25q32fw_pr_info("response = %p, rsplen = %d", command->response, command->rsplen);
#endif

	if (command->command == NULL || command->cmdlen == 0) {
		dev_err(&spi->dev, "Invalid command!");
		return -EINVAL;
	}

#if W25Q32FW_DEBUG
	w25q32fw_pr_info("command = 0x%02x", *(u8 *) command->command);
#endif

	spi_message_init(&message);
	memset(transfer, 0x00, sizeof(transfer));

	transfer[0].speed_hz = W25Q32FW_SPEED_HZ;
	transfer[0].len = command->cmdlen;
	transfer[0].tx_buf = command->command;
	spi_message_add_tail(transfer, &message);

	if (command->data && command->datalen > 0) {
		transfer[1].speed_hz = W25Q32FW_SPEED_HZ;
		transfer[1].len = command->datalen;
		transfer[1].tx_buf = command->data;
		spi_message_add_tail(transfer + 1, &message);
	}

	if (command->response && command->rsplen > 0) {
		memset(command->response, 0xFF, command->rsplen);
		transfer[2].speed_hz = W25Q32FW_SPEED_HZ;
		transfer[2].len = command->rsplen;
		transfer[2].tx_buf = transfer[2].rx_buf = command->response;
		spi_message_add_tail(transfer + 2, &message);
	}

	return spi_sync(spi, &message);
}

static int w25q32fw_read_id(struct w25q32fw_device *w25q32fw)
{
	u16 id;
	int ret;
	u8 capacity;
	struct spi_device *spi = w25q32fw->spi;
	struct w25q32fw_command command;
	u8 cmd_buff[] = { 0x90, 0x00, 0x00, 0x00 };

	w25q32fw_pr_pos_info();

	command.command = cmd_buff;
	command.cmdlen = sizeof(cmd_buff);
	command.data = NULL;
	command.datalen = 0;
	command.response = &id;
	command.rsplen = sizeof(id);

	ret = w25q32fw_spi_send_command(spi, &command);
	if (ret < 0) {
		dev_err(&spi->dev, "Failed to w25q32fw_spi_send_command: %d\n", ret);
		return ret;
	}

	dev_info(&spi->dev, "DEVICE ID = 0x%04x\n", id);

	capacity = ((id >> 12) & 0x0F) * 10 + ((id >> 8) & 0x0F);
	if (capacity < 3) {
		dev_err(&spi->dev, "DEVICE ID Invalid!");
		return -EINVAL;
	}

	w25q32fw->capacity = 1 << (capacity + 7);
	dev_info(&spi->dev, "Capacity = %d\n", w25q32fw->capacity);

	return 0;
}

static int w25q32fw_read_data_one(struct spi_device *spi, u32 offset, void *buff, size_t size)
{
	int ret;
	struct w25q32fw_command command;
	u8 cmd_buff[] = { 0x03, (offset >> 16) & 0xFF, (offset >> 8) & 0xFF, offset & 0xFF };

#if W25Q32FW_DEBUG
	w25q32fw_pr_info("offset = %d, size = %d", offset, size);
#endif

	command.command = cmd_buff;
	command.cmdlen = sizeof(cmd_buff);
	command.data = NULL;
	command.datalen = 0;
	command.response = buff;
	command.rsplen = size;

	ret = w25q32fw_spi_send_command(spi, &command);
	if (ret < 0) {
		dev_err(&spi->dev, "Failed to w25q32fw_spi_send_command: %d\n", ret);
		return ret;
	}

	return 0;
}

static ssize_t w25q32fw_read_data(struct w25q32fw_device *w25q32fw, u32 offset, char __user *data, size_t size)
{
	size_t size_bak;
	struct spi_device *spi = w25q32fw->spi;

	w25q32fw_pr_info("offset = %d, size = %d", offset, size);

	if (offset + size > w25q32fw->capacity) {
		if (offset < w25q32fw->capacity) {
			size = w25q32fw->capacity - offset;
		} else {
			size = 0;
		}

		w25q32fw_pr_info("real: offset = %d, size = %d", offset, size);
	}

	size_bak = size;

	while (size > 0) {
		int ret;
		size_t rdlen;
		char buff[W25Q32FW_RD_MAX];

		rdlen = size > W25Q32FW_RD_MAX ? W25Q32FW_RD_MAX : size;
		ret = w25q32fw_read_data_one(spi, offset, buff, rdlen);
		if (ret < 0) {
			dev_err(&spi->dev, "Failed to w25q32fw_read_data_one: %d\n", ret);
			return ret;
		}

		if (copy_to_user(data, buff, rdlen)) {
			dev_err(&spi->dev, "Failed to copy_to_user\n");
			return -EFAULT;
		}

		data += rdlen;
		size -= rdlen;
		offset += rdlen;
	}

	return size_bak;
}

static int w25q32fw_read_status(struct spi_device *spi, u8 *status)
{
	int ret;
	struct w25q32fw_command command;
	u8 cmd_data = 0x05;

	command.command = &cmd_data;
	command.cmdlen = 1;
	command.data = NULL;
	command.datalen = 0;
	command.response = status;
	command.rsplen = 1;

	ret = w25q32fw_spi_send_command(spi, &command);
	if (ret < 0) {
		dev_err(&spi->dev, "Failed to w25q32fw_spi_send_command: %d\n", ret);
		return ret;
	}

	return 0;
}

static int w25q32fw_wait_ready(struct spi_device *spi)
{
	while (1) {
		int ret;
		u8 status = 0;

		ret = w25q32fw_read_status(spi, &status);
		if (ret < 0) {
			dev_warn(&spi->dev, "Failed to w25q32fw_read_status: %d\n", ret);
		} else {
#if W25Q32FW_DEBUG
			dev_info(&spi->dev, "status = 0x%02x\n", status);
#endif

			if ((status & 1) == 0) {
				break;
			}
		}

		udelay(100);
	}

	return 0;
}

static int w25q32fw_set_write_enable(struct spi_device *spi, bool enable)
{
	int ret;
	struct w25q32fw_command command;
	u8 cmd_data = enable ? 0x06 : 0x04;

	command.command = &cmd_data;
	command.cmdlen = 1;
	command.data = NULL;
	command.datalen = 0;
	command.response = NULL;
	command.rsplen = 0;

	ret = w25q32fw_spi_send_command(spi, &command);
	if (ret < 0) {
		dev_err(&spi->dev, "Failed to w25q32fw_spi_send_command: %d\n", ret);
		return ret;
	}

	return 0;
}

static int w25q32fw_write_data_one(struct spi_device *spi, u32 offset, void *buff, size_t size)
{
	int ret;
	struct w25q32fw_command command;
	u8 cmd_buff[] = { 0x02, (offset >> 16) & 0xFF, (offset >> 8) & 0xFF, offset & 0xFF };

#if W25Q32FW_DEBUG
	w25q32fw_pr_info("offset = %d, size = %d", offset, size);
#endif

	ret = w25q32fw_set_write_enable(spi, true);
	if (ret < 0) {
		dev_err(&spi->dev, "Failed to w25q32fw_set_write_enable: %d\n", ret);
		return ret;
	}

	command.command = cmd_buff;
	command.cmdlen = sizeof(cmd_buff);
	command.data = buff;
	command.datalen = size;
	command.response = NULL;
	command.rsplen = 0;

	ret = w25q32fw_spi_send_command(spi, &command);
	if (ret < 0) {
		dev_err(&spi->dev, "Failed to w25q32fw_spi_send_command: %d\n", ret);
		return ret;
	}

	w25q32fw_wait_ready(spi);

	return 0;
}

static ssize_t w25q32fw_write_data(struct w25q32fw_device *w25q32fw, u32 offset, const char __user *data, size_t size)
{
	size_t size_bak;
	struct spi_device *spi = w25q32fw->spi;

	w25q32fw_pr_info("offset = %d, size = %d", offset, size);

	if (offset + size > w25q32fw->capacity) {
		if (offset < w25q32fw->capacity) {
			size = w25q32fw->capacity - offset;
		} else {
			dev_err(&spi->dev, "No space left!");
			return -ENOSPC;
		}

		w25q32fw_pr_info("real: offset = %d, size = %d", offset, size);
	}

	size_bak = size;

	while (size > 0) {
		int ret;
		u32 wrlen;
		char buff[W25Q32FW_WR_MAX];

		wrlen = (offset / 256 + 1) * 256 - offset;
		if (wrlen > size) {
			wrlen = size;
		}

		if (wrlen > W25Q32FW_WR_MAX) {
			wrlen = W25Q32FW_WR_MAX;
		}

		if (copy_from_user(buff, data, wrlen)) {
			dev_err(&spi->dev, "Failed to copy_from_user\n");
			return -EFAULT;
		}

		ret = w25q32fw_write_data_one(spi, offset, buff, wrlen);
		if (ret < 0) {
			dev_err(&spi->dev, "Failed to w25q32fw_write_data_one: %d\n", ret);
			return ret;
		}

		data += wrlen;
		size -= wrlen;
		offset += wrlen;
	}

	return size_bak;
}

static int w25q32fw_erase(struct spi_device *spi, u32 offset, w25q32fw_erase_type_t type)
{
	int ret;
	struct w25q32fw_command command;
	u8 cmd_buff[] = { 0x20, (offset >> 16) & 0xFF, (offset >> 8) & 0xFF, offset & 0xFF };

	w25q32fw_pr_info("offset = %d, type = %d", offset, type);

	command.command = cmd_buff;
	command.cmdlen = sizeof(cmd_buff);
	command.data = NULL;
	command.datalen = 0;
	command.response = NULL;
	command.rsplen = 0;

	switch (type) {
	case W25Q32FW_ERASE_SECTOR:
		cmd_buff[0] = 0x20;
		break;

	case W25Q32FW_ERASE_BLOCK_32K:
		cmd_buff[0] = 0x52;
		break;

	case W25Q32FW_ERASE_BLOCK_64K:
		cmd_buff[0] = 0xD8;
		break;

	case W25Q32FW_ERASE_CHIP:
		cmd_buff[0] = 0x60; // 0xC7
		command.cmdlen = 1;
		break;

	default:
		dev_err(&spi->dev, "Invalid erase type %d", type);
		return -EINVAL;
	}

	ret = w25q32fw_set_write_enable(spi, true);
	if (ret < 0) {
		dev_err(&spi->dev, "Failed to w25q32fw_set_write_enable: %d\n", ret);
		return ret;
	}

	ret = w25q32fw_spi_send_command(spi, &command);
	if (ret < 0) {
		dev_err(&spi->dev, "Failed to w25q32fw_spi_send_command: %d\n", ret);
		return ret;
	}

	w25q32fw_wait_ready(spi);

	return 0;
}

static int w25q32fw_set_enable(struct w25q32fw_device *w25q32fw, bool enable)
{
	int ret = 0;
	struct spi_device *spi = w25q32fw->spi;

	if (enable == w25q32fw->enabled) {
		dev_info(&spi->dev, "Nothing to be done %s\n", __FUNCTION__);
		return 0;
	}

	if (enable) {
		if (gpio_is_valid(w25q32fw->gpio_pwr)) {
			gpio_direction_output(w25q32fw->gpio_pwr, 1);
			gpio_set_value(w25q32fw->gpio_pwr, 1);
			msleep(100);
		}

		if (gpio_is_valid(w25q32fw->gpio_rst)) {
			gpio_direction_output(w25q32fw->gpio_rst, 0);
			gpio_set_value(w25q32fw->gpio_rst, 0);
			msleep(100);
		}

		ret = w25q32fw_read_id(w25q32fw);
		if (ret < 0) {
			dev_err(&spi->dev, "Failed to w25q32fw_read_id: %d\n", ret);
			enable = false;
		}
	}

	if (enable == false) {
		if (gpio_is_valid(w25q32fw->gpio_rst)) {
			gpio_direction_output(w25q32fw->gpio_rst, 1);
			gpio_set_value(w25q32fw->gpio_rst, 1);
		}
	}

	w25q32fw->enabled = enable;

	return ret;
}

// ================================================================================

static ssize_t w25q32fw_misc_write(struct file *file,  const char __user *buff, size_t size, loff_t *offset)
{
	ssize_t wrlen;
	struct w25q32fw_device *w25q32fw = file->private_data;
	struct spi_device *spi = w25q32fw->spi;

	wrlen = w25q32fw_write_data(w25q32fw, *offset, buff, size);
	if (wrlen < 0) {
		dev_err(&spi->dev, "Failed to w25q32fw_write_data: %d\n", wrlen);
		return wrlen;
	}

	*offset += wrlen;

	return wrlen;
}

static ssize_t w25q32fw_misc_read(struct file *file, char __user *buff, size_t size, loff_t *offset)
{
	ssize_t rdlen;
	struct w25q32fw_device *w25q32fw = file->private_data;
	struct spi_device *spi = w25q32fw->spi;

	rdlen = w25q32fw_read_data(w25q32fw, *offset, buff, size);
	if (rdlen < 0) {
		dev_err(&spi->dev, "Failed to w25q32fw_read_data: %d\n", rdlen);
		return rdlen;
	}

	*offset += rdlen;

	return rdlen;
}

static int w25q32fw_misc_open(struct inode *inode, struct file *file)
{
	int ret;
	struct miscdevice *misc = file->private_data;
	struct w25q32fw_device *w25q32fw = container_of(misc, struct w25q32fw_device, misc);
	struct spi_device *spi = w25q32fw->spi;

	w25q32fw_pr_pos_info();

	dev_info(&spi->dev, "gpio_rst = %d\n", w25q32fw->gpio_rst);

	ret = w25q32fw_set_enable(w25q32fw, true);
	if (ret < 0) {
		dev_err(&spi->dev, "Failed to w25q32fw_set_enable: %d\n", ret);
		return ret;
	}

	if ((file->f_flags & O_ACCMODE) == O_WRONLY) {
		ret = w25q32fw_erase(spi, 0, W25Q32FW_ERASE_CHIP);
		if (ret < 0) {
			dev_err(&spi->dev, "Failed to w25q32fw_erase: %d\n", ret);
			goto out_w25q32fw_disable;
		}
	}

	file->private_data = w25q32fw;

	return 0;

out_w25q32fw_disable:
	w25q32fw_set_enable(w25q32fw, false);
	return ret;
}

static int w25q32fw_misc_release(struct inode *inode, struct file *file)
{
	struct w25q32fw_device *w25q32fw = file->private_data;

	w25q32fw_pr_pos_info();

	w25q32fw_set_enable(w25q32fw, false);

	return 0;
}

static const struct file_operations w25q32fw_spi_fops = {
	.open = w25q32fw_misc_open,
	.release = w25q32fw_misc_release,
	.write = w25q32fw_misc_write,
	.read = w25q32fw_misc_read,
};

// ================================================================================

static int w25q32fw_spi_parse_dt(struct w25q32fw_device *w25q32fw)
{
	u32 value;
	struct spi_device *spi = w25q32fw->spi;
	struct device_node *np = spi->dev.of_node;
	struct dw_spi_chip *chip_data = &w25q32fw->chip_data;

	if (of_property_read_u32(np, "poll_mode", &value) < 0) {
		dev_warn(&spi->dev, "Failed to get poll_mode, default set 0\n");
		chip_data->poll_mode = 0;
	} else {
		chip_data->poll_mode = value;
	}

	if (of_property_read_u32(np, "type", &value)) {
		dev_warn(&spi->dev, "Failed to get type, default set 0\n");
		chip_data->type = 0;
	} else {
		chip_data->type = value;
	}

	if (of_property_read_u32(np, "enable_dma", &value)) {
		dev_warn(&spi->dev, "Failed to get enable_dma, default set 0\n");
		chip_data->enable_dma = 0;
	} else {
		chip_data->enable_dma = value;
	}

	dev_info(&spi->dev, "poll_mode = %d\n", chip_data->poll_mode);
	dev_info(&spi->dev, "type = %d\n", chip_data->type);
	dev_info(&spi->dev, "enable_dma = %d\n", chip_data->enable_dma);

	w25q32fw->gpio_rst = of_get_gpio(np, 0);
	if (gpio_is_valid(w25q32fw->gpio_rst)) {
		if (gpio_request(w25q32fw->gpio_rst, "W25Q32FW-RST") < 0) {
			dev_err(&spi->dev, "Failed to gpio_request %d", w25q32fw->gpio_rst);
		}
	}

	w25q32fw->gpio_pwr = of_get_gpio(np, 1);
	if (gpio_is_valid(w25q32fw->gpio_pwr)) {
		if (gpio_request(w25q32fw->gpio_pwr, "W25Q32FW-PWR") < 0) {
			dev_err(&spi->dev, "Failed to gpio_request %d", w25q32fw->gpio_pwr);
		}

		gpio_direction_output(w25q32fw->gpio_pwr, 1);
		gpio_set_value(w25q32fw->gpio_pwr, 1);
	}

	dev_info(&spi->dev, "gpio_rst = %d\n", w25q32fw->gpio_rst);
	dev_info(&spi->dev, "gpio_pwr = %d\n", w25q32fw->gpio_pwr);

	return 0;
}

#if W25Q32FW_DUMP
static int w25q32fw_dump_thread(void *data)
{
	struct w25q32fw_device *w25q32fw = data;

	while (w25q32fw_set_enable(w25q32fw, true) < 0) {
		msleep(200);
	}

	w25q32fw_set_enable(w25q32fw, false);

	return 0;
}
#endif

static int w25q32fw_spi_probe(struct spi_device *spi)
{
	int ret;
	struct miscdevice *misc;
	struct w25q32fw_device *w25q32fw;

	w25q32fw = (struct w25q32fw_device *) devm_kzalloc(&spi->dev, sizeof(struct w25q32fw_device), GFP_KERNEL);
	if(w25q32fw == NULL){
		dev_err(&spi->dev, "Faile to devm_kzalloc\n");
		return -ENOMEM;
	}

	w25q32fw->spi = spi;
	spi_set_drvdata(spi, w25q32fw);

	ret = w25q32fw_spi_parse_dt(w25q32fw);
	if (ret < 0) {
		dev_err(&spi->dev, "Failed to w25q32fw_spi_parse_dt: %d\n", ret);
		goto out_devm_kfree;
	}

	spi->bits_per_word = 8;
	spi->controller_data = &w25q32fw->chip_data;

	ret = spi_setup(spi);
	if (ret < 0){
		dev_err(&spi->dev, "Failed to spi_setup: %d\n", ret);
		goto out_devm_kfree;
	}

	snprintf(w25q32fw->misc_name, sizeof(w25q32fw->misc_name), "W25Q32FW%d-%d", spi->master->bus_num, spi->chip_select);
	misc = &w25q32fw->misc;
	misc->minor = MISC_DYNAMIC_MINOR;
	misc->name = w25q32fw->misc_name;
	misc->fops = &w25q32fw_spi_fops;

	ret = misc_register(misc);
	if (ret < 0) {
		dev_err(&spi->dev, "Failed to misc_register: %d\n", ret);
		goto out_devm_kfree;
	}

#if W25Q32FW_DUMP
	kthread_run(w25q32fw_dump_thread, w25q32fw, "W25Q32FW-Dump");
#endif

	return 0;

out_devm_kfree:
	devm_kfree(&spi->dev, w25q32fw);
	return ret;
}

static int w25q32fw_spi_remove(struct spi_device *spi)
{
	struct w25q32fw_device *w25q32fw = spi_get_drvdata(spi);

	w25q32fw_pr_pos_info();

	misc_deregister(&w25q32fw->misc);
	devm_kfree(&spi->dev, w25q32fw);

	return 0;
}

static struct spi_driver w25q32fw_spi_driver = {
	.driver = {
		.name	= "spi-w25q32fw",
		.owner = THIS_MODULE,
	},
	.probe = w25q32fw_spi_probe,
	.remove = w25q32fw_spi_remove,
};

static int __init w25q32fw_spi_init(void)
{
	return spi_register_driver(&w25q32fw_spi_driver);
}

static void __exit w25q32fw_spi_exit(void)
{
	spi_unregister_driver(&w25q32fw_spi_driver);
}

module_init(w25q32fw_spi_init);
module_exit(w25q32fw_spi_exit);

MODULE_DESCRIPTION("W25Q32FW SPI Flash Driver");
MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_LICENSE("GPL");
