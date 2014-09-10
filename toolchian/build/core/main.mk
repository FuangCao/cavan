LOWEST_KERNEL_VERSION = 2.6.15
GCC_VERSION_LIST = $(patsubst config-%.mk,%,$(notdir $(wildcard build/toolchian/config-*.mk)))

include build/toolchian/config-$(GCC_VERSION).mk

export BINUTILS_VERSION GCC_VERSION GLIBC_VERSION KERNEL_VERSION LOWEST_KERNEL_VERSION
export GMP_VERSION MPFR_VERSION MPC_VERSION

ROOT_PATH = $(shell pwd)
BUILD_PATH = $(ROOT_PATH)/build
PACKAGE_PATH = $(ROOT_PATH)/package
PATCH_PATH = $(ROOT_PATH)/patch
SCRIPT_PATH = $(BUILD_PATH)/script

DOWNLOAD_PATH = $(word 1,$(wildcard /source /work/source $(HOME)/source $(HOME)/work/source))
ifeq ($(DOWNLOAD_PATH),)
DOWNLOAD_PATH = $(HOME)/download
endif

CAVAN_TARGET_PLAT = $(CAVAN_TARGET_ARCH)-cavan-linux-$(CAVAN_TARGET_EABI)

WORK_PATH = $(HOME)/cavan-toolchian
SRC_PATH = $(WORK_PATH)/src
UTILS_PATH = $(WORK_PATH)/utils
DECOMP_PATH = $(WORK_PATH)/decomp
MARK_PATH = $(WORK_PATH)/mark
OUT_PATH = $(WORK_PATH)/out

CAVAN_HOST_ARCH = $(CAVAN_BUILD_ARCH)
TOOLCHIAN_PREFIX = $(WORK_PATH)/toolchian

ifeq ($(CAVAN_HOST_ARCH),$(CAVAN_TARGET_ARCH))
TOOLCHIAN_NAME = $(CAVAN_TARGET_PLAT)-$(GCC_VERSION)
else
TOOLCHIAN_NAME = $(CAVAN_TARGET_PLAT)-$(GCC_VERSION)-$(CAVAN_HOST_ARCH)
endif

ifneq ($(CAVAN_TARGET_MX32),)
TOOLCHIAN_NAME := $(TOOLCHIAN_NAME)-$(CAVAN_TARGET_MX32)
endif

ifneq ($(filter amd64 x86_64,$(CAVAN_HOST_ARCH)),)
CAVAN_HOST_ARCH_32 = i686
endif

TOOLCHIAN_PATH = $(TOOLCHIAN_PREFIX)/$(TOOLCHIAN_NAME)
SYSROOT_PATH = $(TOOLCHIAN_PATH)/sysroot

ROOTFS_PATH = $(WORK_PATH)/rootfs/$(CAVAN_TARGET_ARCH)
OUT_ROOTFS = $(OUT_PATH)/rootfs/$(CAVAN_TARGET_ARCH)
OUT_UTILS = $(OUT_PATH)/utils
MARK_ROOTFS = $(MARK_PATH)/rootfs/$(CAVAN_TARGET_ARCH)
MARK_UTILS = $(MARK_PATH)/utils

BUILD_CORE = $(BUILD_PATH)/core
BUILD_TOOLCHIAN = $(BUILD_PATH)/toolchian
BUILD_ROOTFS = $(BUILD_PATH)/rootfs
BUILD_UTILS = $(BUILD_PATH)/utils

MARK_ROOTFS_READY = $(MARK_ROOTFS)/ready
MARK_UTILS_READY1 = $(MARK_UTILS)/ready1
MARK_UTILS_READY2 = $(MARK_UTILS)/ready2

MAKEFILE_TOOLCHIAN = $(BUILD_TOOLCHIAN)/main.mk
MAKEFILE_ROOTFS = $(BUILD_ROOTFS)/main.mk
MAKEFILE_UTILS = $(BUILD_UTILS)/main.mk
MAKEFILE_DEFINES = $(BUILD_CORE)/defines.mk

PYTHON_PARSER = $(SCRIPT_PATH)/parser.py
BASH_DOWNLOAD = $(SCRIPT_PATH)/download.sh

MARK_HOST_APPS = $(MARK_UTILS)/environment

ifeq ($(CAVAN_HOST_ARCH),$(CAVAN_BUILD_ARCH))
define export_path
$(foreach path,/ /usr/ /usr/local/,$(eval export PATH := $1$(path)sbin:$1$(path)bin:$(PATH)))
endef
else
define export_path
$(warning export path $1, nothing to be done)
endef
endif

$(call export_path,$(UTILS_PATH))
$(call export_path,$(TOOLCHIAN_PATH))

DOWNLOAD_COMMAND = bash $(BASH_DOWNLOAD)

