#!/bin/bash

function cavan-go-setup()
{
	echo "GOPATH = ${GOPATH}"
	sudo mkdir -pv "${GOPATH}" || return 1
	sudo chmod -v 0777 "${GOPATH}" || return 1

	go get -v -u github.com/nsf/gocode || return 1
	go get -v -u github.com/golang/protobuf/protoc-gen-go || return 1
	echo "Successfull"
}
