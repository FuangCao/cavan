XML_CONFIG = $(BUILD_UTILS)/config.xml

XZ_VERSION = 5.0.3
XZ_NAME = xz-$(XZ_VERSION)
XZ_URL = http://tukaani.org/xz
XZ_CONFIG =
XZ_MARK = $(MARK_UTILS)/$(XZ_NAME)

PKG_CONFIG_LIBDIR = $(UTILS_PATH)/usr/lib/pkgconfig
UTILS_COMMON_CONFIG = --prefix=$(UTILS_PATH)/usr

export PKG_CONFIG_LIBDIR UTILS_COMMON_CONFIG

include $(MAKEFILE_DEFINES)

all: $(MARK_UTILS_READY)
	$(Q)echo "Host utils compile successfull"

$(MARK_UTILS_READY): $(XZ_MARK)
	$(call auto_make,install_utils,$(MARK_UTILS),$(OUT_UTILS),$(XML_CONFIG))

$(XZ_MARK):
	$(eval DOWNLOAD_TYPES_BAK = $(DOWNLOAD_TYPES))
	$(eval DOWNLOAD_TYPES = $(filter-out %.xz,$(DOWNLOAD_TYPES)))
	$(call install_utils,$(XZ_CONFIG),$(XZ_URL))
	$(eval DOWNLOAD_TYPES = $(DOWNLOAD_TYPES_BAK))
