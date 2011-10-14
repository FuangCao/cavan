FFMPEG_VERSION = snapshot
FFMPEG_NAME = ffmpeg-$(FFMPEG_VERSION)
FFMPEG_URL = file:///home/cavan/Downloads

MAKE = make KERNEL_INC="$(ROOTFS_PATH)/usr/include"

include $(MAKEFILE_DEFINES)

all:
	$(call simple_decompression_file,$(FFMPEG_NAME),$(shell pwd)/ffmpeg,$(FFMPEG_URL))
	$(Q)test -d ffmpeg
	$(Q)sb2 ./configure --prefix=/usr
	$(Q)+sb2 $(MAKE)
	$(Q)+sb2 -m install $(MAKE) install
