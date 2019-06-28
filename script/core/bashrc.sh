#!/bin/sh

stty -ixon

[ -d "${CAVAN_HOME}" ] ||
{
	echo "Please setenv CAVAN_HOME"
	exit 1
}

export CAVAN_OS_NAME=$(uname -s)

case "${CAVAN_OS_NAME}" in
	Darwin)
		export CAVAN_OS_MAC=true
		;;
	Linux)
		export CAVAN_OS_LINUX=true
		;;
esac

[ "${PATH_BAK}" ] || PATH_BAK="${PATH}"

GOPATH="/usr/go"
KERNEL_HOME="${PROJECT_HOME}/kernel"
UBOOT_HOME="${PROJECT_HOME}/u-boot"
FSLDROID_HOME="${PROJECT_HOME}/fsldroid"

[ -d "${TOOLS_HOME}" ] &&
{
	if [ "${JDK_VERSION}" ]
	then
		[[ ${JDK_VERSION} = [0-9]* ]] && JDK_VERSION="jdk${JDK_VERSION}"
		JAVA_HOME="${TOOLS_HOME}/${JDK_VERSION}"
	else
		JAVA_HOME="${TOOLS_HOME}/jdk"
	fi
}

JRE_HOME="${JAVA_HOME}/jre"
TOOLS_ANDROID="${TOOLS_HOME}/android"
TOOLS_GO="${TOOLS_HOME}/go"
SDK_HOME="${TOOLS_ANDROID}/sdk"
NDK_HOME="${TOOLS_ANDROID}/ndk"
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
APKTOOL_JAR="${TOOLS_ANDROID}/apktool.jar"
JAVA_BIN="${JAVA_HOME}/bin:${JRE_HOME}/bin"
FSLDROID_BIN="${FSLDROID_HOME}/out/host/linux-x86/bin"
UBOOT_BIN="${UBOOT_HOME}/tools"
PATH="${CAVAN_BIN}:${TOOLCHIAN_BIN}:${JAVA_BIN}:${SDK_BIN}:${NDK_BIN}:${FSLDROID_BIN}:${UBOOT_BIN}:${GOPATH}/bin:${PATH}"
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

FILE_CAVAN_SH="${HOME}/.cavan.sh"

[ -f "${FILE_CAVAN_SH}" ] && source "${FILE_CAVAN_SH}"

export JAVA_HOME JRE_HOME CLASSPATH PATH PATH_BAK SDK_HOME NDK_HOME NDKROOT PYTHONPATH GOPATH
export KERNEL_HOME UBOOT_HOME FSLDROID_HOME PRODUCT_HOME APKTOOL_JAR
export CAVAN_CROSS_COMPILE MAKE_JOBS TARGET_DEVICE CAVAN_OUT_DEBUG
export CAVAN_HOME CAVAN_SERVER_IP CAVAN_SERVER_PORT TOOLS_HOME
export CAVAN_TOOLCHIAN_NAME CAVAN_TOOLCHIAN_GNUEABI CAVAN_TOOLCHIAN_ANDROIDEABI FILE_CAVAN_SH

for pem in ${HOME}/.ssh/*.pem
do
	[ -f "${pem}" ] && chmod 0600 "${pem}" && ssh-add "${pem}" &&
	{
		export SSH_SERVERS="$(basename ${pem/.pem}) ${SSH_SERVERS}"
	}
done > /dev/null 2>&1

for fn in alias.sh file.sh bash_completion application.sh device.sh build.sh stdio.sh path.sh
do
	fn="${CAVAN_HOME}/script/core/${fn}"
	[ -f "${fn}" ] && source "${fn}"
done

for fn in ${CAVAN_HOME}/script/app/*.sh
do
	[ -f "${fn}" ] && source "${fn}"
done

[ "${CAVAN_OS_MAC}" = "true" ] && for fn in ${CAVAN_HOME}/script/mac/*.sh
do
	[ -f "${fn}" ] && source "${fn}"
done

unset JAVA_TOOL_OPTIONS
