LOCAL_PATH := $(call my-dir)

ifneq ($(TARGET_SIMULATOR),true)

include $(CLEAR_VARS)

LOCAL_MODULE := sensors.$(BOARD_PRODUCT_NAME)

LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw

LOCAL_MODULE_TAGS := eng

LOCAL_CFLAGS := -DLOG_TAG=\"Sensors\" -DCONFIG_BUILD_FOR_ANDROID -Wall -Werror

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \

LOCAL_SRC_FILES := \
			hua_sensor_core.c \
			hua_sensor_device.c

LOCAL_SHARED_LIBRARIES := libutils

LOCAL_PRELINK_MODULE := false

include $(BUILD_SHARED_LIBRARY)

endif
