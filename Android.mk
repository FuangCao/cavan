LOCAL_PATH := $(call my-dir)

define cavan-all-files-under
$(patsubst $(LOCAL_PATH)/%,%,$(wildcard $(LOCAL_PATH)/$1))
endef

CAVAN_LIB_SRC_FILES := $(call cavan-all-files-under,lib/*.c)
CAVAN_LIB_SRC_FILES += $(call cavan-all-files-under,android/lib/*.c)
CAVAN_APP_SRC_FILES := $(call cavan-all-files-under,app/*.c)
CAVAN_APP_SRC_FILES += $(call cavan-all-files-under,android/command/*.c)
CAVAN_APP_CORE_SRC_FILES := $(call cavan-all-files-under,app/core/*.c)

CAVAN_LIB_SRC_FILES += $(call cavan-all-files-under,cpp/*.cpp)
CAVAN_LIB_SRC_FILES += $(call cavan-all-files-under,android/lib/*.cpp)
CAVAN_APP_SRC_FILES += $(call cavan-all-files-under,app/*.cpp)
CAVAN_APP_SRC_FILES += $(call cavan-all-files-under,android/command/*.cpp)

CAVAN_ANDROID_VERSION := $(firstword $(subst ., ,$(PLATFORM_VERSION)))
CAVAN_C_INCLUDES := $(LOCAL_PATH)/include $(LOCAL_PATH)/android/include
CAVAN_C_INCLUDES += system/extras/ext4_utils system/vold system/core/fs_mgr/include frameworks/av/include
CAVAN_SHARED_LIBRARIES := libutils libcutils liblog libhardware libbinder liblogwrap libz libselinux libext4_utils libsparse libmedia libgui libEGL
CAVAN_STATIC_LIBRARIES := libfs_mgr libmincrypt
CAVAN_CFLAGS := -DCAVAN_ARCH_ARM -DCAVAN -DCONFIG_ANDROID -DCONFIG_ANDROID_VERSION=$(CAVAN_ANDROID_VERSION)
CAVAN_CFLAGS += -Wall -Wundef -Wextra -Werror -Wno-unused-parameter -D_FILE_OFFSET_BITS=64
CAVAN_CFLAGS += -include $(LOCAL_PATH)/android/include/android.h -include $(LOCAL_PATH)/include/cavan/config.h

include $(CLEAR_VARS)
LOCAL_MODULE := libcavan
LOCAL_CFLAGS := $(CAVAN_CFLAGS)
LOCAL_C_INCLUDES := $(CAVAN_C_INCLUDES)
LOCAL_SRC_FILES := $(CAVAN_LIB_SRC_FILES) $(CAVAN_APP_SRC_FILES)
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS := optional
LOCAL_STATIC_LIBRARIES := $(CAVAN_STATIC_LIBRARIES)
LOCAL_SHARED_LIBRARIES := $(CAVAN_SHARED_LIBRARIES)
include $(BUILD_SHARED_LIBRARY)

$(intermediates)/app/%.o $(intermediates)/android/command/%.o: PRIVATE_CFLAGS += -Dmain=do_cavan_$(basename $(notdir $@))

include $(CLEAR_VARS)
LOCAL_MODULE := libcavan-static
LOCAL_CFLAGS := $(CAVAN_CFLAGS)
LOCAL_C_INCLUDES := $(CAVAN_C_INCLUDES)
LOCAL_SRC_FILES := $(CAVAN_LIB_SRC_FILES) $(CAVAN_APP_SRC_FILES)
LOCAL_STATIC_LIBRARIES := $(CAVAN_STATIC_LIBRARIES)
LOCAL_SHARED_LIBRARIES := $(CAVAN_SHARED_LIBRARIES)
include $(BUILD_STATIC_LIBRARY)

$(intermediates)/app/%.o $(intermediates)/android/command/%.o: PRIVATE_CFLAGS += -Dmain=do_cavan_$(basename $(notdir $@))

include $(CLEAR_VARS)
LOCAL_CFLAGS := $(CAVAN_CFLAGS)
LOCAL_C_INCLUDES := $(CAVAN_C_INCLUDES)
LOCAL_SRC_FILES := $(CAVAN_APP_CORE_SRC_FILES)
LOCAL_MODULE := cavan-main
LOCAL_MODULE_TAGS := optional
LOCAL_STATIC_LIBRARIES := libcavan-static $(CAVAN_STATIC_LIBRARIES)
LOCAL_SHARED_LIBRARIES := $(CAVAN_SHARED_LIBRARIES)
include $(BUILD_EXECUTABLE)

CAVAN_OUT_PATH := $(intermediates)
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

$(intermediates)/app/core/%.o: PRIVATE_CFLAGS += -Umain -I$(CAVAN_OUT_PATH)
