#!/bin/bash

KERNEL_HOME="${PROJECT_HOME}/kernel"
UBOOT_HOME="${PROJECT_HOME}/u-boot"
FSLDROID_HOME="${PROJECT_HOME}/fsldroid"

JAVA_HOME="${TOOLS_HOME}/jdk${JDK_VERSION}"
JRE_HOME="${JAVA_HOME}/jre"
SDK_HOME="${TOOLS_HOME}/${SDK_NAME}"
NDK_HOME="${TOOLS_HOME}/${NDK_NAME}"
NDKROOT="${NDK_HOME}"

[ -d ${JAVA_HOME} -o -d ${JRE_HOME} ] &&
{
	CLASSPATH=".$(find ${JAVA_HOME}/lib ${JRE_HOME}/lib -maxdepth 1 -name "*.jar" | while read line; do echo -n :${line}; done)"
}

CAVAN_TOOLCHIAN_NAME="arm-cavan-linux"
CAVAN_TOOLCHIAN_GNUEABI="${TOOLS_HOME}/${CAVAN_TOOLCHIAN_NAME}-gnueabi"
CAVAN_TOOLCHIAN_ANDROIDEABI="${TOOLS_HOME}/${CAVAN_TOOLCHIAN_NAME}-androideabi"

CAVAN_OUT_DEBUG="${CAVAN_HOME}/out/host/debug"
CAVAN_OUT_ARM="${CAVAN_HOME}/out/arm/static"
CAVAN_BIN="${CAVAN_OUT_DEBUG}:${CAVAN_HOME}/script"
TOOLCHIAN_BIN="${CAVAN_TOOLCHIAN_GNUEABI}/bin:${CAVAN_TOOLCHIAN_ANDROIDEABI}/bin"
SDK_BIN="${SDK_HOME}/platform-tools:${SDK_HOME}/tools"
NDK_BIN="${NDK_HOME}"
JAVA_BIN="${JAVA_HOME}/bin:${JRE_HOME}/bin"
FSLDROID_BIN="${FSLDROID_HOME}/out/host/linux-x86/bin"
UBOOT_BIN="${UBOOT_HOME}/tools"
PATH="${CAVAN_BIN}:${TOOLCHIAN_BIN}:${JAVA_BIN}:${SDK_BIN}:${NDK_BIN}:${FSLDROID_BIN}:${UBOOT_BIN}:${PATH}"
PYTHONPATH="${CAVAN_HOME}/script/python:${PYTHONPATH}"

for dn in ${HOME} ${HOME}/cavan-toolchian/utils
do
	PATH="${PATH}:${dn}/usr/local/bin:${dn}/usr/local/sbin:${dn}/usr/bin:${dn}/usr/sbin:${dn}/bin:${dn}/sbin"
done

CMD_TCP_COPY='cavan-tcp_copy'
CMD_TCP_DD='cavan-tcp_dd'

CAVAN_CROSS_COMPILE="arm-cavan-linux-gnueabi-"
MAKE_JOBS="4"
MAKE_KERNEL="make -C ${KERNEL_HOME} arch=arm cross_compile=${CAVAN_CROSS_COMPILE}"
MAKE_UBOOT="make -C ${UBOOT_HOME} arch=arm cross_compile=${CAVAN_CROSS_COMPILE}"

TARGET_DEVICE="/dev/sdb"

export JAVA_HOME JRE_HOME CLASSPATH PATH SDK_HOME NDK_HOME NDKROOT PYTHONPATH
export KERNEL_HOME UBOOT_HOME FSLDROID_HOME PRODUCT_HOME
export CAVAN_CROSS_COMPILE MAKE_JOBS TARGET_DEVICE CAVAN_OUT_DEBUG
export CAVAN_HOME CAVAN_SERVER_IP CAVAN_SERVER_PORT
export CAVAN_TOOLCHIAN_NAME CAVAN_TOOLCHIAN_GNUEABI CAVAN_TOOLCHIAN_ANDROIDEABI

for fn in alias.sh file.sh bash_completion application.sh ${FSLDROID_HOME}/build/envsetup.sh
do
	[ -f "${fn}" ] || fn="${CAVAN_HOME}/script/core/${fn}"
	[ -f "${fn}" ] && source ${fn}
done
