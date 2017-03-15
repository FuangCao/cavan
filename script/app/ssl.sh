#!/bin/bash

function cavan-openssl-create()
{
	openssl genrsa -out key.pem 1024/2038 || return 1
	openssl req -new -key key.pem -out cert.csr || return 1
	openssl req -new -x509 -key key.pem -out cert.pem -days 1095 || return 1
}

function cavan-openssl-verify()
{
	openssl verify $1
}

function cavan-openssl-show-csr()
{
	openssl req -noout -text -in $1
}

function cavan-openssl-show-key()
{
	openssl rsa -noout -text -in $1
}

function cavan-openssl-show-cert
{
	openssl x509 -noout -text -in $1
}
