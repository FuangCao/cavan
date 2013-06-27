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
alias grep='grep --color=auto'
alias fgrep='fgrep --color=auto'
alias egrep='egrep --color=auto'
alias ll='ls -alF'
alias la='ls -A'
alias l='ls -CF'
alias mkae='make'
alias mkea='make'
alias cavan-daemon-command='cavan-service -s 0 --'

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
	cavan-loop_run -wd2 "adb logcat $*" || return 1
}

function cavan-adb-kmsg()
{
	cavan-adb-loop_run "cat /proc/kmsg" || return 1
}

function cavan-adb-build-env()
{
	adb remount || return 1
	adb push ${CMD_ARM_CAVAN_MAIN} ${CMD_SYSTEM_CAVAN_MAIN} || return 1
	adb shell chmod 777 ${CMD_SYSTEM_CAVAN_MAIN} || return 1

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
