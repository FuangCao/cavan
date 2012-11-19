LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE = false

LOCAL_CFLAGS := -DCONFIG_BUILD_FOR_ANDROID -Werror
LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../include

LOCAL_STATIC_LIBRARIES :=
LOCAL_SHARED_LIBRARIES := \
	libutils

LOCAL_SRC_FILES := \
	$(notdir $(wildcard $(LOCAL_PATH)/*.c))

LOCAL_MODULE := libhuamobile
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
