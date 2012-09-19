XML_CONFIG = $(BUILD_ROOTFS)/config.xml

SB2_LIBTOOL_VERSION = 1.5.26
SB2_LIBTOOL_NAME = libtool-$(SB2_LIBTOOL_VERSION)
SB2_LIBTOOL_URL = http://ftp.gnu.org/gnu/libtool/$(SB2_LIBTOOL_NAME).tar.gz
SB2_INIT_MARK = $(MARK_ROOTFS)/sb2_init
SB2_CONFIG_PATH = $(HOME)/.scratchbox2
ROOTFS_BASE = $(BUILD_ROOTFS)/base

include $(MAKEFILE_DEFINES)

define find_libtool_package
$(firstword $(wildcard $(DOWNLOAD_PATH)/$(SB2_LIBTOOL_NAME).tar.gz $(PACKAGE_PATH)/$(SB2_LIBTOOL_NAME).tar.gz))
endef

all: $(MARK_ROOTFS_READY)
	$(Q)ln -vsf vim $(ROOTFS_PATH)/usr/bin/vi
	$(Q)chmod 0440 $(ROOTFS_PATH)/etc/sudoers
	$(Q)echo "ROOTFS compile successfull"

$(MARK_ROOTFS_READY): $(SB2_INIT_MARK)
	$(call auto_make,install_rootfs,$(MARK_ROOTFS),$(OUT_ROOTFS),$(XML_CONFIG))

$(SB2_INIT_MARK):
	$(Q)rm $(ROOTFS_PATH) -rf && cp $(ROOTFS_BASE) $(ROOTFS_PATH) -a
	$(Q)cd $(ROOTFS_PATH) && mkdir bin sbin root home/cavan lib usr/lib libexec tmp proc sys dev etc usr/bin usr/sbin var/run -pv
	$(Q)ln -vsf bash $(ROOTFS_PATH)/bin/sh
	$(Q)cp $(SYSROOT_BT_PATH)/* $(TOOLCHIAN_BT_PATH)/$(CAVAN_TARGET_PLAT)/lib $(ROOTFS_PATH) -a
	$(Q)cp $(TOOLCHIAN_BT_PATH)/$(CAVAN_TARGET_PLAT)/include $(ROOTFS_PATH)/usr -a
	$(Q)[ "$(SB2_LIBTOOL_PACKAGE)" ] || $(call download_package,$(SB2_LIBTOOL_NAME),$(SB2_LIBTOOL_URL))
	$(eval SB2_LIBTOOL_PACKAGE = $(call find_libtool_package))
	$(Q)[ "$(SB2_LIBTOOL_PACKAGE)" ] || \
	{ \
		$(call pr_red_info,No package $(SB2_LIBTOOL_NAME) found!); \
		exit 1; \
	}
	$(Q)rm $(SB2_CONFIG_PATH) -rf && mkdir $(SB2_CONFIG_PATH) -pv && cp $(SB2_LIBTOOL_PACKAGE) $(SB2_CONFIG_PATH) -a
	$(Q)cd $(ROOTFS_PATH) && sb2-init -c qemu-$(CAVAN_TARGET_ARCH) $(CAVAN_TARGET_PLAT) $(CAVAN_TARGET_PLAT)-gcc
	$(call generate_mark)