export CAVAN_TARGET_PLAT ROOT_PATH PACKAGE_PATH BUILD_PATH PATCH_PATH SCRIPT_PATH
export SRC_PATH ROOTFS_PATH UTILS_PATH DECOMP_PATH PATH DOWNLOAD_PATH
export OUT_PATH OUT_UTILS OUT_ROOTFS
export BUILD_CORE BUILD_TOOLCHIAN BUILD_ROOTFS BUILD_UTILS
export MARK_PATH MARK_ROOTFS MARK_UTILS
export MARK_ROOTFS_READY MARK_UTILS_READY1 MARK_UTILS_READY2
export MAKEFILE_DEFINES MAKEFILE_TOOLCHIAN
export PYTHON_PARSER XML_APPLICATION BASH_DOWNLOAD DOWNLOAD_COMMAND
export TOOLCHIAN_PREFIX TOOLCHIAN_NAME TOOLCHIAN_PATH SYSROOT_PATH
export CAVAN_HOST_ARCH CAVAN_HOST_ARCH_32

$(info ============================================================)
$(info PACKAGE_PATH = $(PACKAGE_PATH))
$(info PATCH_PATH = $(PATCH_PATH))
$(info DOWNLOAD_PATH = $(DOWNLOAD_PATH))
$(info ============================================================)

include $(MAKEFILE_DEFINES)

$(CAVAN_TARGET_EABI) glibc: $(MARK_UTILS_READY1)
	$(Q)+make -f $(MAKEFILE_TOOLCHIAN) $@

rootfs: $(MARK_ROOTFS_READY)
	$(Q)echo "$@ compile successfull"

rootfs-public:
	$(call remake_device_node,$(ROOTFS_PATH)/dev/console,c,5,1)
	$(Q)sudo chown root:root $(ROOTFS_PATH) -R

utils: $(MARK_UTILS_READY1) $(MARK_UTILS_READY2)
	$(Q)echo "$@ compile successfull"

environment: build_env $(MARK_HOST_APPS)
	$(Q)echo "$@ install successfull"

$(MARK_HOST_APPS):
	$(Q)for pkg in $(HOST_APPS); \
	do \
		echo "$(APT_GET) $${pkg}"; \
		$(APT_GET) $${pkg}; \
	done
	$(call generate_mark)

$(MARK_ROOTFS_READY): $(MARK_UTILS_READY2)
	$(Q)+make -f $(MAKEFILE_ROOTFS)

$(MARK_UTILS_READY1): build_env
	$(Q)+make -f $(MAKEFILE_UTILS) $@

$(MARK_UTILS_READY2): $(CAVAN_TARGET_EABI)
	$(Q)+make -f $(MAKEFILE_UTILS) $@

clean:
	$(call remove_files,$(DECOMP_PATH) $(SRC_PATH) $(OUT_PATH))

distclean: clean
	$(call remove_files,$(TOOLCHIAN_PREFIX) $(MARK_PATH) $(UTILS_PATH) $(WORK_PATH))

build_env:
	$(Q)if test -L $(WORK_PATH) -o ! -d $(WORK_PATH); \
	then \
		rm $(WORK_PATH) -rfv  && mkdir $(WORK_PATH) -pv; \
	fi
	$(Q)mkdir $(SRC_PATH) $(UTILS_PATH) $(OUT_UTILS) $(OUT_ROOTFS) $(DECOMP_PATH) -pv
	$(Q)[ -d "$(DOWNLOAD_PATH)" ] || mkdir -pv "$(DOWNLOAD_PATH)"
	$(Q)mkdir $(MARK_ROOTFS) $(MARK_UTILS) -pv

$(addprefix gcc-,$(GCC_VERSION_LIST)):
	$(Q)+make GCC_VERSION=$(patsubst gcc-%,%,$@)

$(addprefix android-gcc-,$(GCC_VERSION_LIST)):
	$(Q)+make GCC_VERSION=$(patsubst android-gcc-%,%,$@) CAVAN_TARGET_EABI=androideabi

$(addprefix host-gcc-,$(GCC_VERSION_LIST)):
	$(Q)+make GCC_VERSION=$(patsubst host-gcc-%,%,$@) CAVAN_TARGET_ARCH=$(CAVAN_BUILD_ARCH)

ifneq ($(CAVAN_HOST_ARCH_32),)
$(patsubst %,host-gcc-%-32,$(GCC_VERSION_LIST)):
	$(Q)+make GCC_VERSION=$(patsubst host-gcc-%-32,%,$@) CAVAN_TARGET_ARCH=$(CAVAN_HOST_ARCH_32) CAVAN_HOST_ARCH=$(CAVAN_HOST_ARCH_32)
endif

ifneq ($(filter x86_64 amd64,$(CAVAN_BUILD_ARCH)),)
$(patsubst %,host-gcc-%-m32,$(GCC_VERSION_LIST)):
	$(Q)+make GCC_VERSION=$(patsubst host-gcc-%-m32,%,$@) CAVAN_TARGET_ARCH=$(CAVAN_BUILD_ARCH) CAVAN_TARGET_MX32=mx32
endif

$(addprefix target-gcc-,$(GCC_VERSION_LIST)):
	$(Q)+make GCC_VERSION=$(patsubst target-gcc-%,%,$@) CAVAN_HOST_ARCH=$(CAVAN_TARGET_ARCH)

.PHONY: build_env $(CAVAN_TARGET_EABI)
