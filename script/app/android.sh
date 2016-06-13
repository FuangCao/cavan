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

function cavan-apk-sign()
{
	local KEYSTORE APK_UNSIGNED APK_SIGNED

	[ "$1" ] || return 1

	APK_UNSIGNED="$1"

	if [ "$2" ]
	then
		APK_SIGNED="$2"
	else
		APK_SIGNED="${APK_UNSIGNED}-signed.apk"
	fi

	KEYSTORE="${CAVAN_HOME}/build/core/cavan.keystore"

	jarsigner -digestalg "SHA1" -sigalg "MD5withRSA" -tsa "https://timestamp.geotrust.com/tsa" -storepass "CFA8888" -keystore "${KEYSTORE}" -signedjar "${APK_SIGNED}" "${APK_UNSIGNED}" "${KEYSTORE}"
}

function cavan-apktool()
{
	java -jar "${APKTOOL_JAR}" $@
}

alias apktool="cavan-apktool"

function cavan-apk-decode()
{
	cavan-apktool d -f $@
}

function cavan-apk-encode()
{
	cavan-apktool b -f $@
}

function cavan-apk-rename()
{
	local ROOT_DIR MANIFEST PACKAGE PACKAGE_NEW APK_UNSIGNED APK_SIGNED APK_TARGET

	[ "$1" ] ||
	{
		echo "cavan-apk-rename xxxx.apk NAME NAME_NEW"
		return 1
	}

	OUT_DIR="/tmp/cavan-apk"
	echo "OUT_DIR = ${OUT_DIR}"

	rm -rf "${OUT_DIR}"

	echo "decode: $1 => ${OUT_DIR}"
	cavan-apk-decode "$1" -o "${OUT_DIR}" || return 1

	MANIFEST="${OUT_DIR}/AndroidManifest.xml"
	echo "MANIFEST = ${MANIFEST}"

	if [ "$3" ]
	then
		PACKAGE="$2"
		PACKAGE_NEW="$3"
	else
		PACKAGE=$(cat "${MANIFEST}" | grep '\bpackage="[^"]\+"' | sed 's/^.*package="\([^"]\+\)".*$/\1/g')

		if [ "$2" ]
		then
			PACKAGE_NEW="$2"
		else
			PACKAGE_NEW="com.cavan.${PACKAGE}"
		fi
	fi

	echo "rename: ${PACKAGE} => ${PACKAGE_NEW}"

	sed -i "s/\(android:name=\"\)\./\1${PACKAGE}./g" "${MANIFEST}" || return 1
	sed -i "s/\(\bpackage=\)\"${PACKAGE}\"/\1\"${PACKAGE_NEW}\"/g" "${MANIFEST}" || return 1
	sed -i "s/\(\bandroid:authorities=\"\)${PACKAGE}/\1${PACKAGE_NEW}/g" "${MANIFEST}" || return 1

	for fn in $(find "${OUT_DIR}" -name "*.smali")
	do
		sed -i "s#\(/data/data/\)${PACKAGE}#\1${PACKAGE_NEW}#g" "${fn}" || return 1
	done

	APK_UNSIGNED="${OUT_DIR}/cavan-unsigned.apk"

	echo "encode: ${OUT_DIR} => ${APK_UNSIGNED}"
	cavan-apk-encode "${OUT_DIR}" -o "${APK_UNSIGNED}" || return 1

	APK_SIGNED="${OUT_DIR}/cavan-signed.apk"

	echo "signature: ${APK_UNSIGNED} => ${APK_SIGNED}"
	cavan-apk-sign "${APK_UNSIGNED}" "${APK_SIGNED}" || return 1

	APK_TARGET="${OUT_DIR}/cavan.apk"

	echo "zipalign: ${APK_SIGNED}" "${APK_TARGET}"
	zipalign -v 4 "${APK_SIGNED}" "${APK_TARGET}" || return 1

	echo "File stored in: ${APK_TARGET}"
}
