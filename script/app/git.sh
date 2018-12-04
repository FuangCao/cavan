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

function cavan-git-tag-create()
{
	local tag="$1"

	[ "${tag}" ] || tag="$(date +v%Y-%m-%d-%H%M%S)"

	echo "tag = ${tag}"

	git tag "${tag}"
}

function cavan-git-tag-push()
{
	cavan-git-push --tags
}

function cavan-git-tag-delete()
{
	local tag="$1"

	echo "tag = ${tag}"

	[ "${tag}" ] || return 1

	git tag -d "${tag}" && cavan-git-push ":refs/tags/${tag}" || return 1
}
