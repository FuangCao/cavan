#!/bin/bash

function cavan-umount-disk()
{
	local dev

	for dev in $1[0-9]*
	do
		[ -b "${dev}" ] || continue

		echo "umount: ${dev}"

		while sudo umount "${dev}"
		do
			echo "umount ${dev} successfull"
		done
	done
}

function cavan-mount-disk()
{
	local dev devname mpoint

	for dev in $1[0-9]*
	do
		[ -b ${dev} ] || continue

		devname=$(basename "${dev}")
		mpoint="/mnt/${devname}"

		echo "umount: ${dev}"

		while sudo umount "${dev}"
		do
			echo "umount ${dev} successfull"
		done

		echo "mount: ${dev} => ${mpoint}"

		sudo mkdir -pv "${mpoint}" || return 1
		sudo mount "${dev}" "${mpoint}" && echo "mount ${dev} successfull"
	done
}
