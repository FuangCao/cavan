APP_ABI := x86 armeabi armeabi-v7a

all:
	+ndk-build APP_ABI="$(APP_ABI)"

$(APP_ABI): Android.mk
	+ndk-build APP_ABI="$@"

.PHONY: $(APP_ABI)
