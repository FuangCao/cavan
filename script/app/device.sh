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

function cavan-readline()
{
	local line

	while :;
	do
		read line && echo $line;
	done
}

function cavan-mount-overlay()
{
	[ "$3" ] ||
	{
		echo "cavan-mount-overlay <lowerdir> <upperdir> <workdir>"
		return 1
	}

	sudo mount -t overlay overlay -o lowerdir=$3,upperdir=$1,workdir=$2 $3
}
