LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CFLAGS :=
LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../include

LOCAL_STATIC_LIBRARIES :=
LOCAL_SHARED_LIBRARIES := \
	libutils libhuamobile

LOCAL_SRC_FILES := \
	$(notdir $(wildcard $(LOCAL_PATH)/*.c))

LOCAL_MODULE := hua_tstest
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)
