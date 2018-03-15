#!/bin/bash

function cavan-cross-config()
{
	local command

	command="./configure --prefix=/usr --build=$(gcc -dumpmachine) --target=arm-cavan-linux-gnueabi $*"
	cavan-read-choise "Command = \"${command}\"" "Y" || return 1
	command || return 1

	return 0
}

function cavan-app-install()
{
	local command

	if [ "$1" ]
	then
		command="make DESTDIR=$1 install"
	else
		command="make install"
	fi

	cavan-read-choise "Command = \"${command}\"" "Y" || return 1
	command || return 1

	return 0
}

function cavan-git-daemon-run()
{
	local PORT BASE_PATH COMMAND

	PORT=${1-"7777"}
	BASE_PATH=${2-${HOME}/git}

	mkdir ${BASE_PATH} -p || exit 1

	echo "BASE_PATH = ${BASE_PATH}"
	echo "PORT = ${PORT}"

	COMMAND="$(git --exec-path)/git-daemon --verbose --port=${PORT} --export-all --enable=receive-pack --enable=upload-pack --enable=upload-archive --base-path=${BASE_PATH}"

	if which cavan-service > /dev/null
	then
		cavan-service --start -s 0 --exec "${COMMAND}"
	else
		${COMMAND}
	fi
}

function cavan-daemon-run()
{
	cavan-tcp_dd_server -ds0
	cavan-tcp_dd_server -ds0 --udp

	while cavan-alarm remove 0 -l 2>&1 > /dev/null
	do
		echo "Remove a alarm succesfually"
	done

	cavan-alarm add cavan-sprd-compile ${CAVAN_HOME}/script/configs/project.cfg -lt 23:00 -r 1d
	cavan-alarm list -l

	cavan-tcp_proxy -adp 9999
	cavan-tcp_proxy --daemon --host mirrors.ustc.edu.cn --pport 80 --port 6666
	squid
	distccd --allow "0.0.0.0/0" --port 3333 --daemon
	cavan-web_proxy -dp 9090 --host mirrors.ustc.edu.cn
	cavan-git-daemon-run

	return 0
}

function cavan-apk-rename()
{
	local fn nfn

	for fn in *-[0-9].apk
	do
		nfn="${fn%-*.apk}.apk"
		mv -v "${fn}" "${nfn}"
	done
}

function cavan-android-buildenv()
{
	export TARGET_GCC_VERSION_EXP="${1-4.7.4}-$(uname -m)"

	echo "TARGET_GCC_VERSION_EXP = ${TARGET_GCC_VERSION_EXP}"

	TOOLCHIAN_GNUEABI="${CAVAN_TOOLCHIAN_GNUEABI}-${TARGET_GCC_VERSION_EXP}"
	TOOLCHIAN_ANDROIDEABI="${CAVAN_TOOLCHIAN_ANDROIDEABI}-${TARGET_GCC_VERSION_EXP}"

	echo "TOOLCHIAN_GNUEABI = ${TOOLCHIAN_GNUEABI}"
	echo "TOOLCHIAN_ANDROIDEABI = ${TOOLCHIAN_ANDROIDEABI}"

	[ -d "${TOOLCHIAN_GNUEABI}" ] && [ -d "${TOOLCHIAN_ANDROIDEABI}" ] ||
	{
		echo "${TOOLCHIAN_GNUEABI} or ${TOOLCHIAN_ANDROIDEABI} not found"
		return 1
	}

	DIR_ANDROID_PREBUILT_ARM_GCC="prebuilts/gcc/linux-x86/arm"

	(
		cd "${DIR_ANDROID_PREBUILT_ARM_GCC}" &&
		{
			rm -f "arm-eabi-${TARGET_GCC_VERSION_EXP}" && ln -vsf "${TOOLCHIAN_GNUEABI}" "arm-eabi-${TARGET_GCC_VERSION_EXP}" || return 1
			rm -f "arm-linux-androideabi-${TARGET_GCC_VERSION_EXP}" ] && ln -vsf "${TOOLCHIAN_ANDROIDEABI}" "arm-linux-androideabi-${TARGET_GCC_VERSION_EXP}" || return 1
		}
	)

	for DIR_INCLUDE_SYS in $(find /usr/include/ -maxdepth 2 -type d -name sys)
	do
		FILE_CAPABILITY_H="${DIR_INCLUDE_SYS}/capability.h"

		echo "DIR_INCLUDE_SYS = ${DIR_INCLUDE_SYS}"
		echo "FILE_CAPABILITY_H = ${FILE_CAPABILITY_H}"

		[ -f "${FILE_CAPABILITY_H}" ] || su -c "echo '#include <sys/resource.h>' > ${FILE_CAPABILITY_H}"
	done

	export HOST_TOOLCHAIN_PREFIX="prebuilts/gcc/linux-x86/host/i686-linux-glibc2.7-4.6"

	echo "HOST_TOOLCHAIN_PREFIX = ${HOST_TOOLCHAIN_PREFIX}"
}

