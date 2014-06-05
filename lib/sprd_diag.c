/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Sun Jan 27 21:46:02 CST 2013
 */

#include <cavan.h>
#include <cavan/math.h>
#include <cavan/memory.h>
#include <cavan/sprd_diag.h>

static u16 const sprd_diag_crc16_table[256] =
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

u16 sprd_diag_crc16(u16 crc, const u8 *buff, size_t size)
{
	const u8 *buff_end;

	for (buff_end = buff + size; buff < buff_end; buff++)
	{
		crc = (crc >> 8) ^ sprd_diag_crc16_table[(crc ^ buff[0]) & 0xFF];
	}

	return crc;
}

void sprd_diag_show_data(const char *prompt, const char *data, size_t size)
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
#if __WORDSIZE == 64
		println("%s[%ld] = 0x%s", prompt, size, buff);
#else
		println("%s[%d] = 0x%s", prompt, size, buff);
#endif
	}
	else
	{
#if __WORDSIZE == 64
		println("%s[%ld] = None", prompt, size);
#else
		println("%s[%d] = None", prompt, size);
#endif
	}
}

char *sprd_diag_imei_tostring(const byte *imei, size_t isize, char *buff, size_t size)
{
	char *buff_end;
	const byte *imei_end;

	for (buff_end = buff + size - 1, imei_end = imei + isize - 1; buff < buff_end && imei < imei_end; buff += 2, imei++)
	{
		buff[0] = value2char((imei[0] >> 4) & 0x0F);
		buff[1] = value2char(imei[1] & 0x0F);
	}

	if (buff < buff_end)
	{
		*buff++ = value2char((imei[0] >> 4) & 0x0F);
	}

	*buff = 0;

	return buff;
}

size_t sprd_diag_text2imei(const char *text, byte *imei, size_t isize)
{
	byte *imei_bak = imei, *imei_end;
	byte lsb = 0x0A;

	for (imei_end = imei + isize; text[0] && text[1] && imei < imei_end; text += 2, imei++)
	{
		*imei = char2value(text[0]) << 4 | lsb;
		lsb = char2value(text[1]);
	}

	if (imei < imei_end && *text)
	{
		*imei++ = char2value(*text) << 4 | lsb;
	}

	return imei - imei_bak;
}

char *sprd_diag_text2mac(const char *text, byte *mac, size_t size)
{
	text = math_text2memory(text, mac, size, 16);
	mem_reverse_simple(mac, mac + size - 1);

	return (char *)text;
}

void sprd_diag_show_imei(const struct sprd_diag_imei_data *imei)
{
	char buff[64];

	sprd_diag_imei_tostring(imei->imei1, sizeof(imei->imei1), buff, sizeof(buff));
	println("IMEI1 = %s", buff);

	sprd_diag_imei_tostring(imei->imei2, sizeof(imei->imei2), buff, sizeof(buff));
	println("IMEI2 = %s", buff);

	sprd_diag_imei_tostring(imei->imei3, sizeof(imei->imei3), buff, sizeof(buff));
	println("IMEI3 = %s", buff);

	sprd_diag_imei_tostring(imei->imei4, sizeof(imei->imei4), buff, sizeof(buff));
	println("IMEI4 = %s", buff);

	sprd_diag_show_data("WIFI-MAC", (char *)imei->wifi_mac, sizeof(imei->wifi_mac));
	sprd_diag_show_data("BT-MAC", (char *)imei->bt_mac, sizeof(imei->bt_mac));
}

char *sprd_diag_encode_data(const char *src, size_t srclen, char *dest, size_t destlen)
{
	const char *src_end = src + srclen;
	char *dest_end = dest + destlen;

	while (src < src_end && dest < dest_end)
	{
		switch (*src)
		{
		case SPRD_DIAG_FLAG_BYTE:
		case SPRD_DIAG_ESCAPE_BYTE:
			*dest++ = SPRD_DIAG_ESCAPE_BYTE;
			if (dest < dest_end)
			{
				*dest++ = *src++ ^ SPRD_DIAG_COMPLEMENT_BYTE;
			}
			break;

		default:
			*dest++ = *src++;
		}
	}

	return dest;
}

