include $(TOPDIR)/rules.mk

PKG_NAME := cavan
PKG_VERSION := 1.0
PKG_RELEASE := 1
PKG_INSTALL := 1
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

PKG_SOURCE_PROTO := git
PKG_SOURCE_URL := https://gitee.com/fuangcao/cavan.git
PKG_SOURCE_VERSION := master
PKG_SOURCE_SUBDIR := $(PKG_NAME)
PKG_SOURCE := $(PKG_NAME).tar.xz

include $(INCLUDE_DIR)/package.mk

TARGET_CFLAGS += -DCONFIG_OPENWRT
CONFIGURE_ARGS += --disable-cavan-cpp

define Package/cavan
	SECTION := utils
	CATEGORY := Utilities
	TITLE := Cavan Tools
	DEPENDS := +libpthread +librt
endef

define Package/cavan/description
	Cavan Tools
endef

define Package/cavan/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/app/cavan-main $(1)/usr/bin
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
