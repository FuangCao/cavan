ifeq ($(CAVAN_HOST_ARCH),$(CAVAN_BUILD_ARCH))
CAVAN_HOST_PLAT = $(CAVAN_BUILD_PLAT)
TOOLCHIAN_PATH = $(TOOLCHIAN_BT_PATH)
OUT_TOOLCHIAN = $(OUT_TOOLCHIAN_BT)
MARK_TOOLCHIAN = $(MARK_TOOLCHIAN_BT)
MARK_TOOLCHIAN_READY = $(MARK_TOOLCHIAN_BT_READY)
else
CAVAN_HOST_PLAT = $(CAVAN_TARGET_PLAT)
TOOLCHIAN_PATH = $(TOOLCHIAN_TT_PATH)
OUT_TOOLCHIAN = $(OUT_TOOLCHIAN_TT)
MARK_TOOLCHIAN = $(MARK_TOOLCHIAN_TT)
MARK_TOOLCHIAN_READY = $(MARK_TOOLCHIAN_TT_READY)
endif

BINUTILS_NAME = binutils-$(BINUTILS_VERSION)
GCC_NAME = gcc-$(GCC_VERSION)
HEADER_NAME = header
GLIBC_NAME = glibc-$(GLIBC_VERSION)
GLIBC_PORTS_NAME = glibc-ports-$(GLIBC_VERSION)
KERNEL_NAME = linux-$(KERNEL_VERSION)
GMP_NAME = gmp-$(GMP_VERSION)
MPFR_NAME = mpfr-$(MPFR_VERSION)
MPC_NAME = mpc-$(MPC_VERSION)

SRC_BINUTILS = $(SRC_PATH)/$(BINUTILS_NAME)
SRC_GCC = $(SRC_PATH)/$(GCC_NAME)
SRC_GLIBC = $(SRC_PATH)/$(GLIBC_NAME)
SRC_KERNEL = $(SRC_PATH)/$(KERNEL_NAME)
SRC_GMP = $(SRC_PATH)/$(GMP_NAME)
SRC_MPFR = $(SRC_PATH)/$(MPFR_NAME)
SRC_MPC = $(SRC_PATH)/$(MPC_NAME)

OUT_BINUTILS = $(OUT_TOOLCHIAN)/$(BINUTILS_NAME)
OUT_GCC1 = $(OUT_TOOLCHIAN)/$(GCC_NAME)-stage1
OUT_GCC2 = $(OUT_TOOLCHIAN)/$(GCC_NAME)-stage2
OUT_GLIBC = $(OUT_TOOLCHIAN)/$(GLIBC_NAME)

MARK_BINUTILS = $(MARK_TOOLCHIAN)/$(BINUTILS_NAME)
MARK_GCC1 = $(MARK_TOOLCHIAN)/$(GCC_NAME)-stage1
MARK_GCC2 = $(MARK_TOOLCHIAN)/$(GCC_NAME)-stage2
MARK_GLIBC = $(MARK_TOOLCHIAN)/$(GLIBC_NAME)
MARK_HEADER = $(MARK_TOOLCHIAN)/$(HEADER_NAME)

MAKEFILE_BINUTILS = $(call find_makefile,$(BUILD_TOOLCHIAN)/$(BINUTILS_NAME).mk $(BUILD_TOOLCHIAN)/binutils.mk)
MAKEFILE_GCC = $(call find_makefile,$(BUILD_TOOLCHIAN)/$(GCC_NAME).mk $(BUILD_TOOLCHIAN)/gcc.mk)
MAKEFILE_GLIBC = $(call find_makefile,$(BUILD_TOOLCHIAN)/$(GLIBC_NAME).mk $(BUILD_TOOLCHIAN)/glibc.mk)
MAKEFILE_HEADER = $(BUILD_TOOLCHIAN)/$(HEADER_NAME).mk
XML_CONFIG = $(BUILD_TOOLCHIAN)/config.xml

GCC_URL = http://ftp.gnu.org/gnu/gcc/$(GCC_NAME)
GLIBC_URL = http://ftp.gnu.org/gnu/glibc
BINUTILS_URL = http://ftp.gnu.org/gnu/binutils
GMP_URL = http://ftp.gnu.org/gnu/gmp
MPFR_URL = http://ftp.gnu.org/gnu/mpfr
MPC_URL = http://www.multiprecision.org/mpc/download
KERNEL_URL = http://www.kernel.org/pub/linux/kernel/v3.0

