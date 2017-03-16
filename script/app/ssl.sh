#!/bin/bash

function cavan-openssl-create()
{
	[ "$1" = "all" ] &&
	{
		echo "generate key.pem"
		openssl genrsa -out key.pem 1024/2038 || return 1

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

function cavan-openssl-show-csr()
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

function cavan-openssl_convert-pem-pfx()
{
	openssl pkcs12 -export -out "$4" -inkey "$1" -in "$3" -certfile "$2"
}
