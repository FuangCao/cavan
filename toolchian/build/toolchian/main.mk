ifeq ($(CAVAN_HOST_ARCH),$(CAVAN_TARGET_ARCH))
CAVAN_HOST_PLAT = $(CAVAN_TARGET_PLAT)
else
CAVAN_HOST_PLAT = $(CAVAN_BUILD_PLAT)
endif

ifeq ($(CAVAN_BUILD_ARCH),$(CAVAN_TARGET_ARCH))
CAVAN_HOST_PLAT = $(CAVAN_BUILD_PLAT)
endif

CPU_BINUTILS_OPTION =
CPU_GCC_OPTION =
GLIBC_LIB_NAME = lib

ifeq ($(CAVAN_TARGET_ARCH),arm)
CPU_BINUTILS_OPTION += --with-float=soft
CPU_GCC_OPTION += --with-arch=armv5te --with-float=soft --with-fpu=vfp --with-abi=aapcs
endif

ifneq ($(filter amd64 x86_64,$(CAVAN_TARGET_ARCH)),)
ifeq ($(CAVAN_HOST_ARCH_32),)
$(error you must defind CAVAN_HOST_ARCH_32)
endif
ifneq ($(CAVAN_TARGET_MX32),)
CPU_GCC_OPTION += --with-abi=$(CAVAN_TARGET_MX32)
else
CPU_GCC_OPTION += --enable-multiarch --with-arch-32=$(CAVAN_HOST_ARCH_32) --with-abi=m64 --with-multilib-list=m32,m64 --with-tune=generic
endif
endif

PACKAGE_VERSION_STRING = Fuang.Cao <cavan.cfa@gmail.com>

OUT_TOOLCHIAN = $(OUT_PATH)/toolchian/$(TOOLCHIAN_NAME)
MARK_TOOLCHIAN = $(MARK_PATH)/toolchian/$(TOOLCHIAN_NAME)
MARK_TOOLCHIAN_READY = $(MARK_TOOLCHIAN)/ready

$(info ============================================================)
$(info KERNEL_VERSION = $(KERNEL_VERSION))
$(info BINUTILS_VERSION = $(BINUTILS_VERSION))
$(info GCC_VERSION = $(GCC_VERSION))
$(info GLIBC_VERSION = $(GLIBC_VERSION))
$(info GMP_VERSION = $(GMP_VERSION))
$(info MPC_VERSION = $(MPC_VERSION))
$(info MPFR_VERSION = $(MPFR_VERSION))
$(info CAVAN_BUILD_ARCH = $(CAVAN_BUILD_ARCH))
$(info CAVAN_BUILD_PLAT = $(CAVAN_BUILD_PLAT))
$(info CAVAN_HOST_ARCH = $(CAVAN_HOST_ARCH))
$(info CAVAN_HOST_PLAT = $(CAVAN_HOST_PLAT))
$(info CAVAN_TARGET_ARCH = $(CAVAN_TARGET_ARCH))
$(info CAVAN_TARGET_PLAT = $(CAVAN_TARGET_PLAT))
$(info ============================================================)

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

ifeq ($(GLIBC_LIB_NAME),lib)
MARK_GLIBC = $(MARK_TOOLCHIAN)/$(GLIBC_NAME)
else
MARK_GLIBC = $(MARK_TOOLCHIAN)/$(GLIBC_NAME)-$(GLIBC_LIB_NAME)
endif

MARK_HEADER = $(MARK_TOOLCHIAN)/$(HEADER_NAME)

MAKEFILE_BINUTILS = $(call find_makefile,$(BUILD_TOOLCHIAN)/$(BINUTILS_NAME).mk $(BUILD_TOOLCHIAN)/binutils.mk)
MAKEFILE_GCC = $(call find_makefile,$(BUILD_TOOLCHIAN)/$(GCC_NAME).mk $(BUILD_TOOLCHIAN)/gcc.mk)
MAKEFILE_GLIBC = $(call find_makefile,$(BUILD_TOOLCHIAN)/$(GLIBC_NAME).mk $(BUILD_TOOLCHIAN)/glibc.mk)
MAKEFILE_HEADER = $(BUILD_TOOLCHIAN)/$(HEADER_NAME).mk
XML_CONFIG = $(BUILD_TOOLCHIAN)/config.xml

GCC_URL = http://mirrors.ustc.edu.cn/gnu/gcc/$(GCC_NAME) http://ftp.tsukuba.wide.ad.jp/software/gcc/releases/$(GCC_NAME) http://ftp.gnu.org/gnu/gcc/$(GCC_NAME)
GLIBC_URL = http://mirrors.ustc.edu.cn/gnu/glibc http://ftp.gnu.org/gnu/glibc
BINUTILS_URL = http://mirrors.ustc.edu.cn/gnu/binutils http://ftp.gnu.org/gnu/binutils
GMP_URL = http://mirrors.ustc.edu.cn/gnu/gmp http://ftp.gnu.org/gnu/gmp
MPFR_URL = http://mirrors.ustc.edu.cn/gnu/mpfr http://ftp.gnu.org/gnu/mpfr
MPC_URL = http://mirrors.ustc.edu.cn/gnu/mpc http://www.multiprecision.org/mpc/download
KERNEL_URL = http://www.kernel.org/pub/linux/kernel/v3.0

