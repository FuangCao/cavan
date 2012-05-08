#!/bin/bash

alias vi='vim -O'
alias va='vi *.[chsS]'
alias vh='vi *.h'
alias vc='vi *.c'
alias vs='vi *.[sS]'
alias vm='vi [Mm]akefile'
alias mkconfig='make menuconfig'


function mssh()
{
	ssh user@${CAVAN_SERVER_IP}
}
