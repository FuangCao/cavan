#!/bin/bash

IMAGE_PATH="out/rockchip"

function cavan-rockchip-buildenv()
{
	cavan-do-command "source build/envsetup.sh && setpaths" || return 1
	cavan-do-command "mkdir -pv ${IMAGE_PATH}" || return 1
}

function cavan-rockchip-pack-recovery()
{
	cavan-rockchip-buildenv || return 1

	cavan-do-command "mkbootfs ${OUT}/recovery/root | minigzip > ${IMAGE_PATH}/ramdisk-recovery.img" || return 1
	cavan-do-command "truncate -s '%4' ${IMAGE_PATH}/ramdisk-recovery.img" || return 1
	cavan-do-command "mkbootimg --kernel kernel/kernel.img --ramdisk ${IMAGE_PATH}/ramdisk-recovery.img --output ${IMAGE_PATH}/recovery.img" || return 1
}

function cavan-rockchip-pack-boot()
{
	cavan-rockchip-buildenv || return 1

	cavan-do-command "mkbootfs ${OUT}/root | minigzip > ${IMAGE_PATH}/ramdisk.img" || return 1
	cavan-do-command "truncate -s "%4" ${IMAGE_PATH}/ramdisk.img" || return 1
	cavan-do-command "rkst/mkkrnlimg ${IMAGE_PATH}/ramdisk.img ${IMAGE_PATH}/boot.img" || return 1
}

function cavan-rockchip-pack-system()
{
	local system_size system_args

	cavan-rockchip-buildenv || return 1

	system_size=$(ls -l ${OUT}/system.img | awk '{ print $5; }')
	system_args="-L system -S ${OUT}/root/file_contexts -a system ${IMAGE_PATH}/system.img ${OUT}/system"

	while :;
	do
		cavan-do-command "make_ext4fs -l ${system_size} ${system_args} && tune2fs -c -1 -i 0 ${IMAGE_PATH}/system.img" && break
		system_size=$((${system_size} + 5242880))
	done

	cavan-do-command "e2fsck -fyD ${IMAGE_PATH}/system.img" || return 1
	cavan-do-command "resize2fs -fpM ${IMAGE_PATH}/system.img" || return 1
}
