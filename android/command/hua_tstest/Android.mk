LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CFLAGS =
LOCAL_C_INCLUDES =
LOCAL_STATIC_LIBRARIES =
LOCAL_SHARED_LIBRARIES = libutils

LOCAL_SRC_FILES = \
	$(call all-c-files-under,.)

LOCAL_MODULE = hua_tstest
LOCAL_MODULE_TAGS = optional

include $(BUILD_EXECUTABLE)

