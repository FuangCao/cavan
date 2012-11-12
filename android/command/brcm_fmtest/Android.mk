LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CFLAGS = -DPLATFORM_ANDROID  -DBRCM_BT_USE_BTL_IF -DBT_USE_BTL_IF
LOCAL_C_INCLUDES = \
			$(BRCM_BT_INC_ROOT_PATH)/adaptation/btl-if/client \
			$(BRCM_BT_INC_ROOT_PATH)/adaptation/btl-if/include

LOCAL_STATIC_LIBRARIES =
LOCAL_SHARED_LIBRARIES =

LOCAL_SRC_FILES = brcm_fmtest.c
LOCAL_MODULE := brcm_fmtest
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

