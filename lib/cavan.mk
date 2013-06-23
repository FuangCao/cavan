LOCAL_C_SOURCE := $(call search_all_files,*.c)
LOCAL_MODULE := libcavan

include $(BUILD_LIBA)
include $(BUILD_LIBSO)
