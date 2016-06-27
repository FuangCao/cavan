#!/bin/bash

export BREW_PROFIX=$(brew --prefix)

[ -f "${BREW_PROFIX}/etc/bash_completion" ] && source "${BREW_PROFIX}/etc/bash_completion"

function cavan-mac-install-brew()
{
	ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
}
