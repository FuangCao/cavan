#!/bin/bash

export JACK_SERVER_VM_ARGUMENTS="-Dfile.encoding=UTF-8 -XX:+TieredCompilation -Xmx4096m"

alias shell="adb shell"
alias logcat="adb logcat -v threadtime"
alias cavan-adb-logcat-teamtalk="cavan-adb-logcat -s MoGuLogger"
alias cavan-android-cmake="cmake -DCMAKE_SYSTEM_NAME=Android -DCMAKE_SYSTEM_VERSION=18 -DCMAKE_ANDROID_ARCH=x86 -DCMAKE_ANDROID_NDK=${NDK_HOME} -DCMAKE_ANDROID_STL_TYPE=gnustl_static"
alias cavan-android-jack-kill="jack-admin kill-server"
alias cavan-android-jack-start="jack-admin start-server"
alias cavan-android-jack-restart="jack-admin kill-server && jack-admin start-server"
alias cavan-android-make="cavan-android-command ${CMD_MAKE}"

export FILE_APK_RENAME_JAR="${CAVAN_HOME}/android/app/bin/ApkRename.jar"

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

	[ -d "${android_root}" ] && cd "${android_root}/$1"
}

function cavan-android-command()
{
	[ -d "$ANDROID_BUILD_TOP" ] ||
	{
		echo "ANDROID_BUILD_TOP not set!"
		return 1
	}

	echo "ANDROID_BUILD_TOP = $ANDROID_BUILD_TOP"

	[ "${TARGET_PRODUCT}" ] ||
	{
		echo "TARGET_PRODUCT not set!"
		return 1
	}

	echo "TARGET_PRODUCT = $TARGET_PRODUCT"
	echo "$*"

	(cd "${ANDROID_BUILD_TOP}" && $*) || return 1
}

