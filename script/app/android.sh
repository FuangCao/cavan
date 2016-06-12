#!/bin/bash

function cavan-android-get-root()
{
	local android_root

	if [ -d "${ANDROID_BUILD_TOP}" ]
	then
		echo "${ANDROID_BUILD_TOP}"
		return 0
	else
		android_root="${PWD}"

		while [ "${android_root}" != "/" ]
		do
			[ -e "${android_root}/frameworks/base/Android.mk" ] &&
			{
				echo "${android_root}"
				return 0
			}

			android_root=$(dirname "${android_root}")
		done
	fi

	return 1
}

function cavan-android-croot()
{
	local android_root="$(cavan-android-get-root)"

	[ -d "${android_root}" ] && cd "${android_root}"
}

function cavan-android-lunch()
{
	local android_root="$(cavan-android-get-root)"

	[ -d "${android_root}" ] && source "${android_root}/build/envsetup.sh" && lunch $1
}

alias cavan-lunch-rk3288="cavan-android-lunch rk3288-userdebug"
alias cavan-lunch-ms600="cavan-android-lunch imx6ms600-user"

function cavan-sign-update-zip()
{
	local KEY_DIR KEY_NAME FILE_SIGNAPK FILE_INPUT FILE_OUTPUT

	[ "$1" ] ||
	{
		echo "Usage: sign-update-zip update.zip [keyname]"
		return 1
	}

	FILE_INPUT="$1"
	[[ "${FILE_INPUT}" == *.zip ]] ||
	{
		echo "input file '${FILE_INPUT}' is not a .zip file"
		return 1
	}

	[ -f "${FILE_INPUT}" ] ||
	{
		echo "input file '${FILE_INPUT}' is not exists"
		return 1
	}

	FILE_OUTPUT="${FILE_INPUT%.zip}-sign.zip"
	[ -e "${FILE_OUTPUT}" ] &&
	{
		echo "output file '${FILE_OUTPUT}' is exists"
		return 1
	}

	KEY_DIR="build/target/product/security"
	[ -d "${KEY_DIR}" ] ||
	{
		echo "directory '${KEY_DIR}' is not exists"
		return 1
	}

	FILE_SIGNAPK="out/host/linux-x86/framework/signapk.jar"
	[ -f "${FILE_SIGNAPK}" ] ||
	{
		echo "file '${FILE_SIGNAPK}' is not exists"
		return 1
	}

	if [ "$2" ]
	then
		KEY_NAME="$2"
	else
		KEY_NAME="testkey"
	fi

	java -jar "${FILE_SIGNAPK}" -w "${KEY_DIR}/${KEY_NAME}.x509.pem" "${KEY_DIR}/${KEY_NAME}.pk8" "${FILE_INPUT}" "${FILE_OUTPUT}" || return 1

	return 0
}

function cavan-sign-apk()
{
	local TARGET_KEYSTORE="${CAVAN_HOME}/build/core/cavan.keystore"

	jarsigner -storepass CFA8888 -keystore ${TARGET_KEYSTORE} -signedjar ${1}-signed.apk ${1} ${TARGET_KEYSTORE}
}

alias cavan-apktool="java -jar ${APKTOOL_JAR}"
alias cavan-apktool-unpack="cavan-apktool d -f"
alias cavan-apktool-pack="cavan-apktool b -f"
