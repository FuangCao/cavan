LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE = false

LOCAL_CFLAGS := -DCONFIG_BUILD_FOR_ANDROID -Werror
LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include

LOCAL_STATIC_LIBRARIES :=
LOCAL_SHARED_LIBRARIES := \
	libutils

LOCAL_SRC_FILES := \
	$(patsubst $(LOCAL_PATH)/%,%,$(wildcard $(LOCAL_PATH)/lib/*.c))

LOCAL_MODULE := libcavan
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

define BUILD_CAVAN_EXECUTABLE
include $(CLEAR_VARS)

LOCAL_CFLAGS := -DCONFIG_BUILD_FOR_ANDROID -Werror
LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include

LOCAL_STATIC_LIBRARIES :=
LOCAL_SHARED_LIBRARIES := \
	libcavan

LOCAL_SRC_FILES := $(1)

LOCAL_MODULE := cavan-$(basename $(notdir $(1)))
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)
endef

$(foreach app,$(patsubst $(LOCAL_PATH)/%,%,$(wildcard $(LOCAL_PATH)/app/*.c)),$(eval $(call BUILD_CAVAN_EXECUTABLE,$(app))))
