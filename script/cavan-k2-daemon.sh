#!/bin/sh /etc/rc.common

START=99

WGET_PATH="/tmp/wget"
WGET_URL="http://archive.openwrt.org/barrier_breaker/14.07/ramips/mt7620a/packages/packages/wget_1.16-1_ramips_24kec.ipk"

CAVAN_PATH="/tmp/cavan"
CAVAN_URL="https://gitee.com/fuangcao/shared/raw/master/cavan-main.bz2"
CAVAN_MAIN="${CAVAN_PATH}/cavan-main"

WGET="/usr/bin/wget"
WGET_SSL="${WGET_PATH}/usr/bin/wget-ssl"

download_wget_ssl()
{
	[ -f "${WGET_SSL}" ] && return 0
	mkdir -p "${WGET_PATH}" && cd "${WGET_PATH}" || return 1
	${WGET} -T 10 -cq -O wget.tar.gz "${WGET_URL}" || return 1
	tar -xzvf wget.tar.gz && tar -xzvf data.tar.gz || return 1
}

download_cavan_main()
{
	[ -f "${CAVAN_MAIN}" ] && return 0
	mkdir -p "${CAVAN_PATH}" && cd "${CAVAN_PATH}" || return 1
	${WGET_SSL} --no-check-certificate -T 10 -cq -O cavan-main.bz2 "${CAVAN_URL}" || return 1
	bunzip2 cavan-main.bz2 || return 1
	chmod 0777 cavan-main || return 1
}

cavan_daemon()
{
	while :;
	do
		download_wget_ssl && break
		sleep 20
	done

	while :;
	do
		download_cavan_main && break
		sleep 20
	done

	rm -rf "${WGET_SSL}"

	${CAVAN_MAIN} tcp_dd_server -dp 8888
	${CAVAN_MAIN} http_service -dp 8021
	${CAVAN_MAIN} web_proxy -dp 9090
	${CAVAN_MAIN} tcp_repeater -dp 8864
}

start()
{
	cavan_daemon &
}

stop()
{
	killall "cavan-main"
}
