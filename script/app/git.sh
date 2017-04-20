#!/bin/bash

function cavan-git-force-delete()
{
	git filter-branch --tree-filter "rm -rf $@" HEAD || return 1
	rm -rf .git/logs || return 1
	git reflog --all || return 1
	git prune || return 1
	git gc || return 1
}

function cavan-git-push()
{
	git remote -v | awk '{print $1}' | uniq | while read remote
	do
		echo "remote = ${remote}"
		git push "${remote}" $@
	done
}
