#include <jni.h>
#include <utils/Log.h>
#include <utils/misc.h>
#include <fcntl.h>
#include <asm/ioctl.h>
#include <errno.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "TP_JNI"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

struct swan_ts_i2c_request
{
	u16 offset;
	u16 size;
	void *data;
};

enum swan_ts_ioctl_type
{
	SWAN_TS_IOCTL_TYPE_CORE,
	SWAN_TS_IOCTL_TYPE_DEVICE,
	SWAN_TS_IOCTL_TYPE_OTHER
};

struct ft5406_firmware_data_package
{
	u32 size;
	void *data;
};

#define SWAN_TS_CORE_IOR(nr, size) \
	_IOR(SWAN_TS_IOCTL_TYPE_CORE, nr, size)

#define SWAN_TS_CORE_IORM(nr, size) \
	_IOC(_IOC_READ, SWAN_TS_IOCTL_TYPE_CORE, nr, size)

#define SWAN_TS_CORE_IOW(nr, size) \
	_IOW(SWAN_TS_IOCTL_TYPE_CORE, nr, size)

#define SWAN_TS_CORE_IOWM(nr, size) \
	_IOC(_IOC_WRITE, SWAN_TS_IOCTL_TYPE_CORE, nr, size)

#define SWAN_TS_CORE_IO(nr) \
	SWAN_TS_CORE_IORM(nr, 0)

#define SWAN_TS_DEVICE_IOR(nr, size) \
	_IOR(SWAN_TS_IOCTL_TYPE_DEVICE, nr, size)

#define SWAN_TS_DEVICE_IORM(nr, size) \
	_IOC(_IOC_READ, SWAN_TS_IOCTL_TYPE_DEVICE, nr, size)

#define SWAN_TS_DEVICE_IOW(nr, size) \
	_IOW(SWAN_TS_IOCTL_TYPE_DEVICE, nr, size)

#define SWAN_TS_DEVICE_IOWM(nr, size) \
	_IOC(_IOC_WRITE, SWAN_TS_IOCTL_TYPE_DEVICE, nr, size)

#define SWAN_TS_DEVICE_IO(nr) \
		SWAN_TS_DEVICE_IORM(nr, 0)

#define SWAN_TS_INPUT_IOCTL_CALIBRATION		_IO('E', 0x01)
#define SWAN_TS_IOCTL_CALIBRATION			SWAN_TS_CORE_IO(0x01)
#define SWAN_TS_IOCTL_READ_REGISTERS		SWAN_TS_CORE_IOR(0x02, struct swan_ts_i2c_request)
#define SWAN_TS_IOCTL_WRITE_REGISTERS		SWAN_TS_CORE_IOW(0x03, struct swan_ts_i2c_request)
#define SWAN_TS_IOCTL_READ_DATA				SWAN_TS_CORE_IOR(0x04, struct swan_ts_i2c_request)
#define SWAN_TS_IOCTL_WRITE_DATA			SWAN_TS_CORE_IOW(0x05, struct swan_ts_i2c_request)
#define SWAN_TS_IOCTL_GET_CLIENT_ADDRESS	SWAN_TS_CORE_IOR(0x06, u16)
#define SWAN_TS_IOCTL_SET_CLIENT_ADDRESS	SWAN_TS_CORE_IOW(0x07, u16)
#define SWAN_TS_IOCTL_TEST_CLIENT			SWAN_TS_CORE_IOW(0x08, u16)
#define SWAN_TS_IOCTL_DETECT_CLIENT			SWAN_TS_CORE_IOW(0x09, u32)
#define SWAN_TS_IOCTL_REQUEST_IRQ			SWAN_TS_CORE_IO(0x10)
#define SWAN_TS_IOCTL_FREE_IRQ				SWAN_TS_CORE_IO(0x11)
#define SWAN_TS_IOCTL_DISABLE_IRQ			SWAN_TS_CORE_IO(0x12)
#define SWAN_TS_IOCTL_ENABLE_IRQ			SWAN_TS_CORE_IO(0x13)
#define SWAN_TS_IOCTL_GET_NAME(len)			SWAN_TS_CORE_IORM(0x14, len)

