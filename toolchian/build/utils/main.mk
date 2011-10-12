MAKEFILE_VERSIONS = $(OUT_UTILS)/version.mk
MAKEFILE_NAMES = $(OUT_UTILS)/name.mk
MAKEFILE_DEPENDS = $(OUT_UTILS)/depend.mk
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
	$(Q)python $(PYTHON_PARSER) -m $(MARK_UTILS) -f install_utils -v $(MAKEFILE_VERSIONS) -n $(MAKEFILE_NAMES) -d $(MAKEFILE_DEPENDS) $(XML_CONFIG)
	$(Q)+make -f $(MAKEFILE_INSTALL) VERSION_MK=$(MAKEFILE_VERSIONS) NAME_MK=$(MAKEFILE_NAMES) DEPEND_MK=$(MAKEFILE_DEPENDS)
	$(call generate_mark)

$(XZ_MARK):
	$(eval DOWNLOAD_TYPES = $(filter-out tar.xz,$(DOWNLOAD_TYPES)))
	$(call install_utils,$(XZ_CONFIG),$(XZ_URL))
	$(eval DOWNLOAD_TYPES := tar.xz $(DOWNLOAD_TYPES))
