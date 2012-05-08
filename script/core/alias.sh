#!/bin/bash

alias vi='vim -O'
alias va='vi *.[chsS]'
alias vh='vi *.h'
alias vc='vi *.c'
alias vs='vi *.[sS]'
alias vm='vi [Mm]akefile'
alias mkconfig='make menuconfig'

CMD_TCP_DD_SERVER="${CAVAN_OUT_DEBUG}/cavan-tcp_dd_server"
CMD_TFTP_DD_SERVER="${CAVAN_OUT_DEBUG}/cavan-tftp_dd_server"

function cavan-server-run()
{
	sudo ${CMD_TCP_DD_SERVER} &
	sudo ${CMD_TFTP_DD_SERVER} &

	return 0
}

function cavan-server-stop()
{
	sudo killall ${CMD_TCP_DD_SERVER} ${CMD_TFTP_DD_SERVER}
}

function mssh()
{
	ssh user@${CAVAN_SERVER_IP}
}
