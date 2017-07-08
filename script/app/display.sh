#!/bin/bash

alias cavan-xorg-start="sudo service lightdm start"
alias cavan-xorg-stop="sudo service lightdm stop"
alias cavan-xorg-configure="sudo Xorg -configure"
alias cavan-xorg-config="sudo X -config"

function cavan-display-list()
{
	xrandr | grep "^\S\+\s\+connected" | awk '{ print $1 }' || return 1
}

function cavan-display-get-mode()
{
	cvt "$1" "$2" | grep "^Modeline\b" | sed 's/Modeline\s\+//g' || return 1
}

function cavan-display-add-resolution()
{
	local display mode name

	mode=$(cavan-display-get-mode $@)
	[ "${mode}" ] || return 1

	name=$(echo ${mode} | awk '{ print $1 }')

	echo "Mode: ${mode}"
	echo "Name: ${name}"

	echo "New Mode"
	xrandr --newmode ${mode}

	for display in $(cavan-display-list)
	do
		echo "Add Mode"
		xrandr --addmode "${display}" "${name}" || return 1
		echo "Output"
		xrandr --output "${display}" --mode "${name}" || return 1
	done
}

function cavan-xorg-gen-config()
{
	cat << EOF
Section "Device"
	Identifier	"Configured Video Device"
EndSection

Section "Monitor"
	Identifier	"Configured Monitor"
EOF

	{
		cvt 1920 1080
		cvt 1280 720
	} | sed 's/^/\t/g'

	cat << EOF
EndSection

Section "Screen"
	Identifier	"Default Screen"
	Monitor		"Configured Monitor"
	Device		"Configured Device"
EndSection
EOF
}

function cavan-xorg-setup-x11r6()
{
	local x11r6="/usr/X11R6"

	mkdir -pv "${x11r6}" || return 1

	for fn in lib lib64
	do
		rm "${x11r6}/${fn}"
		ln -vsf "/usr/lib/xorg" "${x11r6}/${fn}" || return 1
	done
}

function cavan-display-install-mga-driver()
{
	cavan-xorg-setup-x11r6 || return 1
	sed -e 's#^\(\s*\)GetXPath#\1export XPATH=\"/usr/X11R6\"#g' -e 's#^\(\s*\)GetXVersion#\1export XPRESENT=1\n\1export XVERSION=\"7.0.0\"#g' -i install.sh || return 1
	bash install.sh || return 1
}
