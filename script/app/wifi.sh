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
	local savedir="/tmp/cavan-wifi"
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

	echo "Press enter to start"
	read

	aircrack-ng -w $1 $*
}

function cavan-wifi-minidwep-gtk()
{
	sudo bash /usr/local/bin/minileafdwep/minidwep-gtk.sh
}
