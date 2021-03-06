LOCAL_PATH := $(call my-dir)

define cavan-all-files-under
$(patsubst $(LOCAL_PATH)/%,%,$(wildcard $(LOCAL_PATH)/$1))
endef

CAVAN_LIB_SRC_FILES := $(call cavan-all-files-under,lib/*.c)
CAVAN_LIB_SRC_FILES += $(call cavan-all-files-under,android/lib/*.c)

CAVAN_ANDROID_VERSION := $(firstword $(subst ., ,$(PLATFORM_VERSION)))
CAVAN_C_INCLUDES := $(LOCAL_PATH)/include $(LOCAL_PATH)/android/include
CAVAN_C_INCLUDES += system/extras/ext4_utils system/vold system/core/fs_mgr/include frameworks/av/include frameworks/native/libs/sensor/include external/libdrm external/libdrm/include/drm
CAVAN_SHARED_LIBRARIES := libutils libcutils liblog libhardware libbinder libz libselinux libEGL libdrm
CAVAN_STATIC_LIBRARIES := libbase
CAVAN_CFLAGS := -DCAVAN_ARCH_ARM -DCAVAN -DCONFIG_ANDROID -DCONFIG_ANDROID_VERSION=$(CAVAN_ANDROID_VERSION) -DCONFIG_CAVAN_DRM -DCONFIG_SENSOR_HAL
CAVAN_CFLAGS += -Werror -Wno-unused-parameter -Wno-overloaded-virtual -Wno-gnu-designator -Wno-date-time -Wno-varargs -D_FILE_OFFSET_BITS=64
CAVAN_CFLAGS += -include $(LOCAL_PATH)/android/include/android.h -include $(LOCAL_PATH)/include/cavan/config.h

include $(CLEAR_VARS)
LOCAL_MODULE := sensors.$(TARGET_BOARD_HARDWARE)
LOCAL_MULTILIB := both
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_CFLAGS := $(CAVAN_CFLAGS)
LOCAL_C_INCLUDES := $(CAVAN_C_INCLUDES)
LOCAL_SRC_FILES := $(CAVAN_LIB_SRC_FILES)
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS := optional
LOCAL_STATIC_LIBRARIES := $(CAVAN_STATIC_LIBRARIES)
LOCAL_SHARED_LIBRARIES := $(CAVAN_SHARED_LIBRARIES)
include $(BUILD_SHARED_LIBRARY)
