include $(MAKEFILE_DEFINES)

all:
	CC=$(CAVAN_TARGET_PLAT)-gcc \
	AR=$(CAVAN_TARGET_PLAT)-ar \
	./configure --prefix=/usr --shared
	+make
	$(call install_to_sysroot)
