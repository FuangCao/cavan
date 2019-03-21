#!/bin/bash

export UMAKE_HOME="${TOOLS_HOME}/umake"

FILE_LSB_RELEASE="/etc/lsb-release"

[ -f "${FILE_LSB_RELEASE}" ] && source "${FILE_LSB_RELEASE}"

function cavan-ubuntu-info()
{
	cat "${FILE_LSB_RELEASE}"
}

function cavan-apt-get-source()
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

alias cavan-apt-get-source-aliyun="cavan-apt-get-source mirrors.aliyun.com"
alias cavan-apt-get-source-cn99="cavan-apt-get-source mirrors.cn99.com"
alias cavan-apt-get-source-sohu="cavan-apt-get-source mirrors.sohu.com"
alias cavan-apt-get-source-ustc="cavan-apt-get-source mirrors.ustc.edu.cn"
alias cavan-apt-get-source-sjtu="cavan-apt-get-source ftp.sjtu.edu.cn"
alias cavan-apt-get-source-us="cavan-apt-get-source us.archive.ubuntu.com"

function cavan-nginx-restart()
{
	sudo killall nginx

	if $(sudo nginx)
	then
		echo "start nginx successfull"
	else
		echo "start nginx failed"
	fi
}

function cavan-umake-setup()
{
	sudo add-apt-repository ppa:ubuntu-desktop/ubuntu-make || return 1
	sudo apt-get update || return 1
	sudo apt-get install ubuntu-make || return 1
}

alias cavan-umake-install-vscode="umake ide visual-studio-code"
alias vscode="${UMAKE_HOME}/ide/visual-studio-code/bin/code"