function cavan-android-lunch()
{
	local android_root="$(cavan-android-get-root)"

	echo "android_root = ${android_root}"

	[ -d "${android_root}" ] && source "${android_root}/build/envsetup.sh" &&
	{
		if [ -n "$2" ]
		then
			if [ "$1" ]
			then
				local lines=()

				for ((i = 0; i < ${#LUNCH_MENU_CHOICES[@]}; i++))
				do
					local line=${LUNCH_MENU_CHOICES[$i]}
					[[ $line == *$1-$2 ]] || continue

					local index=${#lines[@]}
					echo "$index. $line"
					lines[index]=$line
				done

				local choice=${lines[0]}

				[ ${#lines[@]} -gt 1 ] &&
				{
					local answer

					echo -n "Which would you like? [${lines[0]}] "
					read answer

					[ "$answer" ] &&
					{
						choice=${lines[$answer]}

						[ "$choice" ] ||
						{
							echo "Your choice is invalid!"
							return 1
						}
					}
				}
			else
				local products=($(cd ${android_root}/out/target/product && ls))
				local choice="${products[0]}-$2"
			fi

			echo "Lunch combo: $choice"
			lunch "$choice"
		else
			lunch "$1"
		fi
	}
}

function cavan-android-choosecombo()
{
	local android_root envsetup_sh cavan_choosecombo_sh pwd_bak

	android_root="$(cavan-android-get-root)"
	echo "android_root = ${android_root}"

	[ -d "${android_root}" ] || return 1

	envsetup_sh="${android_root}/build/envsetup.sh"
	echo "envsetup_sh = ${envsetup_sh}"

	[ -f "${envsetup_sh}" ] || return 1

	cavan_choosecombo_sh="${android_root}/cavan-choosecombo.sh"
	source "${envsetup_sh}"

	pwd_bak="${PWD}"
	cd "${android_root}" || return 1

	if [ "$1" = "force" ]
	then
		choosecombo || return 1
	elif [ -n "$1" ]
	then
		choosecombo $* || return 1
	else
		[ -f "${cavan_choosecombo_sh}" ] && source ${cavan_choosecombo_sh} || choosecombo || return 1
	fi

	cat > "${cavan_choosecombo_sh}" << EOF
#!/bin/sh

source "${envsetup_sh}"
choosecombo 1 ${TARGET_PRODUCT} ${TARGET_BUILD_VARIANT} ${TARGET_BUILD_PARTITION}
EOF

	chmod a+x "${cavan_choosecombo_sh}"

	cd "${pwd_bak}" || return 1
}

function cavan-lunch-eng()
{
	cavan-android-lunch "$1" "eng"
}

function cavan-lunch-user()
{
	cavan-android-lunch "$1" "user"
}

function cavan-lunch-userdebug()
{
	cavan-android-lunch "$1" "userdebug"
}

alias cavan-lunch-rk3288="cavan-set-jdk-version 1.7 && cavan-android-lunch rk3288-userdebug"
alias cavan-lunch-ms600="cavan-set-jdk-version 1.6 && cavan-android-lunch imx6ms600-user"
alias cavan-lunch-ococci="cavan-set-jdk-version 1.6 && cavan-android-lunch astar_ococci-eng && extract-bsp"
alias cavan-lunch-kh106="cavan-set-jdk-version 1.6 && cavan-android-lunch full_kh106-userdebug"
alias cavan-lunch-msm8916="cavan-set-jdk-version 1.7 && cavan-android-lunch msm8916_64-userdebug"

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

	# jarsigner -digestalg "SHA1" -sigalg "MD5withRSA" -tsa "https://timestamp.geotrust.com/tsa" -storepass "CFA8888" -keystore "${KEYSTORE}" -signedjar "${APK_SIGNED}" "${APK_UNSIGNED}" "${KEYSTORE}"
	jarsigner -verbose -digestalg "SHA1" -sigalg "MD5withRSA" -storepass "CFA8888" -keystore "${KEYSTORE}" -signedjar "${APK_SIGNED}" "${APK_UNSIGNED}" "${KEYSTORE}"
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

function cavan-apk-pack()
{
	local ROOT_DIR APK_UNSIGNED APK_SIGNED

	if [ "$1" ]
	then
		ROOT_DIR=$(realpath "$1")
	else
		ROOT_DIR="${PWD}"
	fi

	echo "ROOT_DIR = ${ROOT_DIR}"

	APK_UNSIGNED="${ROOT_DIR}/cavan-unsigned.apk"
	APK_SIGNED="${ROOT_DIR}/cavan-signed.apk"

	echo "encode: ${ROOT_DIR} => ${APK_UNSIGNED}"
	rm -f "${APK_UNSIGNED}"
	cavan-apk-encode -o "${APK_UNSIGNED}" "${ROOT_DIR}" || return 1

	echo "signature: ${APK_UNSIGNED} => ${APK_SIGNED}"
	rm -f "${APK_SIGNED}"
	cavan-apk-sign "${APK_UNSIGNED}" "${APK_SIGNED}" || return 1
}

function cavan-find-apk-rename()
{
	for dir in java/bin android/app/bin
	do
		APK_RENAME_JAR="${CAVAN_HOME}/${dir}/ApkRename.jar"
		[ -f "${APK_RENAME_JAR}" ] &&
		{
			echo "${APK_RENAME_JAR}"
			break
		}
	done
}

function cavan-apk-rename()
{
	local ROOT_DIR MANIFEST SUFFIX MIME_TYPE IMAGE_PATH SMALI_DIR
	local SOURCE_PKG SOURCE_RE SOURCE_DIR SOURCE_SMALI APK_RENAME_JAR
	local DEST_PKG DEST_RE DEST_DIR_DIR DEST_SMALI
	local APK_UNSIGNED APK_SIGNED APK_TARGET
	local fn step

	APK_RENAME_JAR=$(cavan-find-apk-rename)
	[ "${APK_RENAME_JAR}" ] &&
	{
		echo "APK_RENAME_JAR = ${APK_RENAME_JAR}"
		java -jar "${FILE_APK_RENAME_JAR}" $@ || return 1
		return 0
	}

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

	DEST_SMALI="${SMALI_DIR}_classes2/${DEST_DIR}"
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

	case "${SOURCE_PKG}" in
		com.qiyi.video)
			for fn in $(find "${SMALI_DIR}" -type f -name "*.smali")
			do
				sed -i "s#\(/data/data/\)${SOURCE_RE}#\1${DEST_PKG}#g" "${fn}" || return 1
				sed -i "s%^\(\s*\)invoke-virtual\s*{\s*[^,]\+,\s*[^,]\+,\s*[^,]\+,\s*\([^}]\+\)},\s*Landroid/content/res/Resources;->getIdentifier(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I%\1const-string/jumbo \2, \"${DEST_PKG}\"\n&%g" "${fn}" || return 1
			done
			;;
		*)
			for fn in $(find "${SMALI_DIR}" -type f -name "*.smali")
			do
				sed -i "s#\(/data/data/\)${SOURCE_RE}#\1${DEST_PKG}#g" "${fn}" || return 1
				sed -i "s#\"${SOURCE_RE}\"#\"${DEST_PKG}\"#g" "${fn}" || return 1
			done
			;;
	esac

	[ -d "${SOURCE_SMALI}" ] &&
	{
		mkdir -p "${DEST_SMALI}" || return 1

		echo "copy: ${SOURCE_SMALI} => ${DEST_SMALI}"
		cp -a "${SOURCE_SMALI}"/* "${DEST_SMALI}" || return 1

		for fn in $(find "${DEST_SMALI}" -type f -name "*.smali")
		do
			sed -i "s#^\(\.class\s\+L\)${SOURCE_DIR}/#\1${DEST_DIR}/#g" "${fn}" || return 1
		done
	}

	APK_UNSIGNED="${ROOT_DIR}/cavan-unsigned.apk"

	echo "encode: ${ROOT_DIR} => ${APK_UNSIGNED}"
	cavan-apk-encode "${ROOT_DIR}" -o "${APK_UNSIGNED}" || return 1

	APK_SIGNED="${ROOT_DIR}/cavan-signed.apk"

	echo "signature: ${APK_UNSIGNED} => ${APK_SIGNED}"
	cavan-apk-sign "${APK_UNSIGNED}" "${APK_SIGNED}" || return 1

	echo "zipalign: ${APK_SIGNED}" "${APK_TARGET}"
	zipalign -v 4 "${APK_SIGNED}" "${APK_TARGET}" || return 1

	echo "File stored in: ${APK_TARGET}"
}

function cavan-apk-rename-auto()
{
	local ROOT_DIR APK_DEST APK_FAILED FAILED_DIR BASE_NAME APK_RENAME_JAR

	APK_RENAME_JAR=$(cavan-find-apk-rename)
	[ "${APK_RENAME_JAR}" ] &&
	{
		echo "APK_RENAME_JAR = ${APK_RENAME_JAR}"
		java -jar "${APK_RENAME_JAR}" $@ || return 1
		return 0
	}

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

function cavan-adb-logcat()
{
	cavan-loop_run -wd2 "adb logcat -v threadtime $@" || return 1
}

function cavan-adb-logcat-runtime()
{
	cavan-adb-logcat -s "AndroidRuntime" "$@"
}

function cavan-adb-logcat-system()
{
	cavan-adb-logcat -s "System,System.err" "$@"
}

function cavan-adb-logcat-error()
{
	cavan-adb-logcat-runtime "*:e"
}

function cavan-adb-logcat-cavan()
{
	cavan-adb-logcat -s "Cavan,CavanMain"
}

function cavan-adb-loop_run()
{
	cavan-loop_run -wd2 "adb root && adb wait-for-device && adb remount; adb shell $*" || return 1
}

function cavan-adb-cavan-main()
{
	adb push ${CMD_ARM_CAVAN_MAIN} ${CMD_DATA_CAVAN_MAIN} || return 1
	adb shell "chmod 777 ${CMD_DATA_CAVAN_MAIN} && ${CMD_DATA_CAVAN_MAIN} \"$*\"" || return 1
}

function cavan-adb-tcp_dd_server()
{
	cavan-loop_run -wd2 "adb remount; adb shell cavan-main tcp_dd_server"
}

function cavan-adb-kmsg()
{
	cavan-adb-loop_run "cat /proc/kmsg" || return 1
}

function cavan-adb-build-env()
{
	adb remount || return 1
	adb push ${CMD_ARM_CAVAN_MAIN} ${CMD_SYSTEM_CAVAN_MAIN} || return 1
	adb shell chmod 06777 ${CMD_SYSTEM_CAVAN_MAIN} || return 1

	adb shell chmod 777 /data/bin/bash || return 1
	adb shell cp /data/bin/bash /system/bin/sh || return 1
}

function cavan-make-apk()
{
	[ -f Makefile ] ||
	{
		ln -sf ${CAVAN_HOME}/build/core/apk_main.mk Makefile
	}

	make
}

function cavan-mm-apk()
{
	local pkg_name apk_name

	[ -f "AndroidManifest.xml" ] || return 1

	pkg_name=$(cat AndroidManifest.xml | grep "package=" | sed 's/.*package="\([^"]\+\)"/\1/g')
	apk_name=$(basename ${PWD})

	echo "pkg_name = ${pkg_name}"

	[ -e "Android.mk" ] || cat > Android.mk << EOF
LOCAL_PATH := \$(call my-dir)

include \$(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := \$(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := ${apk_name}
LOCAL_CERTIFICATE := platform

include \$(BUILD_PACKAGE)
EOF

	[ -e "Makefile" ] || cat > Makefile << EOF
install: uninstall
	adb install \$(ANDROID_PRODUCT_OUT)/system/app/${apk_name}.apk

uninstall:
	adb uninstall ${pkg_name}

.PHONE: uninstall
EOF
}

function cavan-adb-push-directory()
{
	local fn

	cd $1 || return 1

	for fn in *
	do
		[ -f "${fn}" ] || continue
		echo "Push ${fn} => $2"
		adb push ${fn} $2 || return 1
	done

	return 0
}

function cavan-android-keystore-create()
{
	local keystore=${1-"debug.keystore"}

	keytool -v -genkey -dname "CN=Fuang Cao, OU=Cavan, O=Cavan, L=Shanghai, ST=Shanghai, C=CN" -alias androiddebugkey -storepass android -keypass android -keyalg RSA -validity 80000 -keystore "${keystore}"
}

function cavan-android-keystore-show()
{
	[ "$1" ] || return 1

	local storepass line

	if [ -n "$2" ]
	then
		storepass="$2"
	else
		storepass="android"

		echo -n "Please input storepass (${storepass}): "
		read line

		[ -n "${line}" ] && storepass="${line}"
	fi

	keytool -v -list -storepass "${storepass}" -keystore "$1"
}

function cavan-android-input-server()
{
	local src_cavan_main="${1-${CAVAN_HOME}/android/app/CavanJni/libs/armeabi/cavan-main}"
	local dest_cavan_main="/data/local/tmp/cavan-main"

	echo "wait for device"
	adb wait-for-device && adb devices || return 1

	echo "${src_cavan_main} -> ${dest_cavan_main}"
	adb push "${src_cavan_main}" "${dest_cavan_main}" || return 1
	adb shell "${dest_cavan_main} input_server -dp 9981" || return 1
}

function cavan-simple-repo-sync()
{
	local url="$1"
	local branch="${2-master}"

	while read line
	do
		echo $line

		set $line

		[ -d "$2" ] &&
		{
			(cd "$2" && git pull) && continue
			[ -d "$2/.git" ] && continue
			rm -rf "$2"
		}

		mkdir -pv $(dirname "$2") && git clone "${url}/$1" "$2" -b "${branch}"
	done < .repo/cavan.list
}

function cavan-fastboot-flash()
{
	local name image

	if [ -d "${ANDROID_BUILD_TOP}/vendor/rockchip" ]
	then
		adb reboot fastboot
	else
		adb reboot-bootloader
	fi

	for name in $*
	do
		echo "command $name"

		case $name in
			reboot | continue)
				fastboot ${name} || return 1
				continue
				;;

			aboot)
				image=${ANDROID_PRODUCT_OUT}/emmc_appsboot.mbn
				;;

			*)
				image="${ANDROID_PRODUCT_OUT}/${name}.img"
				;;
		esac

		echo "${name} <= ${image}"
		fastboot flash $name $image || return 1
	done
}

for app in Settings SecuritySettings RadioInfo AccessibilitySettings ActivityPicker ApnSettings ApplicationSettings BandMode BatteryInfo DateTimeSettings DateTimeSettingsSetupWizard DevelopmentSettings DeviceAdminSettings DeviceInfoSettings Display DisplaySettings DockSettings IccLockSettings InstalledAppDetails LanguageSettings LocalePicker LocalePickerInSetupWizard ManageApplications MasterClear MediaFormat PhysicalKeyboardSettings PrivacySettings ProxySelector RadioInfo RunningServices SecuritySettings Settings SettingsSafetyLegalActivity SoundSettings TestingSettings TetherSettings TextToSpeechSettings UsageStats UserDictionarySettings VoiceInputOutputSettings WirelessSettings
do
	alias cavan-android-open-${app}="adb shell am start com.android.settings/com.android.settings.${app}"
done
