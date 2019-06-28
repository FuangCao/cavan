SHELL := /bin/bash

LOCAL_PATH := $(call my-dir)/../../../..

CAVAN_RES_PATH := $(call my-dir)/../res
CAVAN_RAW_PATH := $(CAVAN_RES_PATH)/raw
CAVAN_MAIN_PATH := $(CAVAN_RAW_PATH)/cavan_main_$(TARGET_ARCH_ABI)

CAVAN_PROJECT_PATH := android/app/CavanJni
CAVAN_JNI_PATH := $(CAVAN_PROJECT_PATH)/jni
CAVAN_MAKEFILE := $(LOCAL_PATH)/$(CAVAN_JNI_PATH)/Android.mk

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

CAVAN_APP_LIST := $(foreach app,$(CAVAN_APP_SRC_FILES),$(basename $(notdir $(app))))

CAVAN_ANDROID_VERSION := $(firstword $(subst ., ,$(PLATFORM_VERSION)))
CAVAN_C_INCLUDES := $(LOCAL_PATH)/include $(LOCAL_PATH)/android/include
CAVAN_CFLAGS := -DCAVAN_ARCH_ARM -DCAVAN -DCONFIG_ANDROID -DCONFIG_ANDROID_VERSION=$(CAVAN_ANDROID_VERSION) -DCONFIG_ANDROID_NDK
CAVAN_CFLAGS += -Wall -Wundef -Wextra -Werror -Wno-unused-parameter -Wno-gnu-designator
CAVAN_CFLAGS += -include $(LOCAL_PATH)/android/include/android.h -include $(LOCAL_PATH)/include/cavan/config.h

CAVAN_CPPFLAGS := -Wno-overloaded-virtual

CAVAN_LDLIBS = -llog -landroid
CAVAN_OUT_PATH := $(TARGET_OBJS)
CAVAN_MAP_H := $(CAVAN_OUT_PATH)/cavan_map.h
CAVAN_MAP_C := $(CAVAN_OUT_PATH)/cavan_map.c
CAVAN_NATIVE_NAME := com_cavan_cavanjni_CavanNative
CAVAN_NATIVE_JAVA := $(LOCAL_PATH)/$(CAVAN_PROJECT_PATH)/src/$(subst _,/,$(CAVAN_NATIVE_NAME)).java
CAVAN_NATIVE_C := $(CAVAN_JNI_PATH)/$(CAVAN_NATIVE_NAME).c

define cavan-build-app-lib
include $$(CLEAR_VARS)
LOCAL_APP_NAME := $$(basename $$(notdir $1))
LOCAL_MODULE := libcavan_app_$$(LOCAL_APP_NAME)
LOCAL_CFLAGS := $$(CAVAN_CFLAGS) -Dmain=do_cavan_$$(LOCAL_APP_NAME)
LOCAL_CPPFLAGS := $$(CAVAN_CPPFLAGS)
LOCAL_C_INCLUDES := $$(CAVAN_C_INCLUDES)
LOCAL_SRC_FILES := $1
LOCAL_STATIC_LIBRARIES := libcavan-static
include $$(BUILD_STATIC_LIBRARY)

CAVAN_APP_LIBRARIES += $$(LOCAL_MODULE)
endef

CAVAN_APP_LIBRARIES :=
$(foreach app,$(CAVAN_APP_SRC_FILES),$(eval $(call cavan-build-app-lib,$(app))))

include $(CLEAR_VARS)
LOCAL_MODULE := libcavan-static
LOCAL_CFLAGS := $(CAVAN_CFLAGS)
LOCAL_CPPFLAGS := $(CAVAN_CPPFLAGS)
LOCAL_C_INCLUDES := $(CAVAN_C_INCLUDES)
LOCAL_SRC_FILES := $(CAVAN_LIB_SRC_FILES)
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libcavan-jni
LOCAL_CFLAGS := $(CAVAN_CFLAGS)
LOCAL_CPPFLAGS := $(CAVAN_CPPFLAGS)
LOCAL_C_INCLUDES := $(CAVAN_C_INCLUDES)
LOCAL_SRC_FILES := $(CAVAN_NATIVE_C) $(CAVAN_JNI_PATH)/CavanMain.c $(CAVAN_JNI_PATH)/com_cavan_cavanjni_CavanJni.c
LOCAL_LDLIBS := $(CAVAN_LDLIBS)
LOCAL_STATIC_LIBRARIES := $(CAVAN_APP_LIBRARIES) libcavan-static
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_CFLAGS := $(CAVAN_CFLAGS)
LOCAL_CPPFLAGS := $(CAVAN_CPPFLAGS)
LOCAL_C_INCLUDES := $(CAVAN_C_INCLUDES) $(CAVAN_OUT_PATH)
LOCAL_SRC_FILES := $(CAVAN_APP_CORE_SRC_FILES)
LOCAL_MODULE := cavan-main
LOCAL_LDLIBS := $(CAVAN_LDLIBS)
LOCAL_STATIC_LIBRARIES := $(CAVAN_APP_LIBRARIES)
include $(BUILD_EXECUTABLE)

