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
	local ROOT_DIR MANIFEST SUFFIX MIME_TYPE IMAGE_PATH SMALI
	local SOURCE_PKG SOURCE_RE SOURCE_DIR SOURCE_SMALI
	local DEST_PKG DEST_RE DEST_DIR_DIR DEST_SMALI
	local APK_UNSIGNED APK_SIGNED APK_TARGET SMALI_LIST
	local fn step

	[ "$1" ] ||
	{
		echo "cavan-apk-rename xxxx.apk NAME NAME_NEW"
		return 1
	}

	ROOT_DIR="/tmp/cavan-apk-rename"
	echo "ROOT_DIR = ${ROOT_DIR}"

	rm -rf "${ROOT_DIR}"

	echo "decode: $1 => ${ROOT_DIR}"
	cavan-apk-decode "$1" -o "${ROOT_DIR}" || return 1

	MANIFEST="${ROOT_DIR}/AndroidManifest.xml"
	echo "MANIFEST = ${MANIFEST}"

	if [ "$2" ]
	then
		APK_TARGET="$2"
	else
		APK_TARGET="${ROOT_DIR}/cavan.apk"
	fi

	if [ "$4" ]
	then
		SOURCE_PKG="$3"
		DEST_PKG="$4"
	else
		SOURCE_PKG=$(cat "${MANIFEST}" | grep '\bpackage="[^"]\+"' | sed 's/^.*package="\([^"]\+\)".*$/\1/g')

		if [ "$3" ]
		then
			DEST_PKG="$3"
		else
			DEST_PKG="com.cavan.${SOURCE_PKG}"
		fi
	fi

	echo "rename: ${SOURCE_PKG} => ${DEST_PKG}"

	SOURCE_RE=${SOURCE_PKG//./\\.}
	echo "SOURCE_RE = ${SOURCE_RE}"

	DEST_RE=${DEST_PKG//./\\.}
	echo "DEST_RE = ${DEST_RE}"

	sed -i "s/\(android:name=\"\)\./\1${SOURCE_RE}\./g" "${MANIFEST}" || return 1
	sed -i "s/\(\bpackage=\)\"${SOURCE_RE}\"/\1\"${DEST_PKG}\"/g" "${MANIFEST}" || return 1
	sed -i "s/\(\bandroid:authorities=\"\)${SOURCE_RE}/\1${DEST_PKG}/g" "${MANIFEST}" || return 1

	SMALI_DIR="${ROOT_DIR}/smali"
	echo "SMALI_DIR = ${SMALI_DIR}"

	SOURCE_DIR=${SOURCE_PKG//./\/}
	echo "SOURCE_DIR = ${SOURCE_DIR}"

	SOURCE_SMALI="${SMALI_DIR}/${SOURCE_DIR}"
	echo "SOURCE_SMALI = ${SOURCE_SMALI}"

	DEST_DIR=${DEST_PKG//./\/}
	echo "DEST_DIR = ${DEST_DIR}"

	DEST_SMALI="${SMALI_DIR}/${DEST_DIR}"
	echo "DEST_SMALI = ${DEST_SMALI}"

	for fn in $(find "${ROOT_DIR}/res" -type f -name "*.xml")
	do
		# echo "Modify file: ${fn}"
		sed -i "s#\b\(xmlns:\w\+=\"http://schemas.android.com/apk/res/\)${SOURCE_RE}#\1${DEST_PKG}#g" "${fn}" || return 1
	done

	for fn in $(find "${ROOT_DIR}/res" -type f)
	do
		MIME_TYPE=$(file -b --mime-type "${fn}")

		case "${MIME_TYPE}" in
			image/png)
				SUFFIX="png";;
			image/jpeg)
				SUFFIX="jpg";;
			image/gif)
				SUFFIX="gif";;
			*)
				continue;;
		esac

		IMAGE_PATH=$(echo "${fn}" | sed "s/\(.*\.\).*$/\1${SUFFIX}/g")
		[ "${fn}" = "${IMAGE_PATH}" ] || mv -v "${fn}" "${IMAGE_PATH}" || return 1
	done

	SMALI_LIST=$(find "${SMALI_DIR}" -type f -name "*.smali")

	for fn in ${SMALI_LIST}
	do
		# echo "Modify file: ${fn}"
		sed -i "s#\(/data/data/\)${SOURCE_RE}#\1${DEST_PKG}#g" "${fn}" || return 1
		sed -i "s#\"${SOURCE_RE}\"#\"${DEST_PKG}\"#g" "${fn}" || return 1
	done

	mkdir -p "${DEST_SMALI}" || return 1

	[ -d "${SOURCE_SMALI}" ] &&
	{
		rm -rf "${DEST_SMALI}"
		cp -a "${SOURCE_SMALI}" "${DEST_SMALI}" || return 1

		for fn in $(find "${DEST_SMALI}" -type f -name "*.smali")
		do
			sed -i "s#L${SOURCE_DIR}/#L${DEST_DIR}/#g" "${fn}" || return 1
		done
	}

	APK_UNSIGNED="${ROOT_DIR}/cavan-unsigned.apk"

	echo "encode: ${ROOT_DIR} => ${APK_UNSIGNED}"

	for step in 1 2 3 4 5
	do
		for fn in ${SMALI_LIST}
		do
			sed -i "s%^\(\s*\)invoke-virtual\s*{\s*[^,]\+,\s*[^,]\+,\s*[^,]\+,\s*\([^}]\+\)},\s*Landroid/content/res/Resources;->getIdentifier(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I%\1const-string/jumbo \2, \"${DEST_PKG}\"\n&%g" "${fn}" || return 1
		done

		cavan-apk-encode "${ROOT_DIR}" -o "${APK_UNSIGNED}" && break

		case "${step}" in
			1)
				[ -d "${DEST_SMALI}" ] || return 1

				for fn in ${DEST_SMALI}/*
				do
					case "${fn}" in
						*/R\$*.smali)
							;;
						*)
							echo "delete: ${fn}"
							rm -rf "${fn}"
							;;
					esac
				done

				for fn in ${SMALI_LIST}
				do
					sed -i "s#\"${DEST_RE}\"#\"${SOURCE_PKG}\"#g" "${fn}" || return 1
				done
				;;
			2)
				echo "delete: ${DEST_SMALI}"
				rm -rf "${DEST_SMALI}"
				;;
			*)
				return 1
				;;
		esac
	done

	APK_SIGNED="${ROOT_DIR}/cavan-signed.apk"

	echo "signature: ${APK_UNSIGNED} => ${APK_SIGNED}"
	cavan-apk-sign "${APK_UNSIGNED}" "${APK_SIGNED}" || return 1

	echo "zipalign: ${APK_SIGNED}" "${APK_TARGET}"
	zipalign -v 4 "${APK_SIGNED}" "${APK_TARGET}" || return 1

	echo "File stored in: ${APK_TARGET}"
}

function cavan-apk-rename-auto()
{
	local ROOT_DIR APK_DEST APK_FAILED FAILED_DIR BASE_NAME

	ROOT_DIR="${!#}"
	echo "ROOT_DIR = ${ROOT_DIR}"

	FAILED_DIR="${ROOT_DIR}/failure"
	echo "FAILED_DIR = ${FAILED_DIR}"

	mkdir -p "${ROOT_DIR}" || return 1
	mkdir -p "${FAILED_DIR}" || return 1

	while [ "$2" ]
	do
		echo "================================================================================"

		BASE_NAME=$(basename -s .apk "$1")
		APK_DEST="${ROOT_DIR}/${BASE_NAME}-cavan.apk"
		APK_FAILED="${FAILED_DIR}/${BASE_NAME}.apk"

		echo "rename: $1 => ${APK_DEST}"

		if [ -f "${APK_DEST}" ]
		then
			echo "skip exist file: ${APK_DEST}"
		elif cavan-apk-rename "$1" "${APK_DEST}"
		then
			rm "${APK_FAILED}"
		else
			cp -av "$1" "${APK_FAILED}" || return 1
			sleep 1
		fi

		shift
	done
}
