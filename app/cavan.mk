LOCAL_LIBRARY := libcavan

ifeq ($(BUILD_ENTRY),cavan)
LOCAL_SOURCE := $(call search_all_files,*.c *.cc *.cpp *.cxx)

ifeq ($(BUILD_OTHERS),true)
LOCAL_SOURCE := $(LOCAL_SOURCE) $(call search_all_files,others/*.c others/*.cc others/*.cpp others/*.cxx)
endif

$(if $(filter %.cc %.cpp %.cxx,$(LOCAL_SOURCE)),$(eval LOCAL_LIBRARY := libcavan++ $(LOCAL_LIBRARY)))
LOCAL_MODULE := main

include $(BUILD_EXEC_PKG)
else
LOCAL_SOURCE := $(call search_all_files,*.c)

ifeq ($(BUILD_OTHERS),true)
LOCAL_SOURCE := $(LOCAL_SOURCE) $(call search_all_files,others/*.c)
endif

$(foreach fn,$(LOCAL_SOURCE),$(eval $(call build_as_execute,$(fn))))
LOCAL_SOURCE := $(call search_all_files,*.cc *.cpp *.cxx)

ifeq ($(BUILD_OTHERS),true)
LOCAL_SOURCE := $(LOCAL_SOURCE) $(call search_all_files,others/*.cc others/*.cpp others/*.cxx)
endif

LOCAL_LIBRARY := libcavan++ $(LOCAL_LIBRARY)
$(foreach fn,$(LOCAL_SOURCE),$(eval $(call build_as_execute,$(fn))))
endif