SYSROOT_PATH = $(TOOLCHIAN_PATH)/sysroot
TOOLCHIAN_COMMON_CONFIG = --prefix=$(TOOLCHIAN_PATH) --build=$(CAVAN_BUILD_PLAT) --host=$(CAVAN_HOST_PLAT) --target=$(CAVAN_TARGET_PLAT)
LIBRARY_COMMON_CONFIG = --prefix=/usr --build=$(CAVAN_BUILD_PLAT) --host=$(CAVAN_TARGET_PLAT)

ifneq ($(CAVAN_BUILD_ARCH),$(CAVAN_TARGET_ARCH))
TOOLCHIAN_COMMON_CONFIG += --with-sysroot=$(SYSROOT_PATH)
endif

export GCC_NAME SRC_BINUTILS SRC_GCC SRC_KERNEL SRC_GLIBC
export SYSROOT_PATH TOOLCHIAN_COMMON_CONFIG LIBRARY_COMMON_CONFIG
export CAVAN_HOST_ARCH CAVAN_HOST_PLAT
export TOOLCHIAN_PATH OUT_TOOLCHIAN MARK_TOOLCHIAN MARK_TOOLCHIAN_READY

$(info ============================================================)
$(info CAVAN_HOST_ARCH = $(CAVAN_HOST_ARCH))
$(info CAVAN_HOST_PLAT = $(CAVAN_HOST_PLAT))
$(info TOOLCHIAN_PATH = $(TOOLCHIAN_PATH))
$(info SYSROOT_PATH = $(SYSROOT_PATH))
$(info OUT_TOOLCHIAN = $(OUT_TOOLCHIAN))
$(info MARK_TOOLCHIAN = $(MARK_TOOLCHIAN))
$(info MARK_TOOLCHIAN_READY = $(MARK_TOOLCHIAN_READY))
$(info ============================================================)

include $(MAKEFILE_DEFINES)

ifeq ($(filter $(GLIBC_VERSION),2.17 2.18),)
define decompression_glibc
if ! test -d "$(SRC_GLIBC)"; \
then \
	$(call simple_decompression_file,$(GLIBC_NAME),$(SRC_GLIBC),$(GLIBC_URL)); \
	$(call simple_decompression_file,$(GLIBC_PORTS_NAME),$(SRC_GLIBC)/ports,$(GLIBC_URL)); \
	$(call apply_patchs,$(GLIBC_NAME),$(SRC_GLIBC)); \
fi
endef
else
define decompression_glibc
if ! test -d "$(SRC_GLIBC)"; \
then \
	$(call simple_decompression_file,$(GLIBC_NAME),$(SRC_GLIBC),$(GLIBC_URL)); \
	$(call apply_patchs,$(GLIBC_NAME),$(SRC_GLIBC)); \
fi
endef
endif

define decompression_gcc
if ! test -d $(SRC_GCC); \
then \
	$(call simple_decompression_file,$(GCC_NAME),$(SRC_GCC),$(GCC_URL)); \
	$(call simple_decompression_file,$(GMP_NAME),$(SRC_GCC)/gmp,$(GMP_URL)); \
	$(call simple_decompression_file,$(MPFR_NAME),$(SRC_GCC)/mpfr,$(MPFR_URL)); \
	$(call simple_decompression_file,$(MPC_NAME),$(SRC_GCC)/mpc,$(MPC_URL)); \
	$(call apply_patchs,$(GCC_NAME),$(SRC_GCC)); \
	$(call apply_patchs,$(GMP_NAME),$(SRC_GCC)/gmp); \
	$(call apply_patchs,$(MPFR_NAME),$(SRC_GCC)/mpfr); \
	$(call apply_patchs,$(MPC_NAME),$(SRC_GCC)/mpc); \
	sed -i 's,\s*\(const\s\+char\s\+pkgversion_string.*=\).*;\s*$$,\1 "[$(PACKAGE_VERSION_STRING)] ";,g' $(SRC_GCC)/gcc/version.c; \
