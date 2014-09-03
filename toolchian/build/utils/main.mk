XML_CONFIG1 = $(BUILD_UTILS)/config1.xml
XML_CONFIG2 = $(BUILD_UTILS)/config2.xml

PKG_CONFIG_LIBDIR = $(UTILS_PATH)/usr/lib/pkgconfig
UTILS_COMMON_CONFIG = --prefix=$(UTILS_PATH)/usr

export PKG_CONFIG_LIBDIR UTILS_COMMON_CONFIG

include $(MAKEFILE_DEFINES)

all: $(MARK_UTILS_READY2)
	$(Q)echo "Host utils compile successfull"

$(MARK_UTILS_READY1):
	$(call auto_make,install_utils,$(MARK_UTILS),$(OUT_UTILS),$(XML_CONFIG1))

$(MARK_UTILS_READY2): $(MARK_UTILS_READY1)
	$(call auto_make,install_utils,$(MARK_UTILS),$(OUT_UTILS),$(XML_CONFIG2))