$(CAVAN_MAIN_PATH): $(call map,module-get-installed,$(LOCAL_MODULE)) | $(CAVAN_RAW_PATH)
	@cp -av "$<" "$@"

WANTED_INSTALLED_MODULES += $(CAVAN_MAIN_PATH)

$(addprefix $(LOCAL_PATH)/,$(CAVAN_APP_CORE_SRC_FILES)): $(CAVAN_MAP_H) $(CAVAN_MAP_C)

$(CAVAN_MAP_H): $(addprefix $(LOCAL_PATH)/,$(CAVAN_APP_SRC_FILES)) | $(CAVAN_OUT_PATH)
	$(hide) echo -e "Automatically generate $@"
	$(hide) { \
		echo -e "#include <cavan.h>"; \
		echo; \
		echo -e "__BEGIN_DECLS"; \
		echo; \
		for app in $(CAVAN_APP_LIST); \
		do \
			echo -e "int do_cavan_$${app}(int argc, char *argv[]);"; \
		done; \
		echo; \
		echo -e "__END_DECLS"; \
	} > $@

$(CAVAN_MAP_C): $(addprefix $(LOCAL_PATH)/,$(CAVAN_APP_SRC_FILES)) | $(CAVAN_OUT_PATH)
	$(hide) echo -e "Automatically generate $@"
	$(hide) for app in $(CAVAN_APP_LIST); \
	do \
		echo -e "{ \"$${app}\", do_cavan_$${app} },"; \
	done > $@

$(LOCAL_PATH)/$(CAVAN_NATIVE_C): $(addprefix $(LOCAL_PATH)/,$(CAVAN_APP_SRC_FILES)) $(CAVAN_NATIVE_JAVA) $(CAVAN_MAKEFILE) | $(CAVAN_OUT_PATH)
	$(hide) echo -e "Automatically generate $@"
	$(hide) { \
		echo -e "/* This file was automatically generated by Fuang.Cao, Do not modify */"; \
		echo; \
		echo -e "#include \"CavanMain.h\""; \
		for app in $(CAVAN_APP_LIST); \
		do \
			native_app=$$(echo $${app} | sed 's/\(^\|_\)\([a-z]\)/\u\2/g'); \
			echo; \
			echo -e "extern int do_cavan_$${app}(int argc, char *argv[]);"; \
			echo; \
			echo -e "JNIEXPORT jint Java_$(CAVAN_NATIVE_NAME)_do$${native_app}(JNIEnv *env, jclass clazz, jboolean async, jobjectArray args)"; \
			echo -e "{"; \
			echo -e "\treturn CavanMainExecute(env, \"$${app}\", async, args, do_cavan_$${app});"; \
			echo -e "}"; \
		done; \
	} > $@

$(CAVAN_NATIVE_JAVA): $(addprefix $(LOCAL_PATH)/,$(CAVAN_APP_SRC_FILES)) $(CAVAN_MAKEFILE) | $(CAVAN_OUT_PATH)
	$(hide) echo -e "Automatically generate $@"
	$(hide) mkdir -pv "$(dir $@)"
	$(hide) { \
		class_name=$(basename $(notdir $@)); \
		echo -e "/* This class was automatically generated by Fuang.Cao, Do not modify */"; \
		echo; \
		echo -e "package com.cavan.cavanjni;"; \
		echo; \
		echo -e "import java.util.HashMap;"; \
		for app in $(CAVAN_APP_LIST); \
		do \
			native_app=$$(echo $${app} | sed 's/\(^\|_\)\([a-z]\)/\u\2/g'); \
			echo; \
			echo -e "class CavanCommand$${native_app} extends CavanNativeCommand {"; \
			echo; \
			echo -e "\t@Override"; \
			echo -e "\tpublic int main(boolean async, String... args) {"; \
			echo -e "\t\treturn $${class_name}.do$${native_app}(async, args);"; \
			echo -e "\t}"; \
			echo -e "}"; \
		done; \
		echo; \
		echo -e "public class $${class_name} {"; \
		echo; \
		echo -e "\tpublic static HashMap<String, CavanNativeCommand> sHashMap = new HashMap<String, CavanNativeCommand>();"; \
		echo; \
		echo -e "\tstatic {"; \
		echo -e "\t\tSystem.loadLibrary(\"cavan-jni\");"; \
		echo; \
		for app in $(CAVAN_APP_LIST); \
		do \
			native_app=$$(echo $${app} | sed 's/\(^\|_\)\([a-z]\)/\u\2/g'); \
			echo -e "\t\tsHashMap.put(\"$${app}\", new CavanCommand$${native_app}());"; \
		done; \
		echo -e "\t}"; \
		echo; \
		for app in $(CAVAN_APP_LIST); \
		do \
			native_app=$$(echo $${app} | sed 's/\(^\|_\)\([a-z]\)/\u\2/g'); \
			echo -e "\tpublic static native int do$${native_app}(boolean async, String... args);"; \
		done; \
		echo -e "}"; \
	} > $@

$(CAVAN_OUT_PATH) $(CAVAN_RAW_PATH):
	$(hide) mkdir -pv "$@"
