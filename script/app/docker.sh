#!/bin/bash

alias cavan-docker-install="wget -qO- https://get.docker.com | sh"
alias cavan-docker-alpine="cavan-docker-run alpine"
alias cavan-docker-ubuntu="cavan-docker-run ubuntu"
alias cavan-docker-clean="docker system prune"
alias cavan-docker-images="docker images"
alias cavan-docker-ps="docker ps -a"
alias cavan-docker-start="docker start"
alias cavan-docker-push="docker push"
alias cavan-docker-tag="docker tag"

function cavan-docker-run()
{
	[ "$1" ] && docker run -v /:/rootfs -v /docker:/work -it $1
}

function cavan-docker-exec()
{
	[ "$1" ] && docker exec -it $1 sh
}

function cavan-docker-build()
{
	[ "$1" ] && docker build -t $1 .
}

function cavan-docker-attach()
{
	[ "$1" ] && docker start $1 && docker attach $1
}

function cavan-docker-insecure-registries()
{
	[ "$1" ] && echo "{\"insecure-registries\":[\"$1\"]}"
}
