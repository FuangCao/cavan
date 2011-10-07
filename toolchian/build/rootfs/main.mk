MAKEFILE_VERSIONS = $(OUT_ROOTFS)/version.mk
MAKEFILE_NAMES = $(OUT_ROOTFS)/name.mk
MAKEFILE_DEPENDS = $(OUT_ROOTFS)/depend.mk
XML_CONFIG = $(BUILD_ROOTFS)/config.xml

LIBRARY_MARK = $(MARK_ROOTFS)/library

include $(MAKEFILE_DEFINES)

all: $(MARK_ROOTFS_READY)
	$(Q)echo "ROOTFS compile successfull"

$(MARK_ROOTFS_READY): $(LIBRARY_MARK)
	$(Q)python $(PYTHON_PARSER) -m $(MARK_ROOTFS) -f install_rootfs -v $(MAKEFILE_VERSIONS) -n $(MAKEFILE_NAMES) -d $(MAKEFILE_DEPENDS) $(XML_CONFIG)
	$(Q)+make -f $(MAKEFILE_INSTALL) VERSION_MK=$(MAKEFILE_VERSIONS) NAME_MK=$(MAKEFILE_NAMES) DEPEND_MK=$(MAKEFILE_DEPENDS)
	$(call generate_mark)

$(LIBRARY_MARK):
	$(Q)cd $(ROOTFS_PATH) && mkdir lib usr/lib -p
	$(Q)cp $(SYSROOT_PATH)/lib/*.so $(ROOTFS_PATH)/lib -av
	$(Q)cp $(SYSROOT_PATH)/lib/*.so.* $(ROOTFS_PATH)/lib -av
	$(Q)cp $(SYSROOT_PATH)/usr/lib/*.so $(ROOTFS_PATH)/usr/lib -av
	$(Q)cp $(SYSROOT_PATH)/usr/lib/*.so.* $(ROOTFS_PATH)/usr/lib -av
	$(call generate_mark)
