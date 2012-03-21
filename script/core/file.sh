#!/bin/bash

function cavan_symlink()
{
	[ "$2" ] || return 0

	mkdir $(dirname "$1") -pv || return 1

	if [ -L "$2" ]
	then
		rm "$2" || return 1
	elif [ -d "$2" ]
	then
		if [ -d "$1" ]
		then
			cp "$2"/* "$1" -anv
			rm "$2" -rfv
		else
			rm "$1" -rfv
			mv "$2" "$1" -v
		fi
	fi

	[ -e "$1" ] || mkdir "$1" -p || return 1
	ln -vsf "$1" "$2" || return 1
}
