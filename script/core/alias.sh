#!/bin/bash

alias vi='vim -O'
alias va='vi *.[chsS]'
alias vh='vi *.h'
alias vc='vi *.c'
alias vs='vi *.[sS]'
alias vm='vi [Mm]akefile'
alias mkconfig='make menuconfig'

if [ "${CAVAN_OS_MAC}" = "true" ]
then
alias ls='ls -G@'
alias dir='ls'
alias vdir='ls -lh'
else
alias ls='ls --color=auto'
alias dir='dir --color=auto'
alias vdir='vdir --color=auto'
fi

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
alias cavan-make-module='make -f ${CAVAN_HOME}/driver/build/main.mk'
alias cavan-git-updated='git log . | grep Fuang'
alias cavan-touch-files='find * -type f | while read line; do echo "touch $line"; touch -c "$line"; done'
alias cavan-flash-install="cavan-firefox-flash-install"
alias cdp="cd .."
alias cdpp="cd ../.."
alias cdppp="cd ../../.."

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

function cavan-git-config()
{
	local CMD_GIT_CONFIG="git config --global"

	${CMD_GIT_CONFIG} user.name Fuang.Cao || return 1
	${CMD_GIT_CONFIG} user.email cavan.cfa@gmail.com || return 1
	${CMD_GIT_CONFIG} core.editor vim || return 1
	${CMD_GIT_CONFIG} core.filemode false || return 1
	${CMD_GIT_CONFIG} push.default matching || return 1
	${CMD_GIT_CONFIG} http.sslverify false || return 1
	# ${CMD_GIT_CONFIG} credential.helper cache
	# ${CMD_GIT_CONFIG} credential.helper 'cache --timeout=3600'
	${CMD_GIT_CONFIG} credential.helper store

	for cmd in branch  interactive ui diff grep status
	do
		echo "git config color.${cmd} auto"
		${CMD_GIT_CONFIG} color.${cmd} auto;
	done
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

	(
		cd "${work_path}" && find usr -type f | while read line
		do
			sudo mkdir -pv "/$(dirname "${line}")" || return 1
			sudo cp -av "${line}" "/${line}" || return 1
		done
	)

	plugins_path="/usr/lib/firefox/browser/plugins"
	sudo mkdir -pv ${plugins_path} && sudo cp -av ${work_path}/lib*.so ${plugins_path} || return 1

	rm ${work_path} -rfv

	return 1
}

function cavan-kernel-config()
{
	local ARCH CMD_MAKE DEF_CONFIG SAVE_CONFIG KERNEL_PATH BOARD_NAME KERNEL_SOURCE

	[ "${ANDROID_BUILD_TOP}" ] ||
	{
		echo "please run source build/envsetup"
		return 1
	}

	BOARD_NAME="${1-${TARGET_PRODUCT}}"

	[ "${BOARD_NAME}" ] ||
	{
		echo "Please give the board name"
		return 1
	}

	ARCH="${2-arm}"

	if [ -d "${ANDROID_BUILD_TOP}/device/marvell" ]
	then
		KERNEL_PATH="${OUT}/obj/kernel"
		if [[ ${BOARD_NAME} = pxa1L88H* ]]
		then
			DEF_CONFIG="${BOARD_NAME/H/_h}_defconfig"
		elif [[ ${BOARD_NAME} = pxa1L88X* ]]
		then
			DEF_CONFIG="${BOARD_NAME/X/_x}_defconfig"
		else
			DEF_CONFIG="${BOARD_NAME}_defconfig"
		fi
	else
		KERNEL_PATH="${OUT}/obj/KERNEL_OBJ"
		DEF_CONFIG="${BOARD_NAME}_defconfig"
	fi

	[ -d "${KERNEL_PATH}" ] ||
	{
		echo "kernel path ${KERNEL_PATH} not found!"
		return 1
	}

	CMD_MAKE="make -C ${KERNEL_PATH} ARCH=${ARCH} CROSS_COMPILE=arm-eabi-"

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

	KERNEL_SOURCE="${KERNEL_PATH}/source"
	[ -e ${KERNEL_SOURCE} ] || KERNEL_SOURCE="${KERNEL_PATH}"

	cp ${KERNEL_PATH}/${SAVE_CONFIG} ${KERNEL_SOURCE}/arch/${ARCH}/configs/${DEF_CONFIG} -av && return 0

	return 1
}

function cavan-kernel-menuconfig()
{
	local DEFCONFIG

	[ "$1" ] ||
	{
		echo "Please give board name"
		return 1
	}

	export ARCH="arm"

	[ "${CMD_MAKE}" ] || CMD_MAKE="make"
	DEFCONFIG="$1_defconfig"

	echo "CMD_MAKE = ${CMD_MAKE}"
	echo "DEFCONFIG = ${DEFCONFIG}"

	${CMD_MAKE} ${DEFCONFIG} && ${CMD_MAKE} menuconfig && ${CMD_MAKE} savedefconfig || return 1
	cp -av defconfig arch/arm/configs/${DEFCONFIG} || return 1
}

function vi-then-source()
{
	vi $@ && source $@
}
