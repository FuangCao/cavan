#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <btl_if.h>

#define BRCM_FM_SCAN_MODE_UP	0x80
#define BRCM_FM_SCAN_MODE_DOWN	0x00
#define BRCM_FM_SCAN_MIN_SIGNAL	105

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
};

extern tBTL_IF_Result BTL_IFC_RegisterSubSystem(tCTRL_HANDLE *handle, tBTL_IF_SUBSYSTEM sub, tBTL_IF_DATA_CALLBACK data_cb, tBTL_IF_CTRL_CALLBACK ctrl_cb);
extern tBTL_IF_Result BTL_IFC_CtrlSend(int ctrl_fd, tSUB sub, tBTLIF_CTRL_MSG_ID msg_id, tBTL_PARAMS *params, int param_len);

extern int fm_enable();
extern int fm_disable();

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
		pr_red_info("Unknown event id = %d", id);
    }
}

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

static int brcm_fm_send_ctrl_command(struct brcm_fm_device *dev, int cmd, int arg)
{
	int ret;

	if (arg < 0)
	{
		ret = BTL_IFC_CtrlSend(dev->ctrl_handle, SUB_FM, cmd, NULL, 0);
	}
	else
	{
		tBTLIF_FM_REQ_I_PARAM params;

		params.i1 = arg;
		ret = BTL_IFC_CtrlSend(dev->ctrl_handle, SUB_FM, cmd, (tBTL_PARAMS *)(&params), sizeof(tBTLIF_FM_REQ_I_PARAM));
	}

	if (ret != BTL_IF_SUCCESS)
	{
		pr_red_info("BTL_IFC_CtrlSend cmd = %d, arg = %d", cmd, arg);
		return -EFAULT;
	}

	return 0;
}

static int brcm_fm_enable(struct brcm_fm_device *dev, int func_mask)
{
	int ret;

	pr_pos_info();

	ret = fm_enable();
	if (ret < 0)
	{
		pr_red_info("fm_enable");
		return ret;
	}

	ret = BTL_IFC_RegisterSubSystem(&dev->ctrl_handle, SUB_FM, brcm_fm_data_handler, brcm_fm_ctrl_handler);
	if (ret != BTL_IF_SUCCESS)
	{
		pr_red_info("BTL_IFC_RegisterSubSystem");
		goto out_fm_disable;
	}

	ret = brcm_fm_send_ctrl_command(dev, BTLIF_FM_ENABLE, func_mask);
	if (ret < 0)
	{
		pr_red_info("brcm_fm_send_ctrl_command");
		goto out_fm_disable;
	}

	brcm_fm_audio_enable(1);

	return 0;

out_fm_disable:
	fm_disable();
	return -EFAULT;
}

static int brcm_fm_disable(struct brcm_fm_device *dev)
{
	int ret;

	pr_pos_info();

	brcm_fm_audio_enable(0);

	ret = brcm_fm_send_ctrl_command(dev, BTLIF_FM_DISABLE, -1);
	if (ret < 0)
	{
		pr_red_info("brcm_fm_send_ctrl_command");
		return -EFAULT;
	}

	return fm_disable();
}

static inline int brcm_fm_tune_radio(struct brcm_fm_device *dev, int freq)
{
	return brcm_fm_send_ctrl_command(dev, BTLIF_FM_TUNE, freq);
}

static inline int brcm_fm_set_volume(struct brcm_fm_device *dev, int volume)
{
	return brcm_fm_send_ctrl_command(dev, BTLIF_FM_SET_VOLUME, volume);
}

static int brcm_fm_seek_station(struct brcm_fm_device *dev, int mode)
{
    int ret;
    tBTLIF_FM_REQ_IIII_PARAM params =
    {
		.i1 = mode,
		.i2 = BRCM_FM_SCAN_MIN_SIGNAL,
		.i3 = 0,
		.i4 = 0
    };

    ret = BTL_IFC_CtrlSend(dev->ctrl_handle, SUB_FM, BTLIF_FM_SEARCH, (tBTL_PARAMS *)(&params), sizeof(tBTLIF_FM_REQ_IIII_PARAM));
    if(ret != BTL_IF_SUCCESS)
    {
		pr_red_info("BTL_IFC_CtrlSend");
		return -EFAULT;
    }

	return 0;
}

int main(int argc, char *argv[])
{
	int ret;
	int i;
	struct brcm_fm_device fm_dev;

	ret = brcm_fm_enable(&fm_dev, 1 << 5 | 1 << 6);
	if (ret < 0)
	{
		pr_red_info("brcm_fm_enable");
		return ret;
	}

	ret = brcm_fm_set_volume(&fm_dev, 50);
	if (ret < 0)
	{
		pr_red_info("brcm_fm_set_volume");
		goto out_brcm_fm_disable;
	}

	sleep(1);

	for (i = 0; i < 10; i++)
	{
		ret = brcm_fm_seek_station(&fm_dev, BRCM_FM_SCAN_MODE_UP);
		if (ret < 0)
		{
			pr_red_info("brcm_fm_seek_station");
			goto out_brcm_fm_disable;
		}

		sleep(5);
	}

#if 0
	ret = brcm_fm_tune_radio(&fm_dev, 9140);
	if (ret < 0)
	{
		pr_red_info("brcm_fm_tune_radio");
		goto out_brcm_fm_disable;
	}
#endif

out_brcm_fm_disable:
	brcm_fm_disable(&fm_dev);

	return 0;
}
