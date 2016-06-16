#!/bin/bash

function cavan-git-force-delete()
{
	git filter-branch --tree-filter "rm -rf $@" HEAD || return 1
	rm -rf .git/logs || return 1
	git reflog --all || return 1
	git prune || return 1
	git gc || return 1
}
