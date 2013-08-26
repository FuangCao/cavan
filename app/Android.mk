LOCAL_PATH := $(call my-dir)

CAVAN_CFLAGS = 
CAVAN_INCLUDES = 

define BUILD_CAVAN_EXECUTABLE
include $(CLEAR_VARS)

LOCAL_CFLAGS := -DCONFIG_BUILD_FOR_ANDROID -Werror
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include
LOCAL_STATIC_LIBRARIES := libcavan
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SRC_FILES := $(1)
LOCAL_MODULE := cavan-$(basename $(notdir $(1)))
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)
endef

$(foreach app,$(patsubst $(LOCAL_PATH)/%,%,$(wildcard $(LOCAL_PATH)/*.c)),$(eval $(call BUILD_CAVAN_EXECUTABLE,$(app))))
