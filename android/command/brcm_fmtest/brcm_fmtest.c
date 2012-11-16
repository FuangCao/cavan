#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <poll.h>
#include <btl_if.h>

#define BRCM_FM_SCAN_DIRECTION_UP		0x80
#define BRCM_FM_SCAN_DIRECTION_DOWN		0x00
#define BRCM_FM_SCAN_MIN_SIGNAL			105

#define pr_std_info(fmt, args ...) \
	printf(fmt "\n", ##args)

#define pr_pos_info() \
	pr_std_info("%s => %s[%d]", __FILE__, __FUNCTION__, __LINE__)

#define pr_red_info(fmt, args ...) \
	pr_std_info("\033[31m" fmt "\033[0m", ##args)

#define pr_green_info(fmt, args ...) \
	pr_std_info("\033[32m" fmt "\033[0m", ##args)

#define pr_bold_info(fmt, args ...) \
	pr_std_info("\033[1m" fmt "\033[0m", ##args)

#define pr_error_info(fmt, args ...) \
	if (errno) { \
		pr_red_info("%s[%d](" fmt "): %s", __FUNCTION__, __LINE__, ##args, strerror(errno)); \
	} else { \
		pr_red_info("%s[%d]:" fmt, __FUNCTION__, __LINE__, ##args); \
	}

struct brcm_fm_device
{
	tCTRL_HANDLE ctrl_handle;
	int pipefd[2];
};

extern tBTL_IF_Result BTL_IFC_RegisterSubSystem(tCTRL_HANDLE *handle, tBTL_IF_SUBSYSTEM sub, tBTL_IF_DATA_CALLBACK data_cb, tBTL_IF_CTRL_CALLBACK ctrl_cb);
extern tBTL_IF_Result BTL_IFC_CtrlSend(int ctrl_fd, tSUB sub, tBTLIF_CTRL_MSG_ID msg_id, tBTL_PARAMS *params, int param_len);

extern int fm_enable();
extern int fm_disable();

static struct brcm_fm_device fm_dev;

static void brcm_fm_audio_enable(int enable)
{
	if(enable)
	{
		system("alsa_amixer cset -c sprdphone name=\"BypassFM Playback Switch\" 1");
		system("alsa_amixer cset -c sprdphone name=\"Headset Playback Switch\" 1");
		system("alsa_amixer sset 'LineinFM' on");
		system("alsa_amixer -c sprdphone cset name='Power Codec' 1");
	}
	else
	{
		system("alsa_amixer -c sprdphone cset name='Power Codec' 4");
		system("alsa_amixer cset -c sprdphone name=\"BypassFM Playback Switch\" 0");
		system("alsa_amixer cset -c sprdphone name=\"Speaker Playback Switch\" 0");
		system("alsa_amixer sset 'LineinFM' off");
	}
}

static char *text_copy(char *dest, const char *src)
{
	while ((*dest++ = *src++));

	return dest;
}

static int brcm_fm_wait_command_response(struct brcm_fm_device *dev, tBTLIF_CTRL_MSG_ID id, tBTL_PARAMS *params, int timeout_second)
{
	int ret;
	ssize_t rdlen;
	int fd = dev->pipefd[0];
	tBTLIF_CTRL_MSG_ID msg_id;
	struct pollfd pfd =
	{
		.fd = fd,
		.events = POLLIN,
		.revents = 0
	};

	pr_bold_info("ID = %d, Timeout = %d(s)", id, timeout_second);

	while (1)
	{
		ret = poll(&pfd, 1, timeout_second * 1000);
		if (ret < 0)
		{
			pr_error_info("poll");
			return ret;
		}

		if (pfd.revents == 0)
		{
			pr_red_info("poll timeout");
			return -ETIMEDOUT;
		}

		rdlen = read(fd, &msg_id, sizeof(msg_id));
		if (rdlen < 0)
		{
			pr_red_info("rdlen");
			return rdlen;
		}

		rdlen = read(fd, params, sizeof(*params));
		if (rdlen < 0)
		{
			pr_red_info("read");
			return rdlen;
		}

		pr_bold_info("Response ID = %d, ID = %d", msg_id, id);

		if (msg_id == id)
		{
			break;
		}
	}

	return 0;
}

static int brcm_fm_send_ctrl_command(struct brcm_fm_device *dev, int command, tBTL_PARAMS *params, int timeout, const char *format, ...)
{
	int ret;
	va_list ap;
	char *p = (char *)params;

	pr_bold_info("Command = %d, Format = %s", command, format);

	if (format)
	{
		va_start(ap, format);

		while (1)
		{
			switch (*format++)
			{
			case 0:
				goto label_va_end;

			case 'i':
			case 'I':
			case 'd':
			case 'D':
				*(int *)p = va_arg(ap, int);
				p += sizeof(int);
				break;

			case 'w':
			case 'W':
				*(UINT16 *)p = va_arg(ap, int);
				p += sizeof(UINT16);
				break;

			case 'b':
			case 'B':
			case 'z':
			case 'Z':
				*(BOOLEAN *)p = va_arg(ap, int);
				p += sizeof(BOOLEAN);
				break;

			case 't':
			case 'T':
			case 's':
			case 'S':
				p = text_copy(p, va_arg(ap, char *));
				goto label_va_end;

			default:
				pr_red_info("unknown format %c", *p);
			}
		}

label_va_end:
		va_end(ap);
	}

	ret = BTL_IFC_CtrlSend(dev->ctrl_handle, SUB_FM, command, params, p - (char *)params);
	if (ret != BTL_IF_SUCCESS)
	{
		pr_red_info("BTL_IFC_CtrlSend command = %d", command);
		return -EFAULT;
	}

	switch (command)
	{
	case BTLIF_FM_SET_VOLUME:
		command = BTLIF_FM_SET_VOLUME_EVT;
		break;

	case BTLIF_FM_SEARCH:
		command = BTLIF_FM_SEARCH_CMPL_EVT;
		break;
	}

	return brcm_fm_wait_command_response(dev, command, params, timeout);
}

static int brcm_fm_ctrl_enable(struct brcm_fm_device *dev, int func_mask)
{
	int ret;
	tBTL_PARAMS params;

	ret = brcm_fm_send_ctrl_command(dev, BTLIF_FM_ENABLE, &params, 2, "I", func_mask);
	if (ret < 0)
	{
		pr_red_info("brcm_fm_send_ctrl_command BTLIF_FM_TUNE");
		return ret;
	}

	return params.fm_I_param.i1;
}

static int brcm_fm_ctrl_disable(struct brcm_fm_device *dev)
{
	int ret;
	tBTL_PARAMS params;

	ret = brcm_fm_send_ctrl_command(dev, BTLIF_FM_DISABLE, &params, 2, NULL);
	if (ret < 0)
	{
		pr_red_info("brcm_fm_send_ctrl_command BTLIF_FM_TUNE");
		return ret;
	}

	return params.fm_I_param.i1;
}

static int brcm_fm_ctrl_tune_radio(struct brcm_fm_device *dev, int freq)
{
	int ret;
	tBTL_PARAMS params;

	ret = brcm_fm_send_ctrl_command(dev, BTLIF_FM_TUNE, &params, 2, "I", freq);
	if (ret < 0)
	{
		pr_red_info("brcm_fm_send_ctrl_command BTLIF_FM_TUNE");
		return ret;
	}

	return params.fm_IIII_param.i1;
}

static int brcm_fm_ctrl_set_volume(struct brcm_fm_device *dev, int volume)
{
	int ret;
	tBTL_PARAMS params;

	ret = brcm_fm_send_ctrl_command(dev, BTLIF_FM_SET_VOLUME, &params, 2, "I", volume);
	if (ret < 0)
	{
		pr_red_info("brcm_fm_send_ctrl_command BTLIF_FM_SET_VOLUME");
		return ret;
	}

	return params.fm_II_param.i1;
}

static int brcm_fm_ctrl_seek_station(struct brcm_fm_device *dev, int direction, int *freq, int *rssi)
{
	int ret;
	tBTL_PARAMS params;

	ret = brcm_fm_send_ctrl_command(dev, BTLIF_FM_SEARCH, &params, 20, "IIII", direction, BRCM_FM_SCAN_MIN_SIGNAL, 0, 0);
	if (ret < 0)
	{
		pr_red_info("brcm_fm_send_ctrl_command");
		return ret;
	}

	*rssi = params.fm_IIII_param.i2;
	*freq = params.fm_IIII_param.i3;

	return params.fm_IIII_param.i1;
}

static int brcm_fm_ctrl_seek_station_combo(struct brcm_fm_device *dev, int start_freq, int end_freq, int direction)
{
	return 0;
}

static void brcm_fm_data_handler(tDATA_HANDLE handle, char *p, int len)
{
	pr_pos_info();
}

static void brcm_fm_ctrl_handler(tCTRL_HANDLE handle, tBTLIF_CTRL_MSG_ID id, tBTL_PARAMS *params)
{
	switch (id)
	{
	case BTLIF_FM_ENABLE:
		pr_bold_info("BTLIF_FM_ENABLE: status = %d", params->fm_I_param.i1);
		break;

	case BTLIF_FM_DISABLE:
		pr_bold_info("BTLIF_FM_DISABLE: status = %d", params->fm_I_param.i1);
		break;

	case BTLIF_FM_TUNE:
		pr_bold_info("BTLIF_FM_TUNE: status = %d, rssi = %d, freq = %d, snr = %d ", \
			params->fm_IIII_param.i1, params->fm_IIII_param.i2, params->fm_IIII_param.i3, params->fm_IIII_param.i4);
		break;

	case BTLIF_FM_MUTE:
		pr_bold_info("BTLIF_FM_MUTE: status = %d, is_mute = %d", \
			params->fm_IZ_param.i1, params->fm_IZ_param.z1);
		break;

	case BTLIF_FM_SEARCH:
		pr_bold_info("BTLIF_FM_SEARCH: rssi = %d, freq = %d, snr = %d", \
			params->fm_III_param.i1, params->fm_III_param.i2, params->fm_III_param.i3);
		break;

	case BTLIF_FM_SEARCH_CMPL_EVT:
		pr_bold_info("BTLIF_FM_SEARCH_CMPL_EVT: status = %d, rssi = %d, freq = %d, snr = %d ", \
			params->fm_IIII_param.i1, params->fm_IIII_param.i2, params->fm_IIII_param.i3, params->fm_IIII_param.i4);
		break;

	case BTLIF_FM_SEARCH_ABORT:
		pr_bold_info("BTLIF_FM_SEARCH_ABORT: status = %d, rssi = %d, freq = %d", \
			params->fm_III_param.i1, params->fm_III_param.i2, params->fm_III_param.i3);
		break;

	case BTLIF_FM_SET_RDS_MODE:
		pr_bold_info("BTLIF_FM_SET_RDS_MODE: status = %d, rds_on = %d, af_on = %d", \
			params->fm_IZZ_param.i1, params->fm_IZZ_param.z1, params->fm_IZZ_param.z2);
		break;

	case BTLIF_FM_SET_RDS_RBDS:
		pr_bold_info("BTLIF_FM_SET_RDS_RBDS: status = %d, type = %d", \
			params->fm_II_param.i1, params->fm_II_param.i2);
		break;

	case BTLIF_FM_RDS_UPDATE:
		pr_bold_info("BTLIF_FM_RDS_UPDATE: status = %d, data = %d, index = %d, text = %s", \
			params->fm_IIIS_param.i1, params->fm_IIIS_param.i2, params->fm_IIIS_param.i3, params->fm_IIIS_param.s1);
		break;

	case BTLIF_FM_AUDIO_MODE:
		pr_bold_info("BTLIF_FM_AUDIO_MODE: status = %d, audio_mode = %d", \
			params->fm_II_param.i1, params->fm_II_param.i2);
		break;

	case BTLIF_FM_AUDIO_PATH:
		pr_bold_info("BTLIF_FM_AUDIO_PATH: status = %d, audio_path = %d", \
			params->fm_II_param.i1, params->fm_II_param.i2);
		break;

	case BTLIF_FM_SCAN_STEP:
		pr_bold_info("BTLIF_FM_SCAN_STEP: scan_step = %d", params->fm_I_param.i1);
		break;

	case BTLIF_FM_SET_REGION:
		pr_bold_info("BTLIF_FM_SET_REGION: status = %d, region = %d", \
			params->fm_II_param.i1, params->fm_II_param.i2);
		break;

	case BTLIF_FM_CONFIGURE_DEEMPHASIS:
		pr_bold_info("BTLIF_FM_CONFIGURE_DEEMPHASIS: status = %d, time_const = %d", \
			params->fm_II_param.i1, params->fm_II_param.i2);
		break;

	case BTLIF_FM_ESTIMATE_NFL:
		pr_bold_info("BTLIF_FM_ESTIMATE_NFL: nfloor = %d", params->fm_I_param.i1);
		break;

	case BTLIF_FM_GET_AUDIO_QUALITY:
		pr_bold_info("BTLIF_FM_GET_AUDIO_QUALITY: status = %d, rssi = %d, audio_mode = %d, snr = %d", \
			params->fm_IIII_param.i1, params->fm_IIII_param.i2, params->fm_IIII_param.i3, params->fm_IIII_param.i4);
		break;

	case BTLIF_FM_AF_JMP_EVT:
		pr_bold_info("BTLIF_FM_AF_JMP_EVT: status = %d, freq = %d, rssi = %d", \
			params->fm_III_param.i1, params->fm_III_param.i2, params->fm_III_param.i3);
		break;

	case BTLIF_FM_SET_VOLUME_EVT:
		pr_bold_info("BTLIF_FM_SET_VOLUME_EVT: status = %d, volume = %d", \
			params->fm_II_param.i1, params->fm_II_param.i2);
		break;

	default:
		pr_bold_info("Message ID = %d", id);
	}

	write(fm_dev.pipefd[1], &id, sizeof(id));
	write(fm_dev.pipefd[1], params, sizeof(*params));
}

static int brcm_fm_enable(struct brcm_fm_device *dev, int func_mask, int retry)
{
	int ret;

	while (1)
	{
		fm_disable();
		sleep(1);

		ret = fm_enable();
		if (ret >= 0)
		{
			break;
		}

		pr_red_info("fm_enable retry = %d", retry);

		if (retry)
		{
			retry--;
		}
		else
		{
			return -EFAULT;
		}
	}

	ret = pipe(dev->pipefd);
	if (ret < 0)
	{
		pr_error_info("pipe");
		goto out_power_down;
	}

	ret = BTL_IFC_RegisterSubSystem(&dev->ctrl_handle, SUB_FM, brcm_fm_data_handler, brcm_fm_ctrl_handler);
	if (ret != BTL_IF_SUCCESS)
	{
		pr_red_info("BTL_IFC_RegisterSubSystem");
		goto out_power_down;
	}

	ret = brcm_fm_ctrl_enable(dev, func_mask);
	if (ret != 0)
	{
		pr_red_info("brcm_fm_send_ctrl_command");
		goto out_power_down;
	}

	brcm_fm_audio_enable(1);

	return 0;

out_close_pipe:
	close(dev->pipefd[0]);
	close(dev->pipefd[1]);
out_power_down:
	fm_disable();
	return -EFAULT;
}

static int brcm_fm_disable(struct brcm_fm_device *dev)
{
	int ret;

	brcm_fm_audio_enable(0);

	ret = brcm_fm_ctrl_disable(dev);
	if (ret != 0)
	{
		pr_red_info("brcm_fm_send_ctrl_command");
		return -EFAULT;
	}

	close(dev->pipefd[0]);
	close(dev->pipefd[1]);

	return fm_disable();
}

static void brcm_fm_stop_signal_handler(int signum)
{
	pr_bold_info("signum = %d", signum);

	brcm_fm_disable(&fm_dev);
	exit(-1);
}

int main(int argc, char *argv[])
{
	int ret;
	int freq, rssi;

	ret = brcm_fm_enable(&fm_dev, 1 << 5 | 1 << 6, 5);
	if (ret < 0)
	{
		pr_red_info("brcm_fm_enable");
		return ret;
	}

	signal(SIGTERM, brcm_fm_stop_signal_handler);
	signal(SIGKILL, brcm_fm_stop_signal_handler);
	signal(SIGQUIT, brcm_fm_stop_signal_handler);
	signal(SIGINT, brcm_fm_stop_signal_handler);

	ret = brcm_fm_ctrl_set_volume(&fm_dev, 50);
	if (ret != 0)
	{
		pr_red_info("brcm_fm_set_volume");
		goto out_brcm_fm_disable;
	}

	ret = brcm_fm_ctrl_tune_radio(&fm_dev, 9140);
	if (ret < 0)
	{
		pr_red_info("brcm_fm_tune_radio");
		goto out_brcm_fm_disable;
	}

	if (ret == 0)
	{
		sleep(10);
	}

	while (1)
	{
		ret = brcm_fm_ctrl_seek_station(&fm_dev, BRCM_FM_SCAN_DIRECTION_UP, &freq, &rssi);
		if (ret < 0)
		{
			pr_red_info("brcm_fm_seek_station");
			goto out_brcm_fm_disable;
		}

		if (ret)
		{
			continue;
		}

		pr_bold_info("status = %d, freq = %d, rssi = %d", ret, freq, rssi);

		sleep(5);
	}

out_brcm_fm_disable:
	brcm_fm_disable(&fm_dev);

	return 0;
}
