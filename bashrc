#!/bin/sh

find_cavan_home()
{
	[ -f "${BASH_SOURCE}" ] && realpath $(dirname "${BASH_SOURCE}") && return 0

	for fn in ${HOME}/cavan ${HOME}/work/cavan /cavan /work/cavan
	do
		[ -d "$fn" ] && echo "$fn" && return 0
	done

	echo "${PWD}"
}

[ -d "${CAVAN_HOME}" ] || CAVAN_HOME="$(find_cavan_home)"

source ${CAVAN_HOME}/script/core/bashrc.sh
