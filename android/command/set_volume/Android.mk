LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := framework/base/include
LOCAL_SHARED_LIBRARIES := libmedia

LOCAL_SRC_FILES := \
	$(notdir $(wildcard $(LOCAL_PATH)/*.c $(LOCAL_PATH)/*.cpp))

LOCAL_MODULE := set_volume
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)
