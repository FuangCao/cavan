MAKEFILE_VERSIONS = $(OUT_ROOTFS)/version.mk
MAKEFILE_NAMES = $(OUT_ROOTFS)/name.mk
MAKEFILE_DEPENDS = $(OUT_ROOTFS)/depend.mk
XML_CONFIG = $(BUILD_ROOTFS)/config.xml

ROOTFS_BASE = $(ROOTFS_PATH)/base
BASE_MARK = $(MARK_ROOTFS)/base

PKG_CONFIG_LIBDIR = $(SYSROOT_PATH)/usr/lib/pkgconfig

export PKG_CONFIG_LIBDIR

include $(MAKEFILE_DEFINES)

all: $(MARK_ROOTFS_READY)
	$(Q)echo "ROOTFS compile successfull"

$(MARK_ROOTFS_READY): $(BASE_MARK)
	$(Q)python $(PYTHON_PARSER) -m $(MARK_ROOTFS) -f install_rootfs -v $(MAKEFILE_VERSIONS) -n $(MAKEFILE_NAMES) -d $(MAKEFILE_DEPENDS) $(XML_CONFIG)
	$(Q)+make -f $(MAKEFILE_INSTALL) VERSION_MK=$(MAKEFILE_VERSIONS) NAME_MK=$(MAKEFILE_NAMES) DEPEND_MK=$(MAKEFILE_DEPENDS)
	$(call generate_mark)

$(BASE_MARK):
	$(Q)rm $(ROOTFS_PATH) -rfv
	$(Q)cp $(ROOTFS_BASE) $(ROOTFS_PATH) -av
	$(Q)cd $(ROOTFS_PATH) && mkdir lib usr/lib -pv
	$(call copy_shared_library,$(SYSROOT_PATH)/lib,$(ROOTFS_PATH)/lib)
	$(call copy_shared_library,$(SYSROOT_PATH)/usr/lib,$(ROOTFS_PATH)/usr/lib)
	$(call copy_shared_library,$(SYSROOT_PATH)/usr/$(CAVAN_TARGET_PLAT)/lib,$(ROOTFS_PATH)/lib)
	$(call generate_mark)