#define FT5406_IOCTL_UPGRADE_ENTER		SWAN_TS_DEVICE_IO(0x01)
#define FT5406_IOCTL_UPGRADE_FINISH		SWAN_TS_DEVICE_IO(0x02)
#define FT5406_IOCTL_ERASE_APP_CONFIG	SWAN_TS_DEVICE_IO(0x03)
#define FT5406_IOCTL_ERASE_APP			SWAN_TS_DEVICE_IO(0x04)
#define FT5406_IOCTL_ERASE_CONFIG		SWAN_TS_DEVICE_IO(0x05)
#define FT5406_IOCTL_ERASE_PARAM		SWAN_TS_DEVICE_IO(0x06)
#define FT5406_IOCTL_READ_CHECKSUM		SWAN_TS_DEVICE_IOR(0x07, u8)
#define FT5406_IOCTL_UPGRADE_APP		SWAN_TS_DEVICE_IOW(0x08, struct ft5406_firmware_data_package)
#define FT5406_IOCTL_SINGLE_WRITE		SWAN_TS_DEVICE_IOW(0x09, struct ft5406_firmware_data_package)

static jfieldID mFieldDeviceFd;
static jfieldID mFieldDevicePath;
static jfieldID mFieldDeviceName;

static inline int ft5406_upgrade_enter(int fd)
{
	return ioctl(fd, FT5406_IOCTL_UPGRADE_ENTER);
}

static inline int ft5406_upgrade_finish(int fd)
{
	return ioctl(fd, FT5406_IOCTL_UPGRADE_FINISH);
}

static inline int ft5406_erase_app_and_config(int fd)
{
	return ioctl(fd, FT5406_IOCTL_ERASE_APP_CONFIG);
}

static inline int ft5406_erase_app(int fd)
{
	return ioctl(fd, FT5406_IOCTL_ERASE_APP);
}

static inline int ft5406_erase_config(int fd)
{
	return ioctl(fd, FT5406_IOCTL_ERASE_CONFIG);
}

static inline int ft5406_erase_param(int fd)
{
	return ioctl(fd, FT5406_IOCTL_ERASE_PARAM);
}

static inline int ft5406_read_checksum(int fd, u8 *checksum)
{
	return ioctl(fd, FT5406_IOCTL_READ_CHECKSUM, checksum);
}

static inline ssize_t file_read_byte(int fd, void *buff)
{
	return read(fd, buff, 1);
}

ssize_t ft5406_read_firmware_data(const char *cfgpath, char *buff, size_t size)
{
	int fd;
	char *buff_bak, *buff_end;
	ssize_t readlen;
	char readval;

	fd = open(cfgpath, O_RDONLY);
	if (fd < 0)
	{
		LOGE("Failed to open file %s", cfgpath);
		return fd;
	}

	for (buff_bak = buff, buff_end = buff + size; buff < buff_end; buff++)
	{
		u8 value;

		while (1)
		{
			readlen = file_read_byte(fd, &readval);
			if (readlen < 0)
			{
				goto out_close_fd;
			}

			if (readlen < 1)
			{
				goto out_cal_len;
			}

			if (readval != '0')
			{
				continue;
			}

			readlen = file_read_byte(fd, &readval);
			if (readlen < 0)
			{
				goto out_close_fd;
			}

			if (readlen < 1)
			{
				goto out_cal_len;
			}

			if (readval == 'x' || readval == 'X')
			{
				break;
			}
		}

		value = 0;

		while (1)
		{
			readlen = file_read_byte(fd, &readval);
			if (readlen < 0)
			{
				goto out_close_fd;
			}

			if (readlen < 1)
			{
				*buff = value;
				goto out_cal_len;
			}

			if (readval >= '0' && readval <= '9')
			{
				value = (value << 4) + readval - '0';
			}
			else if (readval >= 'a' && readval <= 'z')
			{
				value = (value << 4) + readval - 'a' + 10;
			}
			else if (readval >= 'A' && readval <= 'Z')
			{
				value = (value << 4) + readval - 'A' + 10;
			}
			else
			{
				*buff = value;
				break;
			}
		}
	}

out_cal_len:
	readlen = buff - buff_bak;
out_close_fd:
	close(fd);

	return readlen;
}

