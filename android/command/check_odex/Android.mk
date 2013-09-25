LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
		CheckOdex.cpp

LOCAL_C_INCLUDES := \
		dalvik \
		dalvik/libdex \
		dalvik/vm \
		external/zlib \
		frameworks/base/include

LOCAL_SHARED_LIBRARIES := libcutils libz
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := check_odex

include $(BUILD_EXECUTABLE)
