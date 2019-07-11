LOCAL_SOURCE := $(call search_all_files,*.c)

ifeq ($(BUILD_OTHERS),true)
LOCAL_SOURCE := $(LOCAL_SOURCE) $(call search_all_files,others/*.c)
endif

LOCAL_MODULE := libcavan

ifeq ($(filter static release,$(BUILD_TYPE)),)
include $(BUILD_LIBSO)
else
include $(BUILD_LIBA)
endif
