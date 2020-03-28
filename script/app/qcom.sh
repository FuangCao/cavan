#!/bin/bash

alias cavan-lunch-CP10="cavan-android-choosecombo 1 CP10 userdebug 32"
alias cavan-build-CP10="cavan-lunch-CP10 && cavan-qcom-build-android"

alias cavan-lunch-APH7="cavan-android-choosecombo 1 APH7 userdebug 32"
alias cavan-build-APH7="cavan-lunch-APH7 && cavan-qcom-build-android"

alias cavan-qcom-build-bootloader="cavan-android-make aboot"
alias cavan-qcom-build-kernel="cavan-android-make bootimage"
alias cavan-qcom-build-system="cavan-android-make systemimage"

function cavan-qcom-mkdir()
{
	[ -d "$1" ] && return 0
	[ -e "$1" ] && rm -f "$1" || return 1
	mkdir -pv "$1" || return 1
}

function cavan-qcom-build-android()
{
	local target_system="${ANDROID_PRODUCT_OUT}/system"

	cavan-android-croot || return 1

	cavan-qcom-mkdir "${target_system}/lib" || return 1
	cavan-qcom-mkdir "${target_system}/system/lib64" || return 1
	cavan-android-make || return 1
}

function cavan-qcom-build-modem-msm8953()
{
	cavan-android-croot && cd modem || return 1
	rm -rf ADSP.8953.2.8.2/adsp_proc/obj || return 1
	source factory/build/envsetup_sm55c72.sh || return 1
	./build_all_8953.sh || return 1
	./gen_firehose_8953.sh || return 1
	./gen_symbols_8953.sh || return 1
}

function cavan-qcom-pack-boot-msm8953()
{
	local cmdline="console=ttyHSL0,115200,n8 androidboot.console=ttyHSL0 androidboot.hardware=qcom msm_rtb.filter=0x237 ehci-hcd.park=3 lpm_levels.sleep_disabled=1 androidboot.bootdevice=7824900.sdhci earlycon=msm_hsl_uart,0x78af000 androidboot.selinux=permissive buildvariant=userdebug"
	local bootd_img="${ANDROID_PRODUCT_OUT}/boot.img"

	echo "pack: ${bootd_img} <= $1"

	out/host/linux-x86/bin/mkbootimg --kernel $1 --ramdisk ${ANDROID_PRODUCT_OUT}/ramdisk.img --base 0x80000000 --pagesize 2048 --cmdline "${cmdline}" --os_version 7.1.2 --os_patch_level xxxx-xx-xx --output ${bootd_img} || return 1
	out/host/linux-x86/bin/boot_signer /boot ${ANDROID_PRODUCT_OUT}/boot.img build/target/product/security/sunny_releasekey/verity.pk8 build/target/product/security/sunny_releasekey/verity.x509.pem ${bootd_img} || return 1

	echo "Success: ${bootd_img}"
}

function cavan-qcom-build-kernel-msm8953()
{
	local TARGET_KERNEL_SOURCE="${ANDROID_BUILD_TOP}/kernel/msm-3.18"
	local KERNEL_ARCH="arm64"
	local KERNEL_CROSS_COMPILE="aarch64-linux-android-"
	local KERNEL_OUT="${ANDROID_PRODUCT_OUT}/cavan-kernel"
	local KERNEL_DEFCONFIG="$1"
	local KERNEL_MAKE="${CMD_MAKE} -C ${TARGET_KERNEL_SOURCE} O=${KERNEL_OUT} ARCH=${KERNEL_ARCH} CROSS_COMPILE=${KERNEL_CROSS_COMPILE} KCFLAGS=-mno-android"

	[ -d "${ANDROID_BUILD_TOP}" ] ||
	{
		echo "ANDROID_BUILD_TOP not set!"
		return 1
	}

	mkdir -pv "${KERNEL_OUT}" || return 1
	# rm -rfv "${KERNEL_OUT}/arch/${KERNEL_ARCH}/boot/dts" || return 1
	${KERNEL_MAKE} ${KERNEL_DEFCONFIG} && ${KERNEL_MAKE} || return 1
	# ${KERNEL_MAKE} modules && ${KERNEL_MAKE} INSTALL_MOD_PATH=${ANDROID_PRODUCT_OUT}/system INSTALL_MOD_STRIP=1 modules_install || return 1

	(cd ${ANDROID_BUILD_TOP} && cavan-qcom-pack-boot-msm8953 ${KERNEL_OUT}/arch/${KERNEL_ARCH}/boot/Image.gz-dtb) || return 1

	echo "Build successfull"
}

alias cavan-build-kernel-CP10="cavan-qcom-build-kernel-msm8953 cp10_defconfig"