u8 ft5406_calculate_checksum(const char *buff, size_t size)
{
	const char *buff_end;
	u8 checksum;

	for (buff_end = buff + size, checksum = 0; buff < buff_end; buff++)
	{
		checksum ^= *(u8 *)buff;
	}

	return checksum;
}

int ft5406_firmware_write_last_data(int fd, const void *buff, size_t size)
{
	struct ft5406_firmware_data_package pkg = {size, (void *)buff};

	return ioctl(fd, FT5406_IOCTL_SINGLE_WRITE, &pkg);
}

int ft5406_firmware_upgrade(int dev_fd, const char *cfgpath)
{
	int ret;
	char buff[50 << 10] ;
	ssize_t writelen, bufflen;
	u8 checksum[2];

	bufflen = ft5406_read_firmware_data(cfgpath, buff, sizeof(buff)) - 2;
	if (bufflen < 6)
	{
		LOGE("ft5406_parse_app_file");
		return bufflen;
	}

	LOGI("bufflen = %ld", bufflen);

	ret = ft5406_upgrade_enter(dev_fd);
	if (ret < 0)
	{
		LOGE("ft5406_upgrade_start");
		return ret;
	}

	ret = ft5406_erase_app(dev_fd);
	if (ret < 0)
	{
		LOGE("ft5406_erase_app");
		return ret;
	}

	writelen = write(dev_fd, buff, bufflen - 6);
	if (writelen < 0)
	{
		LOGE("write");
		return writelen;
	}

	writelen = ft5406_firmware_write_last_data(dev_fd, buff + writelen, 6);
	if (writelen < 0)
	{
		LOGE("ft5406_firmware_write_last_data");
		return writelen;
	}

	ret = ft5406_read_checksum(dev_fd, checksum);
	if (ret < 0)
	{
		LOGE("ft5406_read_checksum");
		return ret;
	}

	checksum[1] = ft5406_calculate_checksum(buff, bufflen);

	LOGI("Source checksum = 0x%02x, Dest checksum = 0x%02x", checksum[1], checksum[0]);

	if (checksum[0] != checksum[1])
	{
		LOGE("Checksum do't match");
		return -EFAULT;
	}

	return ft5406_upgrade_finish(dev_fd);
}

