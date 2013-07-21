LOCAL_SOURCE := $(call search_all_files,*.cpp)
LOCAL_MODULE := libcavan++

include $(BUILD_LIBA)
include $(BUILD_LIBSO)
