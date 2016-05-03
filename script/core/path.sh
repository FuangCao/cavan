#!/bin/bash

function cavan-path-push()
{
	CAVAN_PATH_STACK[${#CAVAN_PATH_STACK[@]}]="${PWD}" && cd "$1"
}

function cavan-path-pop()
{
	local stack_top

	let stack_top="${#CAVAN_PATH_STACK[@]} - 1"
	[ "${stack_top}" -ge "0" ] ||
	{
		echo "The path stack is empty"
		return 1
	}

	cd "${CAVAN_PATH_STACK[${stack_top}]}" && unset CAVAN_PATH_STACK[${stack_top}]
}

function cavan-path-parent()
{
	local layer parent_path

	layer="$1"
	[ -n "${layer}" ] || layer="1"

	parent_path="${PWD}"

	while [ "${layer}" -gt "0" ]
	do
		parent_path=$(dirname "${parent_path}")
		let layer="${layer} - 1"
	done

	echo "${parent_path}"
}

function cavan-path-cdp()
{
	local parent_path=$(cavan-path-parent "$1")

	cavan-path-push "${parent_path}"
}

alias cavan-path-cd="cavan-path-push"

for ((i = 1; i < 10; i++))
do
	alias cdp$i="cavan-path-cdp $i"
done