function cavan-set-jdk-version()
{
	[ "$1" ] || return 1

	if [[ $1 = jdk* ]]
	then
		JDK_VERSION="$1"
	else
		JDK_VERSION="jdk$1"
	fi

	[ -d "/tools/${JDK_VERSION}" ] || for fn in /tools/${JDK_VERSION}*
	do
		[ -d "${fn}" ] && JDK_VERSION="$(basename ${fn})"
	done

	[ -d "/tools/${JDK_VERSION}" ] ||
	{
		[ -d "/tools" ] && return 1
		return 0
	}

	echo "JDK_VERSION = ${JDK_VERSION}"

	[ "${PATH_BAK}" ] && PATH="${PATH_BAK}"

	source ${CAVAN_HOME}/script/core/bashrc.sh
}

function cavan-chdir-by-file-simple()
{
	[ "$1" ] || return 1

	while [ "${PWD}" != "/" ]
	do
		[ -f "$1" ] && return 0
		cd .. || break
	done

	return 1
}

function cavan-chdir-by-file()
{
	local pwd_logical pwd_physical

	pwd_logical=$(pwd -L)
	pwd_physical=$(pwd -P)

	cavan-chdir-by-file-simple "$1" && return 0
	cd "${pwd_physical}" && cavan-chdir-by-file-simple "$1" && return 0

	cd "${pwd_logical}"

	return 1
}

function cavan-get-root-by-file()
{
	(cavan-chdir-by-file $1 && pwd)
}

function cavan-get-android-root()
{
	local pathname

	pathname=$(cavan-get-root-by-file "build/envsetup.sh") && CAVAN_ANDROID_ROOT="${pathname}" ||
	{
		[ -d "${CAVAN_ANDROID_ROOT}" ] || return 1
		pathname="${CAVAN_ANDROID_ROOT}"
	}

	export CAVAN_ANDROID_ROOT

	echo "${pathname}"

	return 0
}

function cavan-is-android-root()
{
	[ -f "build/envsetup.sh" ]
}

function cavan-get-kernel-root()
{
	local pathname

	pathname=$(cavan-get-root-by-file "include/linux/kernel.h") && CAVAN_KERNEL_ROOT="${pathname}" ||
	{
		[ -d "${CAVAN_KERNEL_ROOT}" ] || return 1
		pathname="${CAVAN_KERNEL_ROOT}"
	}

	export CAVAN_KERNEL_ROOT

	echo "${pathname}"

	return 0
}

function cavan-is-android-root()
{
	[ -f "include/linux/kernel.h" ]
}

function cavan-chdir-android()
{
	local pathname

	pathname=$(cavan-get-android-root) || return 1
	cd "${pathname}" && return 0

	return 1
}

function cavan-chdir-kernel()
{
	local pathname

	pathname=$(cavan-get-kernel-root) || return 1
	cd "${pathname}" && return 0

	return 1
}

