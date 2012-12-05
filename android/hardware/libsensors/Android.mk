LOCAL_PATH := $(call my-dir)

ifneq ($(TARGET_SIMULATOR),true)

include $(CLEAR_VARS)

LOCAL_MODULE := sensors.$(TARGET_BOARD_PLATFORM)

LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw

LOCAL_MODULE_TAGS := eng

LOCAL_CFLAGS := -DLOG_TAG=\"Sensors\" -DCONFIG_BUILD_FOR_ANDROID -Wall -Werror

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \

LOCAL_SRC_FILES := \
			hua_sensors.c \
			hua_gsensor.c

LOCAL_SHARED_LIBRARIES := liblog libcutils libdl

LOCAL_PRELINK_MODULE := false

include $(BUILD_SHARED_LIBRARY)

endif