SYSROOT_PATH = $(TOOLCHIAN_PATH)/sysroot
TOOLCHIAN_COMMON_CONFIG = --build=$(CAVAN_BUILD_PLAT) --host=$(CAVAN_HOST_PLAT) --target=$(CAVAN_TARGET_PLAT)
TOOLCHIAN_COMMON_CONFIG += --prefix=$(TOOLCHIAN_PATH) --with-sysroot=$(SYSROOT_PATH) --with-pkgversion="Fuang.Cao"
LIBRARY_COMMON_CONFIG = --prefix=/usr --build=$(CAVAN_BUILD_PLAT) --host=$(CAVAN_TARGET_PLAT)

CAVAN_TOOLCHIAN_PREFIXS = $(CAVAN_TARGET_ARCH)-linux-$(CAVAN_TARGET_EABI)

ifeq ($(CAVAN_TARGET_EABI),androideabi)
ifneq ($(filter 4.5% 4.4% 4.3% 4.2% 4.1% 4.0%,$(GCC_VERSION)),)
$(error $(GCC_NAME) do not support androideabi)
endif
else
CAVAN_TOOLCHIAN_PREFIXS += $(CAVAN_TARGET_ARCH)-linux $(CAVAN_TARGET_ARCH)-eabi
endif

export GCC_NAME SRC_BINUTILS SRC_GCC SRC_KERNEL SRC_GLIBC
export SYSROOT_PATH TOOLCHIAN_COMMON_CONFIG LIBRARY_COMMON_CONFIG
export CAVAN_HOST_ARCH CAVAN_HOST_PLAT GLIBC_LIB_NAME
export TOOLCHIAN_PATH OUT_TOOLCHIAN MARK_TOOLCHIAN MARK_TOOLCHIAN_READY
export CPU_GCC_OPTION CPU_BINUTILS_OPTION PACKAGE_VERSION_STRING

$(info ============================================================)
$(info GLIBC_LIB_NAME = $(GLIBC_LIB_NAME))
$(info TOOLCHIAN_PATH = $(TOOLCHIAN_PATH))
$(info SYSROOT_PATH = $(SYSROOT_PATH))
$(info OUT_TOOLCHIAN = $(OUT_TOOLCHIAN))
$(info MARK_TOOLCHIAN = $(MARK_TOOLCHIAN))
$(info MARK_TOOLCHIAN_READY = $(MARK_TOOLCHIAN_READY))
$(info CPU_BINUTILS_OPTION = $(CPU_BINUTILS_OPTION))
$(info CPU_GCC_OPTION = $(CPU_GCC_OPTION))
$(info CAVAN_TOOLCHIAN_PREFIXS = $(CAVAN_TOOLCHIAN_PREFIXS))
$(info ============================================================)

include $(MAKEFILE_DEFINES)

ifeq ($(filter 2.17 2.18 2.19 2.2%,$(GLIBC_VERSION)),)
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
[ -d "$(SRC_GMP)" ] && [ -d "$(SRC_GCC)/gmp" ] && [ -d "$(SRC_GCC)/mpfr" ] && [ -d "$(SRC_GCC)/mpc" ] || \
{ \
	[ -d "$(SRC_GCC)" ] || $(call simple_decompression_file,$(GCC_NAME),$(SRC_GCC),$(GCC_URL)); \
	[ -d "$(SRC_GCC)/gmp" ] || $(call simple_decompression_file,$(GMP_NAME),$(SRC_GCC)/gmp,$(GMP_URL)); \
	[ -d "$(SRC_GCC)/mpfr" ] || $(call simple_decompression_file,$(MPFR_NAME),$(SRC_GCC)/mpfr,$(MPFR_URL)); \
	[ -d "$(SRC_GCC)/mpc" ] || $(call simple_decompression_file,$(MPC_NAME),$(SRC_GCC)/mpc,$(MPC_URL)); \
	$(call apply_patchs,$(GCC_NAME),$(SRC_GCC)); \
	$(call apply_patchs,$(GMP_NAME),$(SRC_GCC)/gmp); \
	$(call apply_patchs,$(MPFR_NAME),$(SRC_GCC)/mpfr); \
	$(call apply_patchs,$(MPC_NAME),$(SRC_GCC)/mpc); \
	sed -i '/k prot/agcc_cv_libc_provides_ssp=yes' $(SRC_GCC)/gcc/configure; \
	sed -i 's/if \((code.*))\)/if (\1 \&\& \!DEBUG_INSN_P (insn))/' $(SRC_GCC)/gcc/sched-deps.c; \
	sed -i 's,\s*\(const\s\+char\s\+pkgversion_string.*=\).*;\s*$$,\1 "[$(PACKAGE_VERSION_STRING)] ";,g' $(SRC_GCC)/gcc/version.c; \
}
endef

