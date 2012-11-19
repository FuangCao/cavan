LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE = false

LOCAL_CFLAGS := -DCONFIG_BUILD_FOR_ANDROID -Werror
LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include

LOCAL_STATIC_LIBRARIES :=
LOCAL_SHARED_LIBRARIES := \
	libutils libsysutils

LOCAL_SRC_FILES := \
	$(patsubst $(LOCAL_PATH)/%,%,$(wildcard $(LOCAL_PATH)/lib/*.c))

LOCAL_MODULE := libcavan
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
