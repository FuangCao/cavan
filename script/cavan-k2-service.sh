#!/bin/sh /etc/rc.common

START=99

DOWNLOAD_PATH="/tmp/download"

WGET_URL="http://archive.openwrt.org/barrier_breaker/14.07/ramips/mt7620a/packages/packages/wget_1.16-1_ramips_24kec.ipk"
DAEMON_URL="https://gitee.com/fuangcao/cavan/raw/master/script/cavan-k2-daemon.sh"

WGET="/usr/bin/wget"
DAEMON_SH="${DOWNLOAD_PATH}/daemon.sh"
WGET_SSL="${DOWNLOAD_PATH}/usr/bin/wget-ssl"

download_wget_ssl()
{
	${WGET} -T 10 -q -O wget.tar.gz "${WGET_URL}" || return 1
	tar -xzvf wget.tar.gz && tar -xzvf data.tar.gz || return 1
}

download_daemon_sh()
{
	${WGET_SSL} --no-check-certificate -T 10 -q -O daemon.sh "${DAEMON_URL}" || return 1
}

cavan_daemon_run()
{
	mkdir -p "${DOWNLOAD_PATH}" && cd "${DOWNLOAD_PATH}" || return 1

	while :;
	do
		download_wget_ssl && break
		sleep 20
	done

	while :;
	do
		download_daemon_sh && break
		sleep 20
	done

	chmod 0777 ${DAEMON_SH} && ${DAEMON_SH} ${WGET_SSL} && rm -rf "${DOWNLOAD_PATH}"
}

start()
{
	cavan_daemon_run > /dev/null 2>&1 &
}

stop()
{
	killall "cavan-main"
}