$(CAVAN_TARGET_EABI): $(MARK_TOOLCHIAN_READY)
	$(Q)echo "$@ compile successfull"

glibc: $(MARK_GLIBC)
	$(Q)echo "$@ compile successfull"

$(MARK_TOOLCHIAN_READY): $(MARK_GCC2)
	$(Q)cd $(TOOLCHIAN_PATH)/bin && for tool in $(CAVAN_TARGET_PLAT)-*; \
	do \
		suffix="$${tool##$(CAVAN_TARGET_PLAT)}"; \
		for prefix in $(CAVAN_TOOLCHIAN_PREFIXS); \
		do \
			ln -vsf "$${tool}" "$${prefix}$${suffix}"; \
		done; \
	done
ifneq ($(CAVAN_TARGET_EABI),androideabi)
ifeq ($(CAVAN_BUILD_ARCH),$(CAVAN_HOST_ARCH))
	$(Q)sed 's/\<__packed\>/__attribute__ ((__packed__))/g' $(SYSROOT_PATH)/usr/include/mtd/ubi-user.h -i
	$(call auto_make,install_library,$(MARK_TOOLCHIAN),$(OUT_TOOLCHIAN),$(XML_CONFIG))
endif
endif
	$(call generate_mark)

ifeq ($(CAVAN_BUILD_ARCH),$(CAVAN_HOST_ARCH))
ifeq ($(CAVAN_TARGET_EABI),androideabi)
$(MARK_GCC2): $(MARK_BINUTILS)
else
$(MARK_GCC2): $(MARK_GLIBC)
endif
	$(call decompression_gcc)
	$(call remake_directory,$(OUT_GCC2))
	$(Q)+make AS_FOR_TARGET="$(CAVAN_TARGET_PLAT)-as" LD_FOR_TARGET="$(CAVAN_TARGET_PLAT)-ld" -C $(OUT_GCC2) -f $(MAKEFILE_GCC) stage2
	$(call generate_mark)

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
$(MARK_GCC2): $(MARK_BINUTILS)
	$(call decompression_gcc)
	$(call remake_directory,$(OUT_GCC2))
	$(Q)+make -C $(OUT_GCC2) -f $(MAKEFILE_GCC) stage2
	$(call generate_mark)
endif

$(MARK_BINUTILS): $(MARK_HEADER)
	$(call decompression_file,$(SRC_BINUTILS),$(BINUTILS_URL))
	$(call remake_directory,$(OUT_BINUTILS))
	$(Q)+make -C $(OUT_BINUTILS) -f $(MAKEFILE_BINUTILS)
	$(call generate_mark)

$(MARK_HEADER): | $(OUT_TOOLCHIAN) $(MARK_TOOLCHIAN) $(TOOLCHIAN_PATH)
ifeq ($(CAVAN_TARGET_EABI),androideabi)
	$(Q)+make CAVAN_TARGET_EABI="gnueabi" glibc
	$(Q)rm -rf "$(SYSROOT_PATH)" && cp -av "$(subst androideabi,gnueabi,$(SYSROOT_PATH))" "$(SYSROOT_PATH)"
else
ifeq ($(CAVAN_BUILD_ARCH),$(CAVAN_HOST_ARCH))
ifeq ($(CAVAN_HOST_ARCH),$(CAVAN_TARGET_ARCH))
ifneq ($(CAVAN_HOST_ARCH_32),)
ifeq ($(CAVAN_TARGET_MX32),)
	$(Q)rm -rf "$(SYSROOT_PATH)"
	$(Q)+make CAVAN_TARGET_ARCH=$(CAVAN_HOST_ARCH_32) GLIBC_LIB_NAME=lib32 glibc
	$(Q)cp -av "$(patsubst %/$(TOOLCHIAN_NAME)/sysroot,%/$(patsubst $(CAVAN_TARGET_ARCH)-%,$(CAVAN_HOST_ARCH_32)-%-$(CAVAN_BUILD_ARCH),$(TOOLCHIAN_NAME))/sysroot,$(SYSROOT_PATH))" "$(SYSROOT_PATH)"
endif
endif
endif
	$(call decompression_file,$(SRC_KERNEL),$(KERNEL_URL))
	$(Q)+make -C $(SRC_KERNEL) -f $(MAKEFILE_HEADER)
else
	$(Q)+make CAVAN_HOST_ARCH=$(CAVAN_BUILD_ARCH)
	$(Q)rm -rf "$(SYSROOT_PATH)" && cp -av "$(patsubst %/sysroot,%-$(CAVAN_BUILD_ARCH)/sysroot,$(SYSROOT_PATH))" "$(SYSROOT_PATH)"
endif
endif
	$(call generate_mark)

$(OUT_TOOLCHIAN) $(MARK_TOOLCHIAN) $(TOOLCHIAN_PATH):
	$(Q)mkdir -pv $@
