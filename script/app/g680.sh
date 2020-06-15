#!/bin/bash

function cavan-g680-build-kernel()
{
	local ROCKDEV_PATH=rockdev/Image-${TARGET_PRODUCT}
	local KERNEL_DEBUG=kernel/arch/arm64/boot/Image
	local PLATFORM_VERSION=9
	local BOARD_BOOTIMG_HEADER_VERSION=1
	local PLATFORM_SECURITY_PATCH=2019-10-05
	local KERNEL_DTS=${1-rk3399-g680-v2}
	local KERNEL_DEFCONFIG=g680_defconfig
	local BOARD_KERNEL_CMDLINE="earlycon=uart8250,mmio32,0xff1a0000 swiotlb=1 console=ttyFIQ0 androidboot.baseband=N/A androidboot.veritymode=enforcing androidboot.hardware=rk30board androidboot.console=ttyFIQ0 androidboot.selinux=permissive init=/init initrd=0x62000001,0x00800000 coherent_pool=1m"

	echo "KERNEL_DTS = ${KERNEL_DTS}"
	echo "KERNEL_DEFCONFIG = ${KERNEL_DEFCONFIG}"
	echo "BOARD_KERNEL_CMDLINE = ${BOARD_KERNEL_CMDLINE}"

	echo "make kernel image"
	(cavan-android-croot kernel && make ARCH=arm64 ${KERNEL_DEFCONFIG} && make ARCH=arm64 ${KERNEL_DTS}.img -j8) || return 1

	echo "make boot.img"
	(cavan-android-croot && mkbootimg --kernel ${KERNEL_DEBUG} --second kernel/resource.img --os_version ${PLATFORM_VERSION} --header_version ${BOARD_BOOTIMG_HEADER_VERSION} --os_patch_level ${PLATFORM_SECURITY_PATCH} --cmdline "${BOARD_KERNEL_CMDLINE}" --output kernel/boot.img && cp -av kernel/boot.img ${ROCKDEV_PATH}) || return 1
}

alias cavan-g680-build-kernel-v1="cavan-g680-build-kernel rk3399-g680-demo-v1"
