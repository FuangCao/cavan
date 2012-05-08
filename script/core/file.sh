#!/bin/bash

function cavan-symlink()
{
	[ "$2" ] || return 0

	mkdir $(dirname "$1") -pv || return 1

	if [ -L "$2" ]
	then
		rm "$2" || return 1
	elif [ -d "$2" ]
	then
		if [ -d "$1" ]
		then
			cp "$2"/* "$1" -anv
			rm "$2" -rfv
		else
			rm "$1" -rfv
			mv "$2" "$1" -v
		fi
	fi

	[ -e "$1" ] || mkdir "$1" -p || return 1
	ln -vsf "$1" "$2" || return 1
}

function get_file_abs_directory()
{
	cd $(dirname $1) && pwd
}

function get_file_abs_path()
{
	echo $(get_file_abs_directory $1)/$(basename $1)
}

function get_remote_file_directory()
{
	get_file_abs_directory $1 | sed "s#^${PROJECT_HOME}#${REMOTE_PROJECT_HOME}#g"
}

function get_remote_file_path()
{
	echo $(get_remote_file_directory $1)/$(basename $1)
}

function build_tcp_command()
{
	echo -n $1
	[ "${CAVAN_SERVER_IP}" ] && echo -n " --ip=${CAVAN_SERVER_IP}"
	[ "${CAVAN_SERVER_PORT}" ] && echo -n " --port=${CAVAN_SERVER_PORT}"
}

function tput()
{
	local command

	command="$(build_tcp_command ${CMD_TCP_COPY})"

	${command} -w $* || return 1

	return 0
}

function gput_file()
{
	tput $1 $(get_remote_file_directory $1) || return 1

	return 0
}

function gput_files()
{
	local fn

	for fn in $*
	do
		gput_file ${fn} || return 1
	done

	return 0
}

function tget()
{
	local command

	command="$(build_tcp_command ${CMD_TCP_COPY})"

	${command} -r $* || return 1

	return 0
}

function gget_file()
{
	tget $(get_remote_file_path $1) $(dirname $1) || return 1

	return 0
}

function gget_files()
{
	local fn

	for fn in $*
	do
		gget_file ${fn} || return 1
	done

	return 0
}

function gput()
{
	local fn

	for fn in $*
	do
		if [ -d "${fn}" ]
		then
			gput ${fn}/* || return 1
		else
			gput_file ${fn} || return 1
		fi
	done

	return 0
}

function gget()
{
	local fn

	for fn in $*
	do
		if [ -d "${fn}" ]
		then
			gget ${fn}/* || return 1
		else
			gget_file ${fn} || return 1
		fi
	done

	return 0
}

function gput_fsldroid()
{
	[ "$1" ] ||
	{
		echo "Please give the product name"
		return 1
	}

	tput ${FSLDROID_HOME}/out/target/product/$1/*.img ${REMOTE_PRODUCT_HOME}/$1 || return 1

	return 0
}

function mkkernel()
{
	local MAKE_KERNEL

	MAKE_KERNEL="make -C ${KERNEL_HOME} arch=arm cross_compile=${CAVAN_CROSS_COMPILE}"
	echo "MAKE_KERNEL = ${MAKE_KERNEL}"

	[ "$1" ] &&
	{
		${MAKE_KERNEL} $1 || return 1
	}

	${MAKE_KERNEL} uImage -j${MAKE_JOBS} || return 1

	return 0
}

function bkernel()
{
	local command

	command="$(build_tcp_command ${CMD_TCP_DD})"

	${command} -w if="${KERNEL_HOME}/arch/arm/boot/uImage" of="${TARGET_DEVICE}" bs=1M seek=1 || return 1

	return 0
}

function mbkernel()
{
	mkkernel $1 && bkernel || return 1

	return 0
}

function mkuboot()
{
	local MAKE_UBOOT

	MAKE_UBOOT="make -C ${UBOOT_HOME} arch=arm cross_compile=${CAVAN_CROSS_COMPILE}"
	echo "MAKE_UBOOT = ${MAKE_UBOOT}"

	[ "$1" ] &&
	{
		${MAKE_UBOOT} distclean || return 1
		${MAKE_UBOOT} $1 || return 1
	}

	${MAKE_UBOOT} -j${MAKE_JOBS} || return 1

	return 0
}

function buboot()
{
	local command

	command="$(build_tcp_command ${CMD_TCP_DD})"

	${command} -w if=${UBOOT_HOME}/u-boot.bin of=${TARGET_DEVICE} bs=1k seek=1 skip=1 || return 1

	return 0
}

function mbuboot()
{
	mkuboot $1 && buboot || return 1

	return 0
}

function mkfsldroid()
{
	local command

	[ "$1" ] ||
	{
		echo "Please give a product name"
		return 1
	}

	command="make -C ${FSLDROID_HOME} PRODUCT-$1-eng -j${MAKE_JOBS} | tee ${FSLDROID_HOME}/$1.log"
	echo ${command}

	${command} || return 1

	return 0
}

function mbfsldroid()
{
	mkfsldroid $1 || return 1
	gput_fsldroid $1 || return 1

	return 0
}
