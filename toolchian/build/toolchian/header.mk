KERNEL_ARCH = $(CAVAN_TARGET_ARCH)

ifneq ($(filter x86%64,$(CAVAN_TARGET_ARCH)),)
KERNEL_ARCH = ia64
endif

ifneq ($(filter i%86,$(CAVAN_TARGET_ARCH)),)
KERNEL_ARCH = x86
endif

HEADER_OPTION = ARCH=$(KERNEL_ARCH) INSTALL_HDR_PATH=$(SYSROOT_PATH)/usr

all:
	$(Q)+make distclean
	$(Q)+make $(HEADER_OPTION) headers_install
