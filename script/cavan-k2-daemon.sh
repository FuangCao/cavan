#!/bin/sh

START=99

WGET_SSL="$1"
CAVAN_PATH="/tmp/cavan"
CAVAN_URL="https://gitee.com/fuangcao/shared/raw/master/cavan-main.bz2"
CAVAN_MAIN="${CAVAN_PATH}/cavan-main"

download_cavan_main()
{
	${WGET_SSL} --no-check-certificate -T 10 -O cavan-main.bz2 "${CAVAN_URL}" || return 1
	bunzip2 cavan-main.bz2 || return 1
}

mkdir -p "${CAVAN_PATH}" && cd "${CAVAN_PATH}" || return 1
rm * -rf

while :;
do
	download_cavan_main && break
	sleep 20
done

chmod 0777 ${CAVAN_MAIN}

${CAVAN_MAIN} tcp_dd_server -dp 8888
${CAVAN_MAIN} http_service -dp 8021
${CAVAN_MAIN} web_proxy -dp 9090
${CAVAN_MAIN} tcp_repeater -dp 8864
${CAVAN_MAIN} role_change client -n K2_$(eth_mac read wan | tr -d ':') -d "free.qydev.com:4044"

rm -rf ${CAVAN_PATH}
