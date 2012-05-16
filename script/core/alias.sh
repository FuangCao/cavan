#!/bin/bash

alias vi='vim -O'
alias va='vi *.[chsS]'
alias vh='vi *.h'
alias vc='vi *.c'
alias vs='vi *.[sS]'
alias vm='vi [Mm]akefile'
alias mkconfig='make menuconfig'
alias ls='ls --color=auto'
alias dir='dir --color=auto'
alias vdir='vdir --color=auto'
alias grep='grep --color=auto'
alias fgrep='fgrep --color=auto'
alias egrep='egrep --color=auto'
alias ll='ls -alF'
alias la='ls -A'
alias l='ls -CF'

CMD_TCP_DD_SERVER="${CAVAN_OUT_DEBUG}/cavan-tcp_dd_server"
CMD_TFTP_DD_SERVER="${CAVAN_OUT_DEBUG}/cavan-tftp_dd_server"

function cavan-server-run()
{
	sudo ${CMD_TCP_DD_SERVER} --daemon
	sudo ${CMD_TFTP_DD_SERVER} --daemon

	return 0
}

function cavan-server-stop()
{
	sudo killall ${CMD_TCP_DD_SERVER} ${CMD_TFTP_DD_SERVER}
}

function mssh()
{
	ssh user@${1-${CAVAN_SERVER_IP}}
}
