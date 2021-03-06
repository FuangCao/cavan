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

alias cavan-rockchip-pack-recovery-push="cavan-rockchip-pack-recovery && cavan-adb-tcp_dd --auto ${IMAGE_PATH}/recovery.img"

function cavan-rockchip-pack-boot()
{
	cavan-rockchip-buildenv || return 1

	cavan-do-command "mkbootfs ${OUT}/root | minigzip > ${IMAGE_PATH}/ramdisk.img" || return 1
	cavan-do-command "truncate -s "%4" ${IMAGE_PATH}/ramdisk.img" || return 1
	cavan-do-command "rkst/mkkrnlimg ${IMAGE_PATH}/ramdisk.img ${IMAGE_PATH}/boot.img" || return 1
}

alias cavan-rockchip-pack-boot-push="cavan-rockchip-pack-boot && cavan-adb-tcp_dd --auto ${IMAGE_PATH}/boot.img"

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

	cavan-do-command "e2fsck -fy ${IMAGE_PATH}/system.img" || return 1
	cavan-do-command "resize2fs -fpM ${IMAGE_PATH}/system.img" || return 1
}

alias cavan-rockchip-pack-system-push="cavan-rockchip-pack-system && cavan-adb-tcp_dd --auto ${IMAGE_PATH}/system.img"

function cavan-rockchip-download()
{
	local options android_root rockdev upgrade_tool

	android_root="$(cavan-android-get-root)"

	[ "${android_root}" ] || return 1

	upgrade_tool="${android_root}/rkbin/tools/upgrade_tool"
	rockdev="${android_root}/rockdev/Image-${TARGET_PRODUCT}"

	adb reboot-bootloader

	for ((i = 0; i < 10; i++))
	do
		${upgrade_tool} TD && break
		sleep 1
	done

	for fn in $@
	do
		[ -f "${fn}" ] ||
		{
			case "${fn}" in
				reboot)
					;;
				param | parameter)
					fn="${rockdev}/parameter.txt"
					;;
				kernel | resource | boot)
					fn="${rockdev}/${fn}.img"
					;;
				kernel.img | resource.img | boot.img)
					fn="${rockdev}/${fn}"
					;;
				u)
					fn="${rockdev}/uboot.img"
					;;
				k)
					fn="${rockdev}/kernel.img"
					;;
				r)
					fn="${rockdev}/resource.img"
					;;
				b)
					fn="${rockdev}/boot.img"
					;;
				s)
					fn="${rockdev}/system.img"
					;;
				v)
					fn="${rockdev}/vendor.img"
					;;
				d)
					fn="${rockdev}/userdata.img"
					;;
				m)
					fn="${rockdev}/misc.img"
					;;
				o)
					fn="${rockdev}/oem.img"
					;;
				*.txt | *.img)
					fn="${rockdev}/${fn}"
					;;
				*)
					fn="${rockdev}/${fn}.img"
					;;
			esac
		}

		case "$(basename ${fn})" in
			boot.img)
				options="DI -b"
				;;
			dtbo.img)
				options="DI dtbo"
				;;
			kernel.img)
				options="DI -k"
				;;
			resource.img)
				options="DI -re"
				;;
			misc.img)
				options="DI -m"
				;;
			oem.img)
				options="DI oem"
				;;
			parameter.txt)
				options="DI -p"
				;;
			recovery.img)
				options="DI -r"
				;;
			system.img)
				options="DI -s"
				;;
			trust.img)
				options="DI trust"
				;;
			uboot.img)
				options="DI -u"
				;;
			vbmeta.img)
				options="DI vbmeta"
				;;
			vendor.img)
				options="DI vendor"
				;;
			update.img)
				options="UF"
				;;
			reboot)
				options="RD"
				unset fn
				;;
			*)
				echo "Invalid image ${fn}"
				continue
		esac

		cavan-do-command "${upgrade_tool} ${options} ${fn}" || break
	done
}

alias cavan-rockchip-reboot-bootloader="adb reboot bootloader"

alias cavan-rockchip-build-kernel="(cavan-android-croot && ./build.sh -K)"
alias cavan-rockchip-build-uboot="(cavan-android-croot && ./build.sh -U)"
alias cavan-rockchip-build-android="cavan-android-croot && ./build.sh -A)"
alias cavan-rockchip-build-all="(cavan-android-croot && ./build.sh -UKAu)"
