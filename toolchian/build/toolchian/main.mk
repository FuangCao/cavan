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
OUT_GCC1 = $(OUT_TOOLCHIAN)/$(GCC_NAME)-1
OUT_GCC2 = $(OUT_TOOLCHIAN)/$(GCC_NAME)-2
OUT_GLIBC = $(OUT_TOOLCHIAN)/$(GLIBC_NAME)

MARK_BINUTILS = $(MARK_TOOLCHIAN)/$(BINUTILS_NAME)
MARK_GCC1 = $(MARK_TOOLCHIAN)/$(GCC_NAME)-1
MARK_GCC2 = $(MARK_TOOLCHIAN)/$(GCC_NAME)-2
MARK_GLIBC = $(MARK_TOOLCHIAN)/$(GLIBC_NAME)
MARK_HEADER = $(MARK_TOOLCHIAN)/$(HEADER_NAME)

MAKEFILE_BINUTILS = $(BUILD_TOOLCHIAN)/$(BINUTILS_NAME).mk
MAKEFILE_GCC = $(BUILD_TOOLCHIAN)/$(GCC_NAME).mk
MAKEFILE_GLIBC = $(BUILD_TOOLCHIAN)/$(GLIBC_NAME).mk
MAKEFILE_HEADER = $(BUILD_TOOLCHIAN)/$(HEADER_NAME).mk
MAKEFILE_TOOLCHIAN_RULE = $(BUILD_TOOLCHIAN)/rule.mk

GCC_URL = http://ftp.gnu.org/gnu/gcc
GLIBC_URL = http://ftp.gnu.org/gnu/glibc
BINUTILS_URL = http://ftp.gnu.org/gnu/binutils
GMP_URL = http://ftp.gnu.org/gnu/gmp
MPFR_URL = http://ftp.gnu.org/gnu/mpfr
MPC_URL = http://www.multiprecision.org/mpc/download
KERNEL_URL = http://down1.chinaunix.net/distfiles/$(KERNEL_NAME).tar.bz2

export GCC_NAME SRC_BINUTILS SRC_GCC SRC_KERNEL SRC_GLIBC
export MAKEFILE_TOOLCHIAN_RULE

include $(MAKEFILE_DEFINES)

define decompression_glibc
if ! test -d "$(SRC_GLIBC)"; \
then \
	$(call simple_decompression_file,$(GLIBC_NAME),$(SRC_GLIBC),$(GLIBC_URL)); \
	$(call simple_decompression_file,$(GLIBC_PORTS_NAME),$(SRC_GLIBC)/ports,$(GLIBC_URL)); \
	$(call apply_patchs,$(GLIBC_NAME),$(SRC_GLIBC)); \
fi
endef

define decompression_gcc
if ! test -d $(SRC_GCC); \
then \
	$(call simple_decompression_file,$(GCC_NAME),$(SRC_GCC),$(GCC_URL)); \
	$(call simple_decompression_file,$(GMP_NAME),$(SRC_GCC)/gmp,$(GMP_URL)); \
	$(call simple_decompression_file,$(MPFR_NAME),$(SRC_GCC)/mpfr,$(MPFR_URL)); \
	$(call simple_decompression_file,$(MPC_NAME),$(SRC_GCC)/mpc,$(MPC_URL)); \
	$(call apply_patchs,$(GCC_NAME),$(SRC_GCC)); \
	sed -i 's,\s*\(const\s\+char\s\+pkgversion_string.*=\).*;\s*$$,\1 "[$(PACKAGE_VERSION_STRING)] ";,g' $(SRC_GCC)/gcc/version.c; \
fi
endef

all: $(MARK_TOOLCHIAN_READY)
	$(Q)echo "Toolchian compile successfull"

$(MARK_TOOLCHIAN_READY): $(MARK_GCC2)
	$(Q)sed 's/\<__packed\>/__attribute__ ((__packed__))/g' $(SYSROOT_PATH)/usr/include/mtd/ubi-user.h -i
	$(call generate_mark)

$(MARK_GCC2): $(MARK_GLIBC)
	$(call decompression_gcc)
	$(call remake_directory,$(OUT_GCC2))
	$(Q)+make -C $(OUT_GCC2) -f $(MAKEFILE_GCC) $(GCC_NAME)-2
	$(call generate_mark)

$(MARK_GLIBC): $(MARK_GCC1)
	$(call decompression_glibc,$(SRC_GLIBC))
	$(call remake_directory,$(OUT_GLIBC))
	$(Q)+make -C $(OUT_GLIBC) -f $(MAKEFILE_GLIBC)
	$(call generate_mark)

$(MARK_GCC1): $(MARK_BINUTILS)
	$(call decompression_gcc)
	$(call remake_directory,$(OUT_GCC1))
	$(Q)+make -C $(OUT_GCC1) -f $(MAKEFILE_GCC) $(GCC_NAME)-1
	$(call generate_mark)

$(MARK_BINUTILS): $(MARK_HEADER)
	$(call decompression_file,$(SRC_BINUTILS),$(BINUTILS_URL))
	$(call remake_directory,$(OUT_BINUTILS))
	$(Q)+make -C $(OUT_BINUTILS) -f $(MAKEFILE_BINUTILS)
	$(call generate_mark)

$(MARK_HEADER):
	$(call decompression_file,$(SRC_KERNEL),$(KERNEL_URL))
	$(Q)+make -C $(SRC_KERNEL) -f $(MAKEFILE_HEADER)
	$(call generate_mark)
