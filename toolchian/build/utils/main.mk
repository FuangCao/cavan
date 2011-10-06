MAKEFILE_VERSIONS = $(OUT_UTILS)/version.mk
MAKEFILE_NAMES = $(OUT_UTILS)/name.mk
MAKEFILE_DEPENDS = $(OUT_UTILS)/depend.mk
XML_CONFIG = $(BUILD_UTILS)/config.xml

MARK_HOST_APPS = $(MARK_UTILS)/host_app
HOST_APPS = zlib1g-dev xz-utils python-dev libffi-dev m4 gawk gettext

include $(MAKEFILE_DEFINES)

all: $(MARK_UTILS_READY)
	$(Q)echo "Host utils compile successfull"

$(MARK_UTILS_READY): $(MARK_HOST_APPS)
	$(Q)python $(PYTHON_PARSER) -m $(MARK_UTILS) -f install_utils -v $(MAKEFILE_VERSIONS) -n $(MAKEFILE_NAMES) -d $(MAKEFILE_DEPENDS) $(XML_CONFIG)
	$(Q)+make -f $(MAKEFILE_INSTALL) VERSION_MK=$(MAKEFILE_VERSIONS) NAME_MK=$(MAKEFILE_NAMES) DEPEND_MK=$(MAKEFILE_DEPENDS)
	$(call generate_mark)

$(MARK_HOST_APPS):
	$(Q)sudo apt-get install $(HOST_APPS)
	$(call generate_mark)
