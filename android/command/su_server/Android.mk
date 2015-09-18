LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SHARED_LIBRARIES := \
	libutils \
	libcutils \
	liblog \
	libbinder

LOCAL_SRC_FILES := \
	SuService.cpp \
	ISuService.cpp

LOCAL_MODULE := libcavan_su
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_SHARED_LIBRARIES := \
	libutils \
	libcutils \
	liblog \
	libcavan_su \
	libbinder

LOCAL_SRC_FILES := main_su_server.cpp

LOCAL_MODULE := cavan_su_server
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_SHARED_LIBRARIES := \
	libutils \
	libcutils \
	liblog \
	libcavan_su \
	libbinder

LOCAL_SRC_FILES := su.cpp

LOCAL_MODULE := cavan_su
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)
