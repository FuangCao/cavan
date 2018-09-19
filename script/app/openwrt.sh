#!/bin/bash

function cavan-openwrt-decode()
{
	local input_fn="$1"
	local output_dn="$2"
	local firmware_bin kernel_bin kernel_bytes rootfs_bin rootfs_dir

	[ "${input_fn}" ] ||
	{
		echo "Usage: $0 <INPUT> [OUTPUT]"
		return 1
	}

	if [ "${output_dn}" ]
	then
		mkdir -pv "${output_dn}" || return 1
	else
		output_dn="."
	fi

	firmware_bin="${output_dn}/firmware.bin"
	kernel_bin="${output_dn}/kernel.bin"
	rootfs_bin="${output_dn}/rootfs.bin"
	rootfs_dir="${output_dn}/rootfs"
	kernel_bytes=$(file "${input_fn}" | sed 's/.* \([0-9]\+\) bytes,.*$/\1/g')

	echo "kernel_bytes = ${kernel_bytes}"

	rm -fv "${firmware_bin}" "${kernel_bin}" "${rootfs_bin}" || return 1
	rm -rf "${rootfs_dir}" || return 1

	dd if="${input_fn}" of="${firmware_bin}" bs=64 skip=1 || return 1
	dd if="${firmware_bin}" of="${rootfs_bin}" bs="${kernel_bytes}" skip=1 || return 1
	dd if="${firmware_bin}" of="${kernel_bin}" bs="${kernel_bytes}" count=1 || return 1
	unsquashfs -d "${rootfs_dir}" "${rootfs_bin}" || return 1

	echo "OK"
}
