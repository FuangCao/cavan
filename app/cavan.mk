LOCAL_LIBRARY := libcavan

ifeq ($(BUILD_ENTRY),cavan)
LOCAL_SOURCE := $(call search_all_files,*.c *.cc *.cpp *.cxx)
$(if $(filter %.cc %.cpp %.cxx,$(LOCAL_SOURCE)),$(eval LOCAL_LIBRARY += libcavan++))
LOCAL_MODULE := main

include $(BUILD_EXEC_PKG)
else
$(foreach fn,$(call search_all_files,*.c),$(eval $(call build_as_execute,$(fn))))
LOCAL_LIBRARY += libcavan++
$(foreach fn,$(call search_all_files,*.cc *.cpp *.cxx),$(eval $(call build_as_execute,$(fn))))
endif