function cavan-save-cavan-sh()
{
	echo "FILE_CAVAN_SH = ${FILE_CAVAN_SH}"

	cat > "${FILE_CAVAN_SH}" << EOF
ADB_HOST="${ADB_HOST}"
ADB_PORT="${ADB_PORT}"
CMD_ADB_TCP_DD="${CMD_ADB_TCP_DD}"
CMD_ADB_TCP_COPY="${CMD_ADB_TCP_COPY}"
CMD_ADB_TCP_EXEC="${CMD_ADB_TCP_EXEC}"

alias cavan-adb-tcp_dd="\${CMD_ADB_TCP_DD}"
alias cavan-adb-tcp_exec="\${CMD_ADB_TCP_EXEC}"
alias cavan-adb-shell="\${CMD_ADB_TCP_EXEC}"
alias cavan-adb-reboot="\${CMD_ADB_TCP_EXEC} reboot"

export ADB_HOST ADB_PORT
export CMD_ADB_TCP_DD CMD_ADB_TCP_COPY CMD_ADB_TCP_EXEC
EOF
}

function cavan-mm-push()
{
	local kernel_root file_list file_config

	[ -f "${FILE_CAVAN_SH}" ] && source "${FILE_CAVAN_SH}"
	[ "${ADB_PORT}" ] || ADB_PORT="9999"

	if [ "${ADB_HOST}" ]
	then
		CMD_ADB_TCP_DD="cavan-tcp_dd -wi ${ADB_HOST} -p ${ADB_PORT}"
		CMD_ADB_TCP_COPY="cavan-tcp_copy -wi ${ADB_HOST} -p ${ADB_PORT}"
		CMD_ADB_TCP_EXEC="cavan-tcp_exec -i ${ADB_HOST} -p ${ADB_PORT}"
	else
		CMD_ADB_TCP_DD="cavan-tcp_dd -wa"
		CMD_ADB_TCP_COPY="cavan-tcp_copy -wa"
		CMD_ADB_TCP_EXEC="cavan-tcp_exec -a"
	fi

	cavan-save-cavan-sh

	kernel_root=$(cavan-get-kernel-root)

	if [ "${kernel_root}" ]
	then
		file_config="${kernel_root}/.git/build.conf"

		[ -f "${file_config}" ] && source "${file_config}"

		KERNEL_NAME="$(basename ${kernel_root})"

		if [ "$1" ]
		then
			KERNEL_CONFIG="$1"
		elif [ -z "${KERNEL_CONFIG}" ]
		then
			if [ "${KERNEL_NAME}" = "kernel_imx" ]
			then
				KERNEL_CONFIG="ms600"
			else
				KERNEL_CONFIG="jw100"
			fi
		fi

		KERNEL_HOME="${kernel_root}"

		{
			echo "KERNEL_NAME=\"${KERNEL_NAME}\""
			echo "KERNEL_HOME=\"${KERNEL_HOME}\""
			echo "KERNEL_CONFIG=\"${KERNEL_CONFIG}\""
		} | tee "${file_config}"

		(
			cd "${KERNEL_HOME}" || return 1

			case "${KERNEL_CONFIG}" in
				ms600|imx6ms600)
					cavan-build-ms600 || return 1
					${CMD_ADB_TCP_DD} --auto .git/boot.img || return 1
					;;
				*)
					make ${KERNEL_CONFIG}_defconfig || return 1
					make ${KERNEL_CONFIG}.img -j${MAKE_JOBS} && ${CMD_ADB_TCP_DD} --auto kernel.img resource.img || return 1
					;;
			esac
		) || return 1

		export KERNEL_NAME KERNEL_HOME KERNEL_CONFIG
	else
		file_list=$(mm -j${MAKE_JOBS} | cavan-tee | awk -F ' *: *'  '/^(Install|target Symbolic):/ { print $2 }' | sed 's/.*(\(\S.*\S\)).*$/\1/g' | uniq; [ "${PIPESTATUS[0]}" = "0" ]) || return 1
		cavan-android-push ${file_list} || return 1
	fi

	return 0
}

function cavan-mm-push-reboot()
{
	cavan-mm-push $@ || return 1
	adb reboot || ${CMD_ADB_TCP_EXEC} reboot || return 1

	return 0
}

