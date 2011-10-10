MAKEFILE_VERSIONS = $(OUT_ROOTFS)/version.mk
MAKEFILE_NAMES = $(OUT_ROOTFS)/name.mk
MAKEFILE_DEPENDS = $(OUT_ROOTFS)/depend.mk
XML_CONFIG = $(BUILD_ROOTFS)/config.xml

SB2_LIBTOOL_VERSION = 1.5.26
SB2_LIBTOOL_NAME = libtool-$(SB2_LIBTOOL_VERSION)
SB2_INIT_MARK = $(MARK_ROOTFS)/sb2_init
SB2_CONFIG_PATH = $(HOME)/.scratchbox2
ROOTFS_BASE = $(BUILD_ROOTFS)/base

include $(MAKEFILE_DEFINES)

all: $(MARK_ROOTFS_READY)
	$(Q)echo "ROOTFS compile successfull"

$(MARK_ROOTFS_READY): $(SB2_INIT_MARK)
	$(Q)python $(PYTHON_PARSER) -m $(MARK_ROOTFS) -f install_rootfs -v $(MAKEFILE_VERSIONS) -n $(MAKEFILE_NAMES) -d $(MAKEFILE_DEPENDS) $(XML_CONFIG)
	$(Q)+make -f $(MAKEFILE_INSTALL) VERSION_MK=$(MAKEFILE_VERSIONS) NAME_MK=$(MAKEFILE_NAMES) DEPEND_MK=$(MAKEFILE_DEPENDS)
	$(call generate_mark)

$(SB2_INIT_MARK):
	$(Q)rm $(ROOTFS_PATH) -rf && mkdir $(ROOTFS_PATH) -pv && cp $(ROOTFS_BASE)/* $(ROOTFS_PATH) -av
	$(Q)cd $(ROOTFS_PATH) && mkdir bin sbin root home tmp proc sys dev etc usr/bin usr/sbin -pv && ln bash bin/sh -vsf
	$(Q)cp $(SYSROOT_PATH)/lib $(ROOTFS_PATH) -av
	$(Q)cp $(SYSROOT_PATH)/usr/lib $(SYSROOT_PATH)/usr/$(CAVAN_TARGET_PLAT)/lib $(ROOTFS_PATH)/usr -av
	$(eval SB2_LIBTOOL_PACKAGE = $(firstword $(wildcard $(DOWNLOAD_PATH)/$(SB2_LIBTOOL_NAME).tar.gz $(PACKAGE_PATH)/$(SB2_LIBTOOL_NAME).tar.gz)))
	$(Q)test -n "$(SB2_LIBTOOL_PACKAGE)" && rm $(SB2_CONFIG_PATH) -rf && mkdir $(SB2_CONFIG_PATH) -pv && cp $(SB2_LIBTOOL_PACKAGE) $(SB2_CONFIG_PATH) -av
	$(Q)cd $(ROOTFS_PATH) && sb2-init -c qemu-$(CAVAN_TARGET_ARCH) $(CAVAN_TARGET_PLAT) $(CAVAN_TARGET_PLAT)-gcc
	$(call generate_mark)
