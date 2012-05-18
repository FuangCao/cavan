#include <jni.h>
#include <utils/Log.h>
#include <utils/misc.h>
#include <fcntl.h>
#include <asm/ioctl.h>
#include <errno.h>

#define TAG "TP_JNI"

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
	_IOR(_IOC_READ, SWAN_TS_IOCTL_TYPE_DEVICE, nr, size)

#define SWAN_TS_DEVICE_IOW(nr, size) \
	_IOW(SWAN_TS_IOCTL_TYPE_DEVICE, nr, size)

#define SWAN_TS_DEVICE_IOWM(nr, size) \
	_IOR(_IOC_WRITE, SWAN_TS_IOCTL_TYPE_DEVICE, nr, size)

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

/*
 * Class:     com_eavoo_touchscreen_EavooTouchScreen
 * Method:    OpenTouchscreenDevice
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_eavoo_touchscreen_EavooTouchScreen_OpenTouchscreenDevice(JNIEnv *env, jobject obj, jstring devpath)
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
		env->SetObjectField(obj, mFieldDeviceName, NULL);
	}
	else
	{
		LOGI("Device name = %s", buff);
		env->SetObjectField(obj, mFieldDeviceName, env->NewStringUTF(buff));
	}

	LOGI("fd = %d", fd);

	env->SetObjectField(obj, mFieldDevicePath, env->NewStringUTF(path));

	return true;
}

/*
 * Class:     com_eavoo_touchscreen_EavooTouchScreen
 * Method:    CloseTouchScreenDevice
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_eavoo_touchscreen_EavooTouchScreen_CloseTouchScreenDevice(JNIEnv *env, jobject obj)
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
 * Class:     com_eavoo_touchscreen_EavooTouchScreen
 * Method:    Calibration
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_eavoo_touchscreen_EavooTouchScreen_Calibration(JNIEnv *env, jobject obj)
{
	int fd;

	fd = env->GetIntField(obj, mFieldDeviceFd);
	if (fd < 0)
	{
		LOGI("Touchscreen device is not opened");
		return false;
	}

	LOGI("fd = %d", fd);

	if (ioctl(fd, SWAN_TS_IOCTL_CALIBRATION) < 0)
	{
		LOGE("ioctl SWAN_TS_IOCTL_CALIBRATION: %s", strerror(errno));
		return false;
	}

	return true;
}

/*
 * Class:     com_eavoo_touchscreen_EavooTouchScreen
 * Method:    UpgradeFt5406
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_eavoo_touchscreen_EavooTouchScreen_UpgradeFt5406(JNIEnv *, jobject, jstring)
{
	return false;
}

static JNINativeMethod nativeMethods[] =
{
	{"OpenTouchscreenDevice", "(Ljava/lang/String;)Z", (void *)Java_com_eavoo_touchscreen_EavooTouchScreen_OpenTouchscreenDevice},
	{"CloseTouchScreenDevice", "()Z", (void *)Java_com_eavoo_touchscreen_EavooTouchScreen_CloseTouchScreenDevice},
	{"Calibration", "()Z", (void *)Java_com_eavoo_touchscreen_EavooTouchScreen_Calibration},
	{"UpgradeFt5406", "(Ljava/lang/String;)Z", (void *)Java_com_eavoo_touchscreen_EavooTouchScreen_UpgradeFt5406}
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

	clazz = env->FindClass("com/eavoo/touchscreen/EavooTouchScreen");
	if (clazz == NULL)
	{
		LOGE("env->FindClass(\"com/eavoo/touchscreen/EavooTouchScreen\")");
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

