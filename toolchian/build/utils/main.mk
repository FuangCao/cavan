XML_CONFIG1 = $(BUILD_UTILS)/config1.xml
XML_CONFIG2 = $(BUILD_UTILS)/config2.xml

XZ_VERSION = 5.0.3
XZ_NAME = xz-$(XZ_VERSION)
XZ_URL = http://tukaani.org/xz
XZ_CONFIG =
XZ_MARK = $(MARK_UTILS)/$(XZ_NAME)

PKG_CONFIG_LIBDIR = $(UTILS_PATH)/usr/lib/pkgconfig
UTILS_COMMON_CONFIG = --prefix=$(UTILS_PATH)/usr

export PKG_CONFIG_LIBDIR UTILS_COMMON_CONFIG

include $(MAKEFILE_DEFINES)

all: $(MARK_UTILS_READY2)
	$(Q)echo "Host utils compile successfull"

$(MARK_UTILS_READY1): $(XZ_MARK)
	$(call auto_make,install_utils,$(MARK_UTILS),$(OUT_UTILS),$(XML_CONFIG1))

$(MARK_UTILS_READY2): $(MARK_UTILS_READY1)
	$(call auto_make,install_utils,$(MARK_UTILS),$(OUT_UTILS),$(XML_CONFIG2))

$(XZ_MARK):
	$(eval DOWNLOAD_TYPES_BAK = $(DOWNLOAD_TYPES))
	$(eval DOWNLOAD_TYPES = $(filter-out %.xz,$(DOWNLOAD_TYPES)))
	$(call install_utils,$(XZ_CONFIG),$(XZ_URL))
	$(eval DOWNLOAD_TYPES = $(DOWNLOAD_TYPES_BAK))
