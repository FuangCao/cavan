LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := hardware/libhardware
LOCAL_SHARED_LIBRARIES := libhardware

LOCAL_SRC_FILES := \
	$(notdir $(wildcard $(LOCAL_PATH)/*.c))

LOCAL_MODULE := sensor_poll
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)
