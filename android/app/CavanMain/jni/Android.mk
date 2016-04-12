LOCAL_PATH := $(call my-dir)/../../../..

define cavan-all-files-under
$(patsubst $(LOCAL_PATH)/%,%,$(wildcard $(LOCAL_PATH)/$1))
endef

CAVAN_LIB_SRC_FILES := $(call cavan-all-files-under,lib/*.c)
CAVAN_LIB_SRC_FILES += $(call cavan-all-files-under,android/lib/*.c)
CAVAN_APP_SRC_FILES := $(call cavan-all-files-under,app/*.c)
CAVAN_APP_SRC_FILES += $(call cavan-all-files-under,android/command/*.c)
CAVAN_APP_CORE_SRC_FILES := $(call cavan-all-files-under,app/core/*.c)

CAVAN_LIB_SRC_FILES += $(call cavan-all-files-under,cpp/*.cpp)
CAVAN_APP_SRC_FILES += $(call cavan-all-files-under,app/*.cpp)

CAVAN_ANDROID_VERSION := $(firstword $(subst ., ,$(PLATFORM_VERSION)))
CAVAN_C_INCLUDES := $(LOCAL_PATH)/include $(LOCAL_PATH)/android/include
CAVAN_CFLAGS := -DCAVAN_ARCH_ARM -DCAVAN -DCONFIG_ANDROID -DCONFIG_ANDROID_VERSION=$(CAVAN_ANDROID_VERSION)  -DCONFIG_ANDROID_NDK -Wall -Wundef -Wextra -Werror -Wno-unused-parameter
CAVAN_CFLAGS += -include $(LOCAL_PATH)/android/include/android.h -include $(LOCAL_PATH)/include/cavan/config.h
CAVAN_LDLIBS = -llog -landroid
CAVAN_OUT_PATH := $(TARGET_OBJS)

CAVAN_APP_LIBRARIES :=

define cavan-build-app-lib
include $$(CLEAR_VARS)
LOCAL_CMD_NAME := $$(basename $$(notdir $1))
LOCAL_MODULE := libcavan_main_$$(LOCAL_CMD_NAME)
CAVAN_APP_LIBRARIES += $$(LOCAL_MODULE)
LOCAL_CFLAGS := $$(CAVAN_CFLAGS) -Dmain=do_cavan_$$(LOCAL_CMD_NAME)
LOCAL_C_INCLUDES := $$(CAVAN_C_INCLUDES)
LOCAL_SRC_FILES := $1
include $$(BUILD_STATIC_LIBRARY)
endef

$(foreach app,$(CAVAN_APP_SRC_FILES),$(eval $(call cavan-build-app-lib,$(app))))

include $(CLEAR_VARS)
LOCAL_MODULE := libcavan-static
LOCAL_CFLAGS := $(CAVAN_CFLAGS)
LOCAL_C_INCLUDES := $(CAVAN_C_INCLUDES)
LOCAL_STATIC_LIBRARIES := $(CAVAN_APP_LIBRARIES)
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libcavan
LOCAL_CFLAGS := $(CAVAN_CFLAGS)
LOCAL_C_INCLUDES := $(CAVAN_C_INCLUDES)
LOCAL_SRC_FILES := $(CAVAN_LIB_SRC_FILES)
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS := optional
LOCAL_LDLIBS := $(CAVAN_LDLIBS)
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_CFLAGS := $(CAVAN_CFLAGS)
LOCAL_C_INCLUDES := $(CAVAN_C_INCLUDES) $(CAVAN_OUT_PATH)
LOCAL_SRC_FILES := $(CAVAN_APP_CORE_SRC_FILES)
LOCAL_MODULE := cavan-main
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libcavan
LOCAL_STATIC_LIBRARIES := $(CAVAN_APP_LIBRARIES)
include $(BUILD_EXECUTABLE)

CAVAN_MAP_H := $(CAVAN_OUT_PATH)/cavan_map.h
CAVAN_MAP_C := $(CAVAN_OUT_PATH)/cavan_map.c

$(addprefix $(LOCAL_PATH)/,$(CAVAN_APP_CORE_SRC_FILES)): $(CAVAN_MAP_H) $(CAVAN_MAP_C)

$(CAVAN_MAP_H): $(addprefix $(LOCAL_PATH)/,$(CAVAN_APP_SRC_FILES)) | $(CAVAN_OUT_PATH)
	$(hide) echo "Automatically generate $@"
	$(hide) { \
		echo "#include <cavan.h>"; \
		echo; \
		echo "__BEGIN_DECLS"; \
		echo; \
		for app in $(basename $(notdir $^)); \
		do \
			echo "int do_cavan_$${app}(int argc, char *argv[]);"; \
		done; \
		echo; \
		echo "__END_DECLS"; \
	} > $@

$(CAVAN_MAP_C): $(addprefix $(LOCAL_PATH)/,$(CAVAN_APP_SRC_FILES)) | $(CAVAN_OUT_PATH)
	$(hide) echo "Automatically generate $@"
	$(hide) for app in $(basename $(notdir $^)); \
	do \
		echo "{ \"$${app}\", do_cavan_$${app} },"; \
	done > $@

$(CAVAN_OUT_PATH):
	$(hide) mkdir -pv $@
