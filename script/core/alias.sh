#!/bin/bash

alias vi='vim -O'
alias va='vi *.[chsS]'
alias vh='vi *.h'
alias vc='vi *.c'
alias vs='vi *.[sS]'
alias vm='vi [Mm]akefile'
alias mkconfig='make menuconfig'
alias ls='ls --color=auto'
alias dir='dir --color=auto'
alias vdir='vdir --color=auto'
alias grep='grep --color=auto --exclude-dir .git --exclude-dir .svn --exclude-dir .cavan-git'
alias fgrep='fgrep --color=auto'
alias egrep='egrep --color=auto'
alias ll='ls -alhF'
alias la='ls -A'
alias l='ls -CF'
alias mkae='make'
alias mkea='make'
alias cavan-daemon-command='cavan-service -s 0 --'
alias git='git $([ -d .cavan-git ] && echo "--git-dir=.cavan-git --work-tree=.")'

CMD_TCP_DD_SERVER="${CAVAN_OUT_DEBUG}/cavan-tcp_dd_server"
CMD_TFTP_DD_SERVER="${CAVAN_OUT_DEBUG}/cavan-tftp_dd_server"

CMD_ARM_CAVAN_MAIN="${CAVAN_OUT_ARM}/cavan-main"
CMD_DATA_CAVAN_MAIN="/data/internal_memory/cavan-main"
CMD_SYSTEM_CAVAN_MAIN="/system/bin/cavan-main"

function cavan-make2()
{
	[ -f "[Mm]akefile" ] ||
	{
		rm "[Mm]akefile" -rfv
		cat > Makefile << EOF
### This file is automatic generation by FuangCao ###

# ARCH =
# CROSS_COMPILE =
# CFLAGS =
# LDFLAGS =

include ${CAVAN_HOME}/script/core/general.mk
EOF
	}

	make || return 1

	return 0
}

function adb-push-directory()
{
	local fn

	cd $1 || return 1

	for fn in *
	do
		[ -f "${fn}" ] || continue
		echo "Push ${fn} => $2"
		adb push ${fn} $2 || return 1
	done

	return 0
}

function cavan-server-run()
{
	[ "$(id -u)" = "0" ] ||
	{
		echo "Only super user can do this"
		return 1
	}

	for server in ${CMD_TCP_DD_SERVER} ${CMD_TFTP_DD_SERVER}
	do
		start-stop-daemon --start --exec ${server} -- --daemon
	done

	return 0
}

function cavan-server-stop()
{
	[ "$(id -u)" = "0" ] ||
	{
		echo "Only super user can do this"
		return 1
	}

	for server in ${CMD_TCP_DD_SERVER} ${CMD_TFTP_DD_SERVER}
	do
		start-stop-daemon --stop --exec ${server}
	done
}

function mssh()
{
	ssh ${SSH_USER}@${1-${CAVAN_SERVER_IP}}
}

function cavan-ssh()
{
	cavan-tcp_exec --port ${1-8888} --ip ${2-${CAVAN_SERVER_IP}} bash
}

function cavan-adb-loop_run()
{
	cavan-loop_run -wd2 "adb shell $*" || return 1
}

function cavan-adb-cavan-main()
{
	adb push ${CMD_ARM_CAVAN_MAIN} ${CMD_DATA_CAVAN_MAIN} || return 1
	adb shell "chmod 777 ${CMD_DATA_CAVAN_MAIN} && ${CMD_DATA_CAVAN_MAIN} \"$*\"" || return 1
}

function cavan-adb-tcp_dd_server()
{
	cavan-loop_run -wd2 "adb remount; adb shell cavan-main tcp_dd_server"
}

function cavan-adb-logcat()
{
	cavan-loop_run -wd2 "adb logcat -v time $*" || return 1
}

function cavan-adb-kmsg()
{
	cavan-adb-loop_run "cat /proc/kmsg" || return 1
}

function cavan-adb-build-env()
{
	adb remount || return 1
	adb push ${CMD_ARM_CAVAN_MAIN} ${CMD_SYSTEM_CAVAN_MAIN} || return 1
	adb shell chmod 06777 ${CMD_SYSTEM_CAVAN_MAIN} || return 1

	adb shell chmod 777 /data/bin/bash || return 1
	adb shell cp /data/bin/bash /system/bin/sh || return 1
}

function cavan-svn-cleanup()
{
	cavan-mkdir .svn/tmp
	svn cleanup
	cavan-mkdir .svn/tmp
	rm -rf .svn/tmp
	svn cleanup
}

function cavan-format-code()
{
	sed -i 's/\s\+$//g' $*
}

