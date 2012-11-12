LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CFLAGS =
LOCAL_C_INCLUDES = \
	external/dbus \
	external/bluetooth/bluez/src \
	external/bluetooth/bluez/lib \
	system/bluetooth/bluedroid/include

LOCAL_STATIC_LIBRARIES =
LOCAL_SHARED_LIBRARIES = \
	libbluedroid libdbus libbluetooth

LOCAL_SRC_FILES = hua_bttest.c
LOCAL_MODULE := hua_bttest
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

