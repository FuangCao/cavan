#!/bin/bash

export BREW_PROFIX=$(brew --prefix)
export FILE_GIT_COMPLETION="${HOME}/.git-completion.bash"

for fn in ${BREW_PROFIX}/etc/bash_completion ${FILE_GIT_COMPLETION}
do
	[ -f "${fn}" ] && source "${fn}"
done

function cavan-mac-install-brew()
{
	ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
}

function cavan-mac-setup()
{
	brew install bash-completion
	curl https://raw.githubusercontent.com/git/git/master/contrib/completion/git-completion.bash > "${FILE_GIT_COMPLETION}"
}