function cavan-make-apk()
{
	[ -f Makefile ] ||
	{
		ln -sf ${CAVAN_HOME}/build/core/apk_main.mk Makefile
	}

	make
}

function cavan-git-config()
{
	local CMD_GIT_CONFIG="git config --global"

	${CMD_GIT_CONFIG} user.name Fuang.Cao || return 1
	${CMD_GIT_CONFIG} user.email cavan.cfa@gmail.com || return 1
	${CMD_GIT_CONFIG} core.editor vim || return 1
	${CMD_GIT_CONFIG} push.default matching || return 1

	for cmd in branch  interactive ui diff grep status
	do
		echo "git config color.${cmd} auto"
		git config --global color.${cmd} auto;
	done
}

function cavan-sign-update-zip()
{
	local KEY_DIR KEY_NAME FILE_SIGNAPK FILE_INPUT FILE_OUTPUT

	[ "$1" ] ||
	{
		echo "Usage: sign-update-zip update.zip [keyname]"
		return 1
	}

	FILE_INPUT="$1"
	[[ "${FILE_INPUT}" == *.zip ]] ||
	{
		echo "input file '${FILE_INPUT}' is not a .zip file"
		return 1
	}

	[ -f "${FILE_INPUT}" ] ||
	{
		echo "input file '${FILE_INPUT}' is not exists"
		return 1
	}

	FILE_OUTPUT="${FILE_INPUT%.zip}-sign.zip"
	[ -e "${FILE_OUTPUT}" ] &&
	{
		echo "output file '${FILE_OUTPUT}' is exists"
		return 1
	}

	KEY_DIR="build/target/product/security"
	[ -d "${KEY_DIR}" ] ||
	{
		echo "directory '${KEY_DIR}' is not exists"
		return 1
	}

	FILE_SIGNAPK="out/host/linux-x86/framework/signapk.jar"
	[ -f "${FILE_SIGNAPK}" ] ||
	{
		echo "file '${FILE_SIGNAPK}' is not exists"
		return 1
	}

	if [ "$2" ]
	then
		KEY_NAME="$2"
	else
		KEY_NAME="testkey"
	fi

	java -jar "${FILE_SIGNAPK}" -w "${KEY_DIR}/${KEY_NAME}.x509.pem" "${KEY_DIR}/${KEY_NAME}.pk8" "${FILE_INPUT}" "${FILE_OUTPUT}" || return 1

	return 0
}

function cavan-firefox-flash-install()
{
	local work_path plugins_path

	[ "$1" ] ||
	{
		echo "Please give flash plugin pathname"
		return 1
	}

	work_path="$(mktemp -d)"
	tar -xvf "$1" -C "${work_path}" || return 1
	plugins_path="/usr/lib/firefox/browser/plugins"
	sudo cp ${work_path}/usr/* /usr -av || return 1
	sudo mkdir ${plugins_path} -pv || return 1
	sudo cp ${work_path}/lib*.so ${plugins_path} -av || return 1
	rm ${work_path} -rfv

	return 1
}

function cavan-kernel-config()
{
	local ARCH CMD_MAKE DEF_CONFIG SAVE_CONFIG KERNEL_PATH BOARD_NAME

	BOARD_NAME="${1-${TARGET_PRODUCT}}"

	[ "${BOARD_NAME}" ] ||
	{
		echo "Please give the board name"
		return 1
	}

	ARCH="${2-arm}"
	DEF_CONFIG="${BOARD_NAME}_defconfig"
	CMD_MAKE="make ARCH=${ARCH} CROSS_COMPILE=arm-eabi-"
	KERNEL_PATH="${OUT}/obj/KERNEL_OBJ"

	[ -d "${KERNEL_PATH}" ] &&
	{
		CMD_MAKE="${CMD_MAKE} -C ${KERNEL_PATH}"
	}

	echo "BOARD_NAME = ${BOARD_NAME}"
	echo "ARCH = ${ARCH}"
	echo "DEF_CONFIG = ${DEF_CONFIG}"
	echo "CMD_MAKE = ${CMD_MAKE}"
	echo "KERNEL_PATH = ${KERNEL_PATH}"

	${CMD_MAKE} ${DEF_CONFIG} || return 1
	${CMD_MAKE} menuconfig || return 1
	if ${CMD_MAKE} savedefconfig
	then
		SAVE_CONFIG="defconfig"
	else
		SAVE_CONFIG=".config"
	fi

	cp ${KERNEL_PATH}/${SAVE_CONFIG} arch/${ARCH}/configs/${DEF_CONFIG} -av && return 0

	return 1
}
