#!/bin/bash

KERNEL_HOME="${PROJECT_HOME}/kernel"
UBOOT_HOME="${PROJECT_HOME}/u-boot"
FSLDROID_HOME="${PROJECT_HOME}/fsldroid"

JAVA_HOME="${TOOLS_HOME}/jdk${JDK_VERSION}"
JRE_HOME="${JAVA_HOME}/jre"
SDK_HOME="${TOOLS_HOME}/android-sdk-linux_86"
CLASS_PATH=$(ls ${JAVA_HOME}/lib/*.jar ${JRE_HOME}/lib/*.jar | while read line; do echo -n ${line}:; done)

CAVAN_PATH="${CAVAN_HOME}/out/host/debug:${CAVAN_HOME}/script"
TOOLCHIAN_PATH="${TOOLS_HOME}/arm-cavan-linux-gnueabi/bin:${TOOLS_HOME}/gcc-4.1.2-glibc-2.5-nptl-3/arm-none-linux-gnueabi/bin"
SDK_PATH="${SDK_HOME}/platform-tools:${SDK_HOME}/tools"
JAVA_PATH="${JAVA_HOME}/bin:${JRE_HOME}/bin"
PATH="${CAVAN_PATH}:${TOOLCHIAN_PATH}:${JAVA_PATH}:${SDK_PATH}:${PATH}"

CMD_TCP_COPY='cavan-tcp_copy'
CMD_TCP_DD='cavan-tcp_dd'

CAVAN_CROSS_COMPILE="arm-cavan-linux-gnueabi-"
MAKE_JOBS="4"
MAKE_KERNEL="make -C ${KERNEL_HOME} arch=arm cross_compile=${CAVAN_CROSS_COMPILE}"
MAKE_UBOOT="make -C ${UBOOT_HOME} arch=arm cross_compile=${CAVAN_CROSS_COMPILE}"

TARGET_DEVICE="/dev/sdb"

export JAVA_HOME JRE_HOME CLASS_PATH PATH
export KERNEL_HOME UBOOT_HOME FSLDROID_HOME PRODUCT_HOME
export CAVAN_CROSS_COMPILE MAKE_JOBS TARGET_DEVICE

for fn in alias.sh file.sh
do
	. ${CAVAN_HOME}/script/core/${fn}
done
