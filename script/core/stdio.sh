#!/bin/bash

CAVAN_MARK_PATH="/etc/cavan/mark"

function cavan-create_mark()
{
	mkdir ${CAVAN_MARK_PATH} -p || return 1
	echo $1 > ${CAVAN_MARK_PATH}/$1 || return 1
}

function cavan-has_mark()
{
	[ -f "${CAVAN_MARK_PATH}/$1" ] && return 0

	return 1
}

function pr_red_info()
{
	echo -e "\033[31m$*\033[0m"
}

function pr_green_info()
{
	echo -e "\033[32m$*\033[0m"
}

function pr_blue_info()
{
	echo -e "\033[34m$*\033[0m"
}

function pr_bold_info()
{
	echo -e "\033[1m$*\033[0m"
}

function pr_title()
{
	echo ============================================================
	pr_bold_info "$*"
	echo ============================================================
}