/*
 * Class:     com_eavoo_touchscreen_TouchScreen
 * Method:    OpenTouchscreenDevice
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_eavoo_touchscreen_TouchScreen_OpenTouchscreenDeviceNative(JNIEnv *env, jobject obj, jstring devpath)
{
	int fd;
	const char *path;
	char buff[1024];

	fd = env->GetIntField(obj, mFieldDeviceFd);
	if (fd > 0)
	{
		close(fd);
	}

	path = (const char *)env->GetStringUTFChars(devpath, NULL);
	if (path == NULL)
	{
		return false;
	}

	LOGI("Open device %s", path);

	fd = open(path, O_RDWR);
	env->SetIntField(obj, mFieldDeviceFd, fd);
	if (fd < 0)
	{
		return false;
	}

	if (ioctl(fd, SWAN_TS_IOCTL_GET_NAME(sizeof(buff)), buff) < 0)
	{
		LOGE("SWAN_TS_IOCTL_GET_NAME failed");
		env->SetObjectField(obj, mFieldDeviceName, env->NewStringUTF("Unknown"));
	}
	else
	{
		LOGI("Device name = %s", buff);
		env->SetObjectField(obj, mFieldDeviceName, env->NewStringUTF(buff));
	}

	env->SetObjectField(obj, mFieldDevicePath, env->NewStringUTF(path));

	return true;
}

/*
 * Class:     com_eavoo_touchscreen_TouchScreen
 * Method:    CloseTouchScreenDevice
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_eavoo_touchscreen_TouchScreen_CloseTouchScreenDeviceNative(JNIEnv *env, jobject obj)
{
	int fd;

	fd = env->GetIntField(obj, mFieldDeviceFd);
	if (fd < 0)
	{
		return false;
	}

	close(fd);
	env->SetIntField(obj, mFieldDeviceFd, -1);

	return true;
}

/*
 * Class:     com_eavoo_touchscreen_TouchScreen
 * Method:    Calibration
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_eavoo_touchscreen_TouchScreen_CalibrationNative(JNIEnv *env, jobject obj)
{
	int fd;

	fd = env->GetIntField(obj, mFieldDeviceFd);
	if (fd < 0)
	{
		LOGI("Touchscreen device is not opened");
		return false;
	}

	if (ioctl(fd, SWAN_TS_IOCTL_CALIBRATION) < 0)
	{
		LOGE("ioctl SWAN_TS_IOCTL_CALIBRATION: %s", strerror(errno));
		return false;
	}

	return true;
}

/*
 * Class:     com_eavoo_touchscreen_TouchScreen
 * Method:    UpgradeFt5406
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_eavoo_touchscreen_TouchScreen_UpgradeFt5406Native(JNIEnv *env, jobject obj, jstring pathname)
{
	int fd;
	const char *cfgpath;
	int ret;

	fd = env->GetIntField(obj, mFieldDeviceFd);
	if (fd < 0)
	{
		LOGI("Touchscreen device is not opened");
		return false;
	}

	cfgpath = (const char *)env->GetStringUTFChars(pathname, NULL);
	if (cfgpath == NULL)
	{
		return false;
	}

	return ft5406_firmware_upgrade(fd, cfgpath) < 0 ? false : true;
}

static JNINativeMethod nativeMethods[] =
{
	{"OpenTouchscreenDeviceNative", "(Ljava/lang/String;)Z", (void *)Java_com_eavoo_touchscreen_TouchScreen_OpenTouchscreenDeviceNative},
	{"CloseTouchScreenDeviceNative", "()Z", (void *)Java_com_eavoo_touchscreen_TouchScreen_CloseTouchScreenDeviceNative},
	{"CalibrationNative", "()Z", (void *)Java_com_eavoo_touchscreen_TouchScreen_CalibrationNative},
	{"UpgradeFt5406Native", "(Ljava/lang/String;)Z", (void *)Java_com_eavoo_touchscreen_TouchScreen_UpgradeFt5406Native}
};

jint JNI_OnLoad(JavaVM *jvm, void *reserved)
{
	jint ret;
	JNIEnv *env;
	jclass clazz;

	LOGI(">>>>>>>>>> JNI_OnLoad <<<<<<<<<<");

	ret = jvm->GetEnv((void **)&env, JNI_VERSION_1_6);
	if (ret < 0)
	{
		LOGE("jvm->GetEnv(&env, JNI_VERSION_1_6)");
		return ret;
	}

	clazz = env->FindClass("com/eavoo/touchscreen/TouchScreen");
	if (clazz == NULL)
	{
		LOGE("env->FindClass(\"com/eavoo/touchscreen/TouchScreen\")");
		return -1;
	}

	mFieldDeviceFd = env->GetFieldID(clazz, "mDeviceFd", "I");
	mFieldDevicePath = env->GetFieldID(clazz, "mDevicePath", "Ljava/lang/String;");
	mFieldDeviceName = env->GetFieldID(clazz, "mDeviceName", "Ljava/lang/String;");

	ret = env->RegisterNatives(clazz, nativeMethods, NELEM(nativeMethods));
	if (ret < 0)
	{
		LOGE("env->RegisterNatives(clazz, nativeMethods, NELEM(nativeMethods))");
		return ret;
	}

	return JNI_VERSION_1_6;
}
