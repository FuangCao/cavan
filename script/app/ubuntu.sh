#!/bin/bash

FILE_LSB_RELEASE="/etc/lsb-release"

source "${FILE_LSB_RELEASE}"

function cavan-ubuntu-info()
{
	cat "${FILE_LSB_RELEASE}"
}

function cavan-ubuntu-source()
{
	local server
	
	server=${1-"cn.archive.ubuntu.com"}

	for name in "main restricted" "universe" "multiverse"
	do
		echo "deb http://${server}/ubuntu/ ${DISTRIB_CODENAME} ${name}"

		for suffix in "updates" "security" "backports"
		do
			echo "deb http://${server}/ubuntu/ ${DISTRIB_CODENAME}-${suffix} ${name}"
		done
	done
}

alias cavan-ubuntu-source-aliyun="cavan-ubuntu-source mirrors.aliyun.com"
alias cavan-ubuntu-source-cn99="cavan-ubuntu-source mirrors.cn99.com"
alias cavan-ubuntu-source-sohu="cavan-ubuntu-source mirrors.sohu.com"
alias cavan-ubuntu-source-ustc="cavan-ubuntu-source mirrors.ustc.edu.cn"
alias cavan-ubuntu-source-sjtu="cavan-ubuntu-source ftp.sjtu.edu.cn"
