/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Sun Jan 27 21:46:02 CST 2013
 */

#include <cavan.h>
#include <cavan/sprd_vbpipe.h>

static u16 const sprd_vbpipe_crc16_table[256] =
{
	0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
	0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
	0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
	0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
	0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
	0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
	0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
	0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
	0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
	0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
	0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
	0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
	0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
	0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
	0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
	0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
	0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
	0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
	0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
	0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
	0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
	0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
	0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
	0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
	0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
	0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
	0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
	0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
	0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
	0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
	0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
	0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

u16 sprd_vbpipe_crc16(u16 crc, const u8 *buff, size_t size)
{
	const u8 *buff_end;

	for (buff_end = buff + size; buff < buff_end; buff++)
	{
		crc = (crc >> 8) ^ sprd_vbpipe_crc16_table[(crc ^ buff[0]) & 0xFF];
	}

	return crc;
}

void sprd_vbpipe_show_data(const char *prompt, const char *data, size_t size)
{
	const char *data_end;
	char buff[size * 2 + 1], *p;

	for (p = buff, data_end = data + size; data < data_end; data++)
	{
		p += sprintf(p, "%02x", *(u8 *)data);
	}

	*p = 0;

	if (prompt == NULL)
	{
		prompt = 0;
	}

	if (size)
	{
		pr_green_info("%s[%d] = 0x%s", prompt, size, buff);
	}
	else
	{
		pr_green_info("%s[%d] = None", prompt, size);
	}
}

void sprd_vbpipe_show_imei(const struct sprd_vbpipe_imei_data *imei)
{
	sprd_vbpipe_show_data("IMEI1", (char *)imei->imei1, sizeof(imei->imei1));
	sprd_vbpipe_show_data("IMEI2", (char *)imei->imei2, sizeof(imei->imei2));
	sprd_vbpipe_show_data("IMEI3", (char *)imei->imei3, sizeof(imei->imei3));
	sprd_vbpipe_show_data("IMEI4", (char *)imei->imei4, sizeof(imei->imei4));
	sprd_vbpipe_show_data("WIFI-MAC", (char *)imei->wifi_mac, sizeof(imei->wifi_mac));
	sprd_vbpipe_show_data("BT-MAC", (char *)imei->bt_mac, sizeof(imei->bt_mac));
}

char *sprd_vbpipe_encode_data(const char *src, size_t srclen, char *dest, size_t destlen)
{
	const char *src_end = src + srclen;
	char *dest_end = dest + destlen;

	while (src < src_end && dest < dest_end)
	{
		switch (*src)
		{
		case SPRD_VBPIPE_FLAG_BYTE:
		case SPRD_VBPIPE_ESCAPE_BYTE:
			*dest++ = SPRD_VBPIPE_ESCAPE_BYTE;
			if (dest < dest_end)
			{
				*dest++ = *src++ ^ SPRD_VBPIPE_COMPLEMENT_BYTE;
			}
			break;

		default:
			*dest++ = *src++;
		}
	}

	return dest;
}

size_t sprd_vbpipe_encode_message(const struct sprd_vbpipe_message_desc *message, const char *src, size_t srclen, char *dest, size_t destlen)
{
	char *p, *dest_end = dest + destlen;

	p = dest;
	*p++ = SPRD_VBPIPE_FLAG_BYTE;
	p = sprd_vbpipe_encode_data((const char *)message, sizeof(*message), p, dest_end - p);
	p = sprd_vbpipe_encode_data(src, srclen, p, dest_end - p);
	*p++ = SPRD_VBPIPE_FLAG_BYTE;

	return p - dest;
}

char *sprd_vbpipe_decode_data(const char *src, size_t srclen, char *dest, size_t destlen, size_t *reslen)
{
	const char *src_end = src + srclen;
	char *dest_bak = dest, *dest_end = dest + destlen;

	while (src < src_end && dest < dest_end)
	{
		switch (*src)
		{
		case SPRD_VBPIPE_ESCAPE_BYTE:
			*dest = src[1] ^ SPRD_VBPIPE_COMPLEMENT_BYTE;
			src += 2;
			break;

		default:
			*dest++ = *src++;
		}
	}

	if (reslen)
	{
		*reslen = dest - dest_bak;
	}

	return (char *)src;
}

ssize_t sprd_vbpipe_read_reply(int fd, struct sprd_vbpipe_command_desc *command)
{
	ssize_t rdlen;
	size_t reslen;
	char c;
	char buff[1024], *p, *p_end;
	struct sprd_vbpipe_message_desc message;

	while (1)
	{
		while (1)
		{
			rdlen = file_read_timeout(fd, &c, 1, 5000);
			if (rdlen < 0)
			{
				pr_error_info("read");
				return rdlen;
			}

			if (c == SPRD_VBPIPE_FLAG_BYTE)
			{
				break;
			}
		}

		p = buff;
		p_end = p + sizeof(buff);

		while (p < p_end)
		{
			rdlen = file_read_timeout(fd, p, 1, 5000);
			if (rdlen < 0)
			{
				return rdlen;
			}

			if (*p != SPRD_VBPIPE_FLAG_BYTE)
			{
				p++;
				continue;
			}

			rdlen = p - buff;
			if (rdlen < (ssize_t)sizeof(message))
			{
				p = buff;
			}
			else
			{
				break;
			}
		}

		p_end = p;
		p = sprd_vbpipe_decode_data(buff, rdlen, (char *)&message, sizeof(message), NULL);
		if (message.type == command->reply_type && message.subtype == command->reply_subtype && message.seq_num == command->seq_num)
		{
			break;
		}
	}

	sprd_vbpipe_decode_data(p, p_end - p, command->reply, command->reply_len, &reslen);

	rdlen = message.length - sizeof(message);
	if (reslen != (size_t)rdlen)
	{
		pr_red_info("reslen(%d) != rdlen(%d)", reslen, rdlen);
		return -EINVAL;
	}

	sprd_vbpipe_show_data("reply", command->reply, reslen);

	return reslen;
}

int sprd_vbpipe_send_command(int fd, struct sprd_vbpipe_command_desc *command)
{
	ssize_t rwlen;
	size_t cmdlen;
	static unsigned int cmd_seq_num = 0;
	struct sprd_vbpipe_message_desc message =
	{
		.seq_num = ++cmd_seq_num,
		.length = sizeof(message) + command->command_len,
		.type = command->cmd_type,
		.subtype = command->cmd_subtype
	};
	char buff[sizeof(message) + command->command_len * 2 + 2];

	cmdlen = sprd_vbpipe_encode_message(&message, command->command, command->command_len, buff, sizeof(buff));
	command->seq_num = message.seq_num;

	while (1)
	{
		sprd_vbpipe_show_data("command", buff, cmdlen);

		rwlen = write(fd, buff, cmdlen);
		if (rwlen < 0)
		{
			pr_error_info("write");
			return rwlen;
		}

		rwlen = sprd_vbpipe_read_reply(fd, command);
		if (rwlen < 0)
		{
			pr_error_info("sprd_vbpipe_read_reply");
			return rwlen;
		}

		if (rwlen > 0)
		{
			break;
		}
	}

	command->reply_len = rwlen;

	return 0;
}

int sprd_vbpipe_read_imei(int fd, struct sprd_vbpipe_imei_data *imei, u8 mask)
{
	int ret;
	u16 crc;
	u16 data = 0;
	struct sprd_vbpipe_command_desc command =
	{
		.cmd_type = SPRD_VBPIPE_DIRECT_NV,
		.cmd_subtype = mask | 1 << 7,
		.reply_type = SPRD_VBPIPE_DIRECT_NV,
		.reply_subtype = SPRD_VBPIPE_OPER_SUCCESS_FLAG,
		.command = (char *)&data,
		.command_len = sizeof(data),
		.reply = (char *)imei,
		.reply_len = sizeof(*imei)
	};

	while (1)
	{
		ret = sprd_vbpipe_send_command(fd, &command);
		if (ret < 0)
		{
			pr_error_info("sprd_vbpipe_send_command");
			return ret;
		}

		if (command.reply_len != sizeof(*imei))
		{
			continue;
		}

		crc = sprd_vbpipe_crc16(0, (u8 *)imei, sizeof(*imei) - sizeof(imei->crc16));
		println("CRC16 = 0x%04x = 0x%04x", imei->crc16, crc);
		if (crc == imei->crc16)
		{
			break;
		}
	}

	return 0;
}

int sprd_vbpipe_write_imei(int fd, struct sprd_vbpipe_imei_data *imei, u8 mask)
{
	char reply[1024];
	struct sprd_vbpipe_command_desc command =
	{
		.cmd_type = SPRD_VBPIPE_DIRECT_NV,
		.cmd_subtype = mask,
		.reply_type = SPRD_VBPIPE_DIRECT_NV,
		.reply_subtype = SPRD_VBPIPE_OPER_SUCCESS_FLAG,
		.command = (char *)imei,
		.command_len = sizeof(*imei),
		.reply = reply,
		.reply_len = sizeof(reply)
	};

	imei->crc16 = sprd_vbpipe_crc16(0, (u8 *)imei, sizeof(*imei) - sizeof(imei->crc16));

	return sprd_vbpipe_send_command(fd, &command);
}
