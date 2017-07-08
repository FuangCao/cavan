#!/bin/bash

alias cavan-disk-get-uuid="blkid -s UUID -o value"
alias cavan-disk-get-type="blkid -s TYPE -o value"
alias cavan-disk-get-parts="cat /proc/partitions | grep '^\s*[0-9].*[0-9]\s*$' | awk '{ print \$4 }'"

function cavan-disk-gen-fstab-single()
{
	local uuid type point

	uuid="$(cavan-disk-get-uuid $1)"
	[ "${uuid}" ] || return 1

	type="$(cavan-disk-get-type $1)"
	point="/mnt/$(basename $1)"

	case "${type}" in
		ext[2-4] | vfat | ntfs)
			sudo mkdir -p "${point}" || return 1
			echo "UUID=${uuid} ${point} ${type} errors=remount-ro 0 0"
			;;

		*)
			return 1
			;;
	esac
}

function cavan-disk-gen-fstab()
{
	local dev

	if [ "$1" ]
	then
		for dev in $@
		do
			cavan-disk-gen-fstab-single "${dev}"
		done
	else
		for dev in $(cavan-disk-get-parts)
		do
			cavan-disk-gen-fstab-single "/dev/${dev}"
		done
	fi
}

function cavan-disk-create-raid0()
{
	local dev

	for ((i = 0; i < 128; i++))
	do
		dev="/dev/md$i"
		[ -e "${dev}" ] || break
	done

	echo "dev = ${dev}"
	sudo mdadm -C "${dev}" -l 0 -n $# $@
}