function cavan-android-auto-push()
{
	local system_path src_path dest_path

	if [ "$1" ]
	then
		system_path=$1
	elif [ "${ANDROID_PRODUCT_OUT}" ]
	then
		system_path="${ANDROID_PRODUCT_OUT}/system"
	else
		system_path="."
	fi

	echo "system_path = ${system_path}"

	adb start-server || return 1

	(
		cd "${system_path}" || return 1

		[ -f "build.prop" -a -f "lib/libandroid_runtime.so" ] ||
		{
			echo "This path is not system"
			return 1
		}

		cavan-inotify . | while read line
		do
			adb wait-for-device && adb root || continue
			adb wait-for-device && adb remount || continue

			src_path="${line:2}"
			dest_path="/system/${src_path}"

			echo "push: ${src_path} => ${dest_path}"
			adb push "${src_path}" "${dest_path}"
		done
	)
}

function make2()
{
    local start_time=$(date +"%s")
    command make "$@"
    local result=$?
    local end_time=$(date +"%s")
    local time_diff=$(($end_time - $start_time))
    local hours=$(($time_diff / 3600 ))
    local mins=$((($time_diff % 3600) / 60))
    local secs=$(($time_diff % 60))

    echo

    if [ $result -eq 0 ]
	then
        echo -n -e "#### make completed successfully "
    else
        echo -n -e "#### make failed to build some targets "
    fi

    if [ $hours -gt 0 ]
	then
        printf "(%02g:%02g:%02g (hh:mm:ss))" $hours $mins $secs
    elif [ $mins -gt 0 ]
	then
        printf "(%02g:%02g (mm:ss))" $mins $secs
    elif [ $secs -gt 0 ]
	then
        printf "(%s seconds)" $secs
    fi

    echo -e " ####"

    return $result
}

function cavan-merge-checkout()
{
	local branch

	[ "$1" ] && branch="$1"
	[ "${branch}" ] || branch="MERGE_HEAD"

	echo "branch = ${branch}"

	git status . | awk -F ' *: *' '/both (added|modified):/ { print $2 }' | while read line
	do
		echo "checkout $line"
		git checkout "${branch}" "$line" || return 1
	done
}

alias cavan-merge-checkout-them='cavan-merge-checkout MERGE_HEAD'
alias cavan-merge-checkout-mine='cavan-merge-checkout HEAD'

function cavan-merge-auto-checkout()
{
	[ "$1" ] && CAVAN_THEM_BRANCH="$1"
	[ "${CAVAN_THEM_BRANCH}" ] || CAVAN_THEM_BRANCH="MERGE_HEAD"

	echo "CAVAN_THEM_BRANCH = ${CAVAN_THEM_BRANCH}"

	git status . | awk -F ' *: *' '/both (added|modified):/ { print $2 }' | while read line
	do
		if git log "$line" | grep Fuang
		then
			echo "skipping $line"
		else
			echo "checkout $line"
			git checkout "${CAVAN_THEM_BRANCH}" "$line" || return 1
		fi
	done

	export CAVAN_THEM_BRANCH
}

function cavan-tc3587xx-inotify()
{
	local src_file dest_file

	[ "$1" ] ||
	{
		echo "Usage: $0 <DEST_FILE>"
		echo "Usage: $0 <SRC_FILE> <DEST_FILE>"
		return 1
	}

	if [ "$2" ]
	then
		src_file="$1"
		dest_file="$2"
	else
		src_file="/tmp/$(basename $1).txt"
		dest_file="$1"
	fi

	echo "${src_file} => ${dest_file}"

	touch "${src_file}"

	cavan-inotify ${src_file} -c "cavan-tc3587xx-converter ${src_file} ${dest_file}"
}

function cavan-gen-vimrc()
{
	cat << EOF
let CAVAN_VIMRC = \$CAVAN_HOME . '/script/vim/vimrc.vim'
let Tlist_Ctags_Cmd = '/usr/bin/ctags'

if filereadable(CAVAN_VIMRC)
  execute 'source ' . CAVAN_VIMRC
endif
EOF
}
