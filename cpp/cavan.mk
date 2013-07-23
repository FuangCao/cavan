LOCAL_SOURCE := $(call search_all_files,*.cpp)
LOCAL_MODULE := libcavan++

ifeq ($(filter -static,$(LDFLAGS)),)
include $(BUILD_LIBSO)
else
include $(BUILD_LIBA)
endif
