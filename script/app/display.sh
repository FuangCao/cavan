#!/bin/bash

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
