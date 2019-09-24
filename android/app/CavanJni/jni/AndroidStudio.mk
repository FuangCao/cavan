LOCAL_PATH := $(call my-dir)/../../../..

CAVAN_PROJECT_PATH := android/app/CavanJni
CAVAN_JNI_PATH := $(CAVAN_PROJECT_PATH)/jni

define cavan-all-files-under
$(patsubst $(LOCAL_PATH)/%,%,$(wildcard $(LOCAL_PATH)/$1))
endef

CAVAN_LIB_SRC_FILES := $(call cavan-all-files-under,lib/*.c)
CAVAN_LIB_SRC_FILES += $(call cavan-all-files-under,android/lib/*.c)
CAVAN_APP_SRC_FILES := $(call cavan-all-files-under,app/*.c)
CAVAN_APP_SRC_FILES += $(call cavan-all-files-under,android/command/*.c)
CAVAN_APP_CORE_SRC_FILES := $(call cavan-all-files-under,app/core/*.c)

CAVAN_LIB_SRC_FILES += $(call cavan-all-files-under,cpp/*.cpp)
CAVAN_APP_SRC_FILES += $(call cavan-all-files-under,app/*.cpp)

CAVAN_ANDROID_VERSION := $(firstword $(subst ., ,$(PLATFORM_VERSION)))
CAVAN_C_INCLUDES := $(LOCAL_PATH)/include $(LOCAL_PATH)/android/include $(LOCAL_PATH)/$(CAVAN_PROJECT_PATH)/build/generated
CAVAN_CFLAGS := -DCAVAN_ARCH_ARM -DCAVAN -DCONFIG_ANDROID -DCONFIG_ANDROID_VERSION=$(CAVAN_ANDROID_VERSION) -DCONFIG_ANDROID_NDK
CAVAN_CFLAGS += -Wno-unused-parameter -Wno-gnu-designator
CAVAN_CFLAGS += -include $(LOCAL_PATH)/android/include/android.h -include $(LOCAL_PATH)/include/cavan/config.h

CAVAN_CPPFLAGS := -Wno-overloaded-virtual

CAVAN_LDLIBS = -llog -landroid
CAVAN_OUT_PATH := $(TARGET_OBJS)
CAVAN_MAP_H := $(CAVAN_OUT_PATH)/cavan_map.h
CAVAN_MAP_C := $(CAVAN_OUT_PATH)/cavan_map.c
CAVAN_NATIVE_NAME := com_cavan_cavanjni_CavanNative
CAVAN_NATIVE_C := $(CAVAN_JNI_PATH)/$(CAVAN_NATIVE_NAME).c

include $(CLEAR_VARS)
LOCAL_MODULE := libcavan-static
LOCAL_CFLAGS := $(CAVAN_CFLAGS)
LOCAL_CPPFLAGS := $(CAVAN_CPPFLAGS)
LOCAL_C_INCLUDES := $(CAVAN_C_INCLUDES)
LOCAL_SRC_FILES := $(CAVAN_LIB_SRC_FILES)
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libcavan-app-static
LOCAL_CFLAGS := $(CAVAN_CFLAGS) -DCAVAN_CMD_PREFIX=do_cavan_
LOCAL_CPPFLAGS := $(CAVAN_CPPFLAGS)
LOCAL_C_INCLUDES := $(CAVAN_C_INCLUDES)
LOCAL_SRC_FILES := $(CAVAN_APP_SRC_FILES)
LOCAL_STATIC_LIBRARIES := libcavan-static
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libcavan-jni
LOCAL_CFLAGS := $(CAVAN_CFLAGS)
LOCAL_CPPFLAGS := $(CAVAN_CPPFLAGS)
LOCAL_C_INCLUDES := $(CAVAN_C_INCLUDES)
LOCAL_SRC_FILES := $(CAVAN_NATIVE_C) $(CAVAN_JNI_PATH)/CavanMain.c $(CAVAN_JNI_PATH)/com_cavan_cavanjni_CavanJni.c
LOCAL_LDLIBS := $(CAVAN_LDLIBS)
LOCAL_STATIC_LIBRARIES := libcavan-app-static
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_CFLAGS := $(CAVAN_CFLAGS)
LOCAL_CPPFLAGS := $(CAVAN_CPPFLAGS)
LOCAL_C_INCLUDES := $(CAVAN_C_INCLUDES) $(CAVAN_OUT_PATH)
LOCAL_SRC_FILES := $(CAVAN_APP_CORE_SRC_FILES)
LOCAL_MODULE := cavan-main
LOCAL_LDLIBS := $(CAVAN_LDLIBS)
LOCAL_STATIC_LIBRARIES := libcavan-app-static
include $(BUILD_EXECUTABLE)