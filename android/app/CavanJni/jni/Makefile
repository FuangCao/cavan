APP_ABI := x86 armeabi # armeabi-v7a
CMD_NDK_BUILD := $(NDKROOT)/ndk-build

all:
	+$(CMD_NDK_BUILD) APP_ABI="$(APP_ABI)"
	adb push ../libs/armeabi/cavan-main /data/local/tmp

$(APP_ABI): Android.mk
	+$(CMD_NDK_BUILD) APP_ABI="$@"

clean:
	@rm -rf ../obj

.PHONY: $(APP_ABI)
