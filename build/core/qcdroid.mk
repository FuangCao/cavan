UTLILS_ROOT = $(HOME)/tools/rvct22
ROOT_DIR = $(shell pwd -P)
AP_ROOT = $(ROOT_DIR)/qcdroid
PROJECT = $(notdir $(ROOT_DIR))

$(info PROJECT = $(PROJECT))

ifeq "$(PROJECT)" "msm7627a"
TARGET_PRODUCT = msm7627a
CP_ROOT = $(ROOT_DIR)/qcom_cp
COMMAND_FILE = USKOLYM_FLASH_SD5D14A_4G.cmd
KERNEL_CONFIG = $(TARGET_PRODUCT)-perf_defconfig
else
TARGET_PRODUCT = EG909
CP_ROOT = $(ROOT_DIR)/qcom_cp_orig
COMMAND_FILE = SSKOLYM_FLASH_H9DP32A4JJACGR_KEM.cmd
KERNEL_CONFIG = egs8235_v1_eg909_defconfig
endif

ARM_COMPILER_PATH = $(UTLILS_ROOT)/rvct22/linux-pentium
ARMTOOLS = RVCT221
ARMROOT = $(UTLILS_ROOT)/rvct22
ARMLIB = $(ARMROOT)/lib
ARMINCLUDE = $(ARMROOT)/include/unix
ARMINC = $(ARMINCLUDE)
ARMCONF = $(ARMROOT)/linux-pentium
ARMDLL = $(ARMROOT)/linux-pentium
ARMBIN = $(ARMROOT)/linux-pentium
ARMHOME = $(ARMROOT)
ARMLMD_LICENSE_FILE = $(UTLILS_ROOT)/license.dat

MODEM_PROC = $(CP_ROOT)/modem_proc
BUILD_MS = $(MODEM_PROC)/build/ms
TARGET_OUT = $(AP_ROOT)/out/target/product/$(TARGET_PRODUCT)
OUT_HOST = $(AP_ROOT)/out/host
KERNEL_OUT = $(AP_ROOT)/out/kernel
KERNEL_ROOT = $(AP_ROOT)/kernel
KERNEL_MAKE = make -C $(KERNEL_ROOT) O=$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-cavan-linux-gnueabi-

PATH := $(ARM_COMPILER_PATH):$(OUT_HOST)/linux-x86/bin:$(PATH)

export ARM_COMPILER_PATH ARMTOOLS ARMROOT ARMLIB ARMINCLUDE ARMINC ARMCONF ARMDLL ARMBIN ARMHOME ARMLMD_LICENSE_FILE PATH

ap:
	+cd $(AP_ROOT) && make PRODUCT-$(TARGET_PRODUCT)-eng

cp:
ifeq "$(PROJECT)" "msm7627a"
	ln -vsf custusnskolym.h $(BUILD_MS)/CUSTUSNSKOLYM.H
	chmod a+x qcom_cp/modem_proc/build/ms/*.pl qcom_cp/modem_proc/tools/build/*.pl -v
else
	ln -vsf custSSNSKOLYM.h $(MODEM_PROC)/build/cust/CUSTSSNSKOLYM.H
	ln -vsf custssnskolym.h $(BUILD_MS)/CUSTSSNSKOLYM.H
endif
	+cd $(BUILD_MS) $(shell while read line; do echo " && "; echo "$${line}" | sed -e "s/^make/make BUILD_UNIX=yes/g" -e "s/\$$0/$(COMMAND_FILE)/g"; done < $(BUILD_MS)/$(COMMAND_FILE))

boot boot.img:
	mkdir $(KERNEL_OUT) -pv
	+$(KERNEL_MAKE) $(KERNEL_CONFIG)
	+$(KERNEL_MAKE) headers_install
	+$(KERNEL_MAKE) zImage
	mkbootimg	--output $(TARGET_OUT)/boot.img \
				--kernel $(KERNEL_OUT)/arch/arm/boot/zImage \
				--ramdisk  $(TARGET_OUT)/ramdisk.img \
				--cmdline "console=ttyMSM2,115200n8 androidboot.hardware=qcom" \
				--base 0x00200000 --pagesize 4096

kconfig menuconfig:
	mkdir $(KERNEL_OUT) -pv
	+$(KERNEL_MAKE) headers_install
	+$(KERNEL_MAKE) menuconfig
	cp $(KERNEL_OUT)/.config $(KERNEL_ROOT)/arch/arm/configs/$(KERNEL_CONFIG) -av
