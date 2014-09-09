KERNEL_ARCH = $(CAVAN_TARGET_ARCH)

ifneq ($(filter i%86,$(CAVAN_TARGET_ARCH)),)
KERNEL_ARCH = x86
endif

ifeq ($(CAVAN_TARGET_ARCH),amd64)
KERNEL_ARCH = x86_64
endif

HEADER_OPTION = ARCH=$(KERNEL_ARCH) INSTALL_HDR_PATH=$(SYSROOT_PATH)/usr

all:
	$(Q)+make distclean
	$(Q)+make $(HEADER_OPTION) headers_install
