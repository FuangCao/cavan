LOCAL_PATH := $(call my-dir)

CAVAN_APP_SRC_FILES := $(wildcard $(LOCAL_PATH)/*.c)
CAVAN_APP_CORE_FILES := $(wildcard $(LOCAL_PATH)/core/*.c)

include $(CLEAR_VARS)

LOCAL_CFLAGS := -DCONFIG_BUILD_FOR_ANDROID -Werror
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include
LOCAL_STATIC_LIBRARIES := libcavan
LOCAL_SHARED_LIBRARIES := libutils
LOCAL_SRC_FILES := $(patsubst $(LOCAL_PATH)/%,%,$(CAVAN_APP_SRC_FILES) $(CAVAN_APP_CORE_FILES))
LOCAL_MODULE := cavan-main
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

LOCAL_INTERMEDIATES_DIR := $(call local-intermediates-dir)
CAVAN_MAP_H := $(LOCAL_INTERMEDIATES_DIR)/cavan_map.h
CAVAN_MAP_C := $(LOCAL_INTERMEDIATES_DIR)/cavan_map.c

$(LOCAL_INTERMEDIATES_DIR)/%.o: PRIVATE_CFLAGS += -Dmain=do_cavan_$(basename $(notdir $@))
$(LOCAL_INTERMEDIATES_DIR)/core/%.o: PRIVATE_CFLAGS += -Umain
$(CAVAN_APP_CORE_FILES): $(CAVAN_MAP_H) $(CAVAN_MAP_C)

$(CAVAN_MAP_H): $(CAVAN_APP_SRC_FILES)
	$(hide)for app in $(basename $(notdir $^)); \
	do \
		echo "int do_cavan_$${app}(int argc, char *argv[]);"; \
	done > $@

$(CAVAN_MAP_C): $(CAVAN_APP_SRC_FILES)
	$(hide)for app in $(basename $(notdir $^)); \
	do \
		echo "{\"$${app}\", do_cavan_$${app}},"; \
	done > $@
