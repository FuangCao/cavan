#!/bin/bash

function cavan-openssl-create()
{
	[ "$1" = "all" ] &&
	{
		echo "generate key.pem"
		openssl genrsa -out key.pem 1024 || return 1

		echo "generate cert.csr"
		openssl req -new -key key.pem -out cert.csr || return 1

		echo "generate cert.pem"
		openssl req -new -x509 -key key.pem -out cert.pem -days 1095 || return 1
	}

	echo "generate cert.jks"
	rm cert.jks
	keytool -import -trustcacerts -file cert.pem -storetype JKS -keystore cert.jks || return 1

	rm cert.bks
	echo "generate cert.bks"
	keytool -provider org.bouncycastle.jce.provider.BouncyCastleProvider -import -trustcacerts -file cert.pem -storetype BKS -keystore cert.bks || return 1
}

function cavan-openssl-verify()
{
	openssl verify "$1"
}

function cavan-openssl-show-req()
{
	openssl req -noout -text -in "$1"
}

function cavan-openssl-show-key()
{
	openssl rsa -noout -text -in "$1"
}

function cavan-openssl-show-cert
{
	openssl x509 -noout -text -in "$1"
}

function cavan-openssl-convert-pfx-pem()
{
	openssl pkcs12 -in "$1" -out "$2" -nodes
}

function cavan-openssl-convert-pem-der()
{
	openssl x509 -outform der -in "$1" -out "$2"
}

function cavan-openssl-convert-pem-crt()
{
	openssl x509 -outform der -in "$1" -out "$2"
}

function cavan-openssl-convert-pem-key()
{
	openssl RSA -in "$1" -out "$2"
}

function cavan-openssl-convert-der-pem()
{
	openssl x509 -inform der -in "$1" -out "$2"
}

function cavan-openssl-convert-crt-pem()
{
	openssl x509 -inform der -in "$1" -out "$2"
}

function cavan-openssl-convert-pem-pfx()
{
	openssl pkcs12 -export -password "pass:CFA8888" -out "$3" -inkey "$1" -in "$2"
}

function cavan-ca-req()
{
	[ "$3" ] ||
	{
		echo "$0 key.pem req.pem www.cavan.com"
		return 1
	}

	openssl genrsa -out "$1" 2048 || return 1
	openssl req -new -days 3650 -key "$1" -out "$2" -subj "/C=CN/ST=Shanghai/L=Shanghai/CN=$3/O=Cavan/OU=Software" || return 1
}

function cavan-ca-sign()
{
	openssl ca -in "$1" -out "$2" -batch || return 1
	cavan-openssl-show-cert "$2" || return 1
}

function cavan-ca-selfsign()
{
	openssl ca -selfsign -in "$1" -out "$2" -extensions v3_ca -batch || return 1
	cavan-openssl-show-cert "$2" || return 1
}

function cavan-ca-init()
{
	mkdir -pv ./demoCA/private ./demoCA/newcerts || return 1
	touch ./demoCA/index.txt || return 1
	echo 01 > ./demoCA/serial || return 1
	cavan-ca-req ./demoCA/private/cakey.pem ./demoCA/careq.pem "www.cavan.com" || return 1
	cavan-ca-selfsign ./demoCA/careq.pem ./demoCA/cacert.pem || return 1
	cavan-openssl-convert-pem-pfx ./demoCA/private/cakey.pem ./demoCA/cacert.pem ./demoCA/cacert.pfx || return 1
}

function cavan-ca-gen()
{
	local path

	[ "$1" ] ||
	{
		echo "$0 baidu.com"
		return 1
	}

	path="./$1"

	mkdir -pv "${path}" || return 1
	[ -f "${path}/req.pem" ] || cavan-ca-req "${path}/key.pem" "${path}/req.pem" "*.$1" || return 1
	[ -f "${path}/cert.pem" ] || cavan-ca-sign "${path}/req.pem" "${path}/cert.pem" || return 1
	[ -f "${path}/cert.pfx" ] || cavan-openssl-convert-pem-pfx "${path}/key.pem" "${path}/cert.pem" "${path}/cert.pfx" || return 1
}
