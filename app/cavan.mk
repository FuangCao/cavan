LOCAL_LIBRARY := libcavan

$(foreach fn,$(call search_all_files,*.c),$(eval $(call build_as_execute,$(fn))))

LOCAL_C_SOURCE := $(call search_all_files,*.c)
LOCAL_MODULE := main

include $(BUILD_EXEC_PKG)
