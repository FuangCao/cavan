LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform
LOCAL_MODULE := bluetooth_setting

include $(BUILD_JAVA_LIBRARY)