size_t sprd_diag_encode_message(const struct sprd_diag_message_desc *message, const char *src, size_t srclen, char *dest, size_t destlen)
{
	char *p, *dest_end = dest + destlen;

	p = dest;
	*p++ = SPRD_DIAG_FLAG_BYTE;
	p = sprd_diag_encode_data((const char *)message, sizeof(*message), p, dest_end - p);
	p = sprd_diag_encode_data(src, srclen, p, dest_end - p);
	*p++ = SPRD_DIAG_FLAG_BYTE;

	return p - dest;
}

char *sprd_diag_decode_data(const char *src, size_t srclen, char *dest, size_t destlen, size_t *reslen)
{
	const char *src_end = src + srclen;
	char *dest_bak = dest, *dest_end = dest + destlen;

	while (src < src_end && dest < dest_end)
	{
		switch (*src)
		{
		case SPRD_DIAG_ESCAPE_BYTE:
			*dest = src[1] ^ SPRD_DIAG_COMPLEMENT_BYTE;
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

ssize_t sprd_diag_read_reply(int fd, struct sprd_diag_command_desc *command)
{
	size_t reslen;
	int found = 0;
	ssize_t rdlen = 0;
	struct sprd_diag_message_desc message;
	char useful[1024], *pos, *pos_end;
	char buff[1024], *p = NULL, *p_end;

	pos = useful;
	pos_end = pos + sizeof(useful);

	while (pos < pos_end && found < 2)
	{
		rdlen = file_read_timeout(fd, buff, sizeof(buff), 500);
		if (rdlen < 0)
		{
			pr_error_info("file_read_timeout");
			return rdlen;
		}

		for (p = buff, p_end = buff + rdlen; p < p_end; p++)
		{
			if (*p == SPRD_DIAG_FLAG_BYTE)
			{
				break;
			}
		}

		if (p < p_end)
		{
			if (found)
			{
				p_end = p;
				p = buff;
			}
			else
label_found_one:
			{
				char *q;

				while (*p == SPRD_DIAG_FLAG_BYTE)
				{
					p++;
				}

				for (q = p; q < p_end; q++)
				{
					if (*q == SPRD_DIAG_FLAG_BYTE)
					{
						p_end = q;
						found++;
						break;
					}
				}
			}

			found++;
		}
		else if (found)
		{
			p = buff;
		}
		else
		{
			continue;
		}

		while (pos < pos_end && p < p_end)
		{
			*pos++ = *p++;
		}
	}

	reslen = pos - useful;
	if (reslen >= sizeof(message) + 2)
	{
		pos_end = pos;
		pos = sprd_diag_decode_data(useful, reslen, (char *)&message, sizeof(message), NULL);

		if (message.type == command->reply_type && message.subtype == command->reply_subtype && message.seq_num == command->seq_num)
		{
			goto label_decode_data;
		}
	}

	if (p == NULL)
	{
		ERROR_RETURN(EINVAL);
	}

	found = 0;
	pos = useful;
	p_end = buff + rdlen;
	goto label_found_one;

label_decode_data:
	sprd_diag_decode_data(pos, pos_end - pos, command->reply, command->reply_len, &reslen);

	rdlen = message.length - sizeof(message);
	if (reslen != (size_t)rdlen)
	{
		pr_red_info("reslen(%d) != rdlen(" PRINT_FORMAT_SIZE ")", reslen, rdlen);
		return -EINVAL;
	}

	sprd_diag_show_data("reply", command->reply, reslen);

	return reslen;
}

int sprd_diag_send_command(int fd, struct sprd_diag_command_desc *command, int retry)
{
	ssize_t rwlen;
	size_t cmdlen;
	static unsigned int cmd_seq_num = 0;
	struct sprd_diag_message_desc message =
	{
		.seq_num = ++cmd_seq_num,
		.length = sizeof(message) + command->command_len,
		.type = command->cmd_type,
		.subtype = command->cmd_subtype
	};
	char buff[sizeof(message) + command->command_len * 2 + 2];

	cmdlen = sprd_diag_encode_message(&message, command->command, command->command_len, buff, sizeof(buff));
	command->seq_num = message.seq_num;

	while (1)
	{
		if (--retry < 0)
		{
			ERROR_RETURN(ETIMEDOUT);
		}

		sprd_diag_show_data("command", buff, cmdlen);

		file_discard_all(fd);

		rwlen = write(fd, buff, cmdlen);
		if (rwlen < 0)
		{
			pr_error_info("write");
			return rwlen;
		}

		fsync(fd);

		if (command->reply == NULL || command->reply_len == 0)
		{
			command->reply_len = 0;
			break;
		}

		rwlen = sprd_diag_read_reply(fd, command);
		if (rwlen < 0 && errno != ETIMEDOUT)
		{
			pr_red_info("sprd_diag_read_reply");
			return rwlen;
		}

		if (rwlen > 0)
		{
			command->reply_len = rwlen;
			break;
		}
	}

	return 0;
}

int sprd_diag_read_imei(int fd, struct sprd_diag_imei_data *imei, u8 mask)
{
	int ret;
	u16 crc;
	u16 data = 0;
	struct sprd_diag_command_desc command =
	{
		.cmd_type = SPRD_DIAG_DIRECT_NV,
		.cmd_subtype = mask | 1 << 7,
		.reply_type = SPRD_DIAG_DIRECT_NV,
		.reply_subtype = SPRD_DIAG_OPER_SUCCESS_FLAG,
		.command = (char *)&data,
		.command_len = sizeof(data),
		.reply = (char *)imei,
		.reply_len = sizeof(*imei)
	};

	while (1)
	{
		ret = sprd_diag_send_command(fd, &command, 10);
		if (ret < 0)
		{
			pr_red_info("sprd_diag_send_command");
			return ret;
		}

		if (command.reply_len != sizeof(*imei))
		{
			continue;
		}

		crc = sprd_diag_crc16(0, (u8 *)imei, sizeof(*imei) - sizeof(imei->crc16));
		println("CRC16 = 0x%04x = 0x%04x", imei->crc16, crc);
		if (crc == imei->crc16)
		{
			break;
		}
	}

	return 0;
}

int sprd_diag_write_imei(int fd, struct sprd_diag_imei_data *imei, u8 mask)
{
	char reply[1024];
	struct sprd_diag_command_desc command =
	{
		.cmd_type = SPRD_DIAG_DIRECT_NV,
		.cmd_subtype = mask,
		.reply_type = SPRD_DIAG_DIRECT_NV,
		.reply_subtype = SPRD_DIAG_OPER_SUCCESS_FLAG,
		.command = (char *)imei,
		.command_len = sizeof(*imei),
		.reply = reply,
		.reply_len = sizeof(reply)
	};

	imei->crc16 = sprd_diag_crc16(0, (u8 *)imei, sizeof(*imei) - sizeof(imei->crc16));

	return sprd_diag_send_command(fd, &command, 10);
}

ssize_t sprd_modem_send_at_command(int fd, char *reply, size_t size, const char *command, ...)
{
	va_list ap;
	ssize_t rwlen;
	char buff[1024];

	va_start(ap, command);
	rwlen = vsnprintf(buff, sizeof(buff), command, ap);
	va_end(ap);

	rwlen = write(fd, buff, rwlen);
	if (rwlen < 0)
	{
		pr_error_info("write");
		return rwlen;
	}

	if (reply == NULL || size == 0)
	{
		return 0;
	}

	rwlen = file_read_timeout(fd, reply, size, 5000);
	if (rwlen < 0)
	{
		pr_error_info("file_read_timeout");
		return rwlen;
	}

	print_ntext(reply, rwlen);

	return rwlen;
}
