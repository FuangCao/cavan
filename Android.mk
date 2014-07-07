LOCAL_PATH := $(call my-dir)

define cavan-all-files-under
$(patsubst $(LOCAL_PATH)/%,%,$(wildcard $(LOCAL_PATH)/$1))
endef

CAVAN_LIB_SRC_FILES := $(call cavan-all-files-under,lib/*.c)
CAVAN_APP_SRC_FILES := $(call cavan-all-files-under,app/*.c)
CAVAN_APP_CORE_SRC_FILES := $(call cavan-all-files-under,app/core/*.c)

CAVAN_C_INCLUDES := $(LOCAL_PATH)/include
CAVAN_SHARED_LIBRARIES := libutils liblog libcorkscrew
CAVAN_CFLAGS := -DCONFIG_BUILD_FOR_ANDROID=1 -DCAVAN_ARCH_ARM -Werror -include $(LOCAL_PATH)/include/cavan/config.h

include $(CLEAR_VARS)
LOCAL_MODULE := libcavan
LOCAL_CFLAGS := $(CAVAN_CFLAGS)
LOCAL_C_INCLUDES := $(CAVAN_C_INCLUDES)
LOCAL_SRC_FILES := $(CAVAN_LIB_SRC_FILES)
LOCAL_SHARED_LIBRARIES := $(CAVAN_SHARED_LIBRARIES)
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libcavan
LOCAL_CFLAGS := $(CAVAN_CFLAGS)
LOCAL_C_INCLUDES := $(CAVAN_C_INCLUDES)
LOCAL_SRC_FILES := $(CAVAN_LIB_SRC_FILES)
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := $(CAVAN_SHARED_LIBRARIES)
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_CFLAGS := $(CAVAN_CFLAGS)
LOCAL_C_INCLUDES := $(CAVAN_C_INCLUDES)
LOCAL_SRC_FILES := $(CAVAN_APP_SRC_FILES) $(CAVAN_APP_CORE_SRC_FILES)
LOCAL_MODULE := cavan-main
LOCAL_MODULE_TAGS := optional
LOCAL_STATIC_LIBRARIES := libcavan
LOCAL_SHARED_LIBRARIES := $(CAVAN_SHARED_LIBRARIES)
include $(BUILD_EXECUTABLE)

CAVAN_OUT_PATH := $(intermediates)
CAVAN_MAP_H := $(CAVAN_OUT_PATH)/cavan_map.h
CAVAN_MAP_C := $(CAVAN_OUT_PATH)/cavan_map.c

$(addprefix $(LOCAL_PATH)/,$(CAVAN_APP_CORE_SRC_FILES)): $(CAVAN_MAP_H) $(CAVAN_MAP_C)

$(CAVAN_MAP_H): $(addprefix $(LOCAL_PATH)/,$(CAVAN_APP_SRC_FILES)) | $(CAVAN_OUT_PATH)
	$(hide) for app in $(basename $(notdir $^)); \
	do \
		echo "int do_cavan_$${app}(int argc, char *argv[]);"; \
	done > $@

$(CAVAN_MAP_C): $(addprefix $(LOCAL_PATH)/,$(CAVAN_APP_SRC_FILES)) | $(CAVAN_OUT_PATH)
	$(hide) for app in $(basename $(notdir $^)); \
	do \
		echo "{\"$${app}\", do_cavan_$${app}},"; \
	done > $@

$(CAVAN_OUT_PATH):
	$(hide) mkdir -pv $@

$(intermediates)/app/%.o: PRIVATE_CFLAGS += -Dmain=do_cavan_$(basename $(notdir $@))
$(intermediates)/app/core/%.o: PRIVATE_CFLAGS += -Umain -I$(CAVAN_OUT_PATH)
