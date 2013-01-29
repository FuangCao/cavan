#!/bin/bash

function cavan-read-choise()
{
	local def_choise choise message

	if [ "$2" = "" ] || [ "$2" = "y" ] || [ "$2" = "Y" ]
	then
		def_choise="Y"
		message="$1 [Y/n]? "
	else
		def_choise="N"
		message="$1 [y/N]? "
	fi

	while :
	do
		echo -n "${message}"
		read choise
		[ "${choise}" ] || choise=${def_choise}
		case ${choise} in
			Y | y)
				return 0 ;;
			n | N)
				return 1 ;;
			*)
				echo "Please input y/Y/n/N/Enter" ;;
		esac
	done
}

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
	PORT=${1-"7777"}
	BASE_PATH=${2-${HOME}/git}

	mkdir ${BASE_PATH} -p || exit 1

	echo "BASE_PATH = ${BASE_PATH}"
	echo "PORT = ${PORT}"

	`git --exec-path`/git-daemon --verbose --port=${PORT} --export-all --enable=receive-pack --enable=upload-pack --enable=upload-archive --base-path=${BASE_PATH}
}

function cavan-daemon-run()
{
	cavan-tcp_dd_server -ds0
	cavan-tcp_proxy -adp 9999
	cavan-tcp_proxy --daemon --pip 123.58.173.89 --pport 80 --port 6666
	cavan-git-daemon-run 7777 &

	return 0
}
