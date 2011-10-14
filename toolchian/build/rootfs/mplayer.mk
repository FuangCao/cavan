FFMPEG_VERSION = 0.8.5
FFMPEG_NAME = ffmpeg-$(FFMPEG_VERSION)
FFMPEG_URL = http://ffmpeg.org/releases

include $(MAKEFILE_DEFINES)

all:
	$(call simple_decompression_file,$(FFMPEG_NAME),$(shell pwd)/ffmpeg,$(FFMPEG_URL))
	$(Q)sb2 ./configure --prefix=/usr
	$(Q)+sb2 make
	$(Q)+sb2 -m install make install
	