fi
endef

all: $(MARK_TOOLCHIAN_READY)
	$(Q)echo "Toolchian compile successfull"

ifeq ($(CAVAN_BUILD_ARCH),$(CAVAN_TARGET_ARCH))
$(MARK_TOOLCHIAN_READY): $(MARK_GLIBC)
else
$(MARK_TOOLCHIAN_READY): $(MARK_GCC2)
endif
	$(Q)cd $(TOOLCHIAN_PATH)/bin && for tool in $(CAVAN_TARGET_PLAT)-*; \
	do \
		ln -vsf "$${tool}" "$(CAVAN_TARGET_ARCH)-linux$${tool##$(CAVAN_TARGET_PLAT)}"; \
		ln -vsf "$${tool}" "$(CAVAN_TARGET_ARCH)-eabi$${tool##$(CAVAN_TARGET_PLAT)}"; \
	done
ifeq ($(CAVAN_HOST_ARCH),$(CAVAN_BUILD_ARCH))
	$(Q)sed 's/\<__packed\>/__attribute__ ((__packed__))/g' $(SYSROOT_PATH)/usr/include/mtd/ubi-user.h -i
	$(call auto_make,install_library,$(MARK_TOOLCHIAN),$(OUT_TOOLCHIAN),$(XML_CONFIG))
else
	$(call generate_mark)
endif

ifeq ($(CAVAN_BUILD_ARCH),$(CAVAN_TARGET_ARCH))
$(MARK_GCC2): $(MARK_BINUTILS)
	$(call decompression_gcc)
	$(call remake_directory,$(OUT_GCC2))
	$(Q)+make -C $(OUT_GCC2) -f $(MAKEFILE_GCC) stage2
	$(call generate_mark)

$(MARK_GLIBC): $(MARK_GCC2)
	$(call decompression_glibc,$(SRC_GLIBC))
	$(call remake_directory,$(OUT_GLIBC))
	$(Q)+make -C $(OUT_GLIBC) -f $(MAKEFILE_GLIBC)
	$(call generate_mark)
else
$(MARK_GCC2): $(MARK_GLIBC)
	$(call decompression_gcc)
	$(call remake_directory,$(OUT_GCC2))
	$(Q)+make AS_FOR_TARGET="$(CAVAN_TARGET_PLAT)-as" LD_FOR_TARGET="$(CAVAN_TARGET_PLAT)-ld" -C $(OUT_GCC2) -f $(MAKEFILE_GCC) stage2
	$(call generate_mark)

ifeq ($(CAVAN_HOST_ARCH),$(CAVAN_BUILD_ARCH))
$(MARK_GLIBC): $(MARK_GCC1)
	$(call decompression_glibc,$(SRC_GLIBC))
	$(call remake_directory,$(OUT_GLIBC))
	$(Q)+make -C $(OUT_GLIBC) -f $(MAKEFILE_GLIBC)
	$(call generate_mark)

$(MARK_GCC1): $(MARK_BINUTILS)
	$(call decompression_gcc)
	$(call remake_directory,$(OUT_GCC1))
	$(Q)+make -C $(OUT_GCC1) -f $(MAKEFILE_GCC) stage1
	$(call generate_mark)
else
$(MARK_GLIBC): $(MARK_BINUTILS)
	$(Q)echo "Nothing to be done"
	$(call generate_mark)
endif
endif

$(MARK_BINUTILS): $(MARK_HEADER)
	$(call decompression_file,$(SRC_BINUTILS),$(BINUTILS_URL))
	$(call remake_directory,$(OUT_BINUTILS))
	$(Q)+make -C $(OUT_BINUTILS) -f $(MAKEFILE_BINUTILS)
	$(call generate_mark)

$(MARK_HEADER):
ifeq ($(CAVAN_HOST_ARCH),$(CAVAN_BUILD_ARCH))
	$(call decompression_file,$(SRC_KERNEL),$(KERNEL_URL))
	$(Q)+make -C $(SRC_KERNEL) -f $(MAKEFILE_HEADER)
else
	$(Q)rm $(SYSROOT_PATH) -rfv
	$(Q)cp $(SYSROOT_BT_PATH) $(SYSROOT_PATH) -av
endif
	$(call generate_mark)
