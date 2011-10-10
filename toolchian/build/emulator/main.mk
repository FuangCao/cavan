MAKEFILE_VERSIONS = $(OUT_EMULATOR)/version.mk
MAKEFILE_NAMES = $(OUT_EMULATOR)/name.mk
MAKEFILE_DEPENDS = $(OUT_EMULATOR)/depend.mk
XML_CONFIG = $(BUILD_EMULATOR)/config.xml

SB2_LIBTOOL_VERSION = 1.5.26
SB2_LIBTOOL_NAME = libtool-$(SB2_LIBTOOL_VERSION)
SB2_INIT_MARK = $(MARK_EMULATOR)/sb2_init
SB2_CONFIG_PATH = $(HOME)/.scratchbox2

include $(MAKEFILE_DEFINES)

all: $(MARK_EMULATOR_READY)
	$(Q)echo "Emulator compile successfull"

$(MARK_EMULATOR_READY): $(SB2_INIT_MARK)
	$(Q)python $(PYTHON_PARSER) -m $(MARK_EMULATOR) -f install_emulator -v $(MAKEFILE_VERSIONS) -n $(MAKEFILE_NAMES) -d $(MAKEFILE_DEPENDS) $(XML_CONFIG)
	$(Q)+make -f $(MAKEFILE_INSTALL) VERSION_MK=$(MAKEFILE_VERSIONS) NAME_MK=$(MAKEFILE_NAMES) DEPEND_MK=$(MAKEFILE_DEPENDS)
	$(Q)ln -vsf bash $(EMULATOR_PATH)/bin/bash
	$(call generate_mark)

$(SB2_INIT_MARK):
	$(Q)cd $(EMULATOR_PATH) && mkdir bin sbin root home tmp proc sys dev etc usr/bin usr/sbin -pv && ln -vsf bash bin/sh -vsf
	$(Q)cp $(SYSROOT_PATH)/lib $(EMULATOR_PATH) -av
	$(Q)cp $(SYSROOT_PATH)/usr/lib $(SYSROOT_PATH)/usr/$(CAVAN_TARGET_PLAT)/lib $(EMULATOR_PATH)/usr -av
	$(Q)cp $(SYSROOT_PATH)/usr/include $(SYSROOT_PATH)/usr/$(CAVAN_TARGET_PLAT)/include $(EMULATOR_PATH)/usr -av
	$(eval SB2_LIBTOOL_PACKAGE = $(firstword $(wildcard $(DOWNLOAD_PATH)/$(SB2_LIBTOOL_NAME).tar.gz $(PACKAGE_PATH)/$(SB2_LIBTOOL_NAME).tar.gz)))
	$(Q)test -n "$(SB2_LIBTOOL_PACKAGE)" && rm $(SB2_CONFIG_PATH) -rf && mkdir $(SB2_CONFIG_PATH) -pv && cp $(SB2_LIBTOOL_PACKAGE) $(SB2_CONFIG_PATH) -av
	$(Q)cd $(EMULATOR_PATH) && sb2-init -c qemu-$(CAVAN_TARGET_ARCH) $(CAVAN_TARGET_PLAT) $(CAVAN_TARGET_PLAT)-gcc
	$(call generate_mark)
