#!/system/bin/sh

SRC_PATH="/system/app/preload"
DEST_PATH="/data/app"
DEST_PRELOAD="${DEST_PATH}/preload"

[ -d "${DEST_PATH}" ] || mkdir -p "${DEST_PATH}" || exit 1
[ -d "${DEST_PRELOAD}" ] || mkdir -p "${DEST_PRELOAD}" || exit 1

cd "${SRC_PATH}" || exit 1

for apk in *.apk
do
	dest_preload="${DEST_PRELOAD}/${apk}"
	[ -e "${dest_preload}" ] && continue

	dest_apk="${DEST_PATH}/${apk}"
	[ -e "${dest_apk}" ] && continue

	src_apk="${SRC_PATH}/${apk}"

	echo "Symlink: ${src_apk} -> ${dest_apk}"
	ln -s "${src_apk}" "${dest_apk}" || continue

	echo "Symlink: ${src_apk} -> ${dest_preload}"
	ln -s "${src_apk}" "${dest_preload}" || continue
done 2>&1 > /dev/kmsg

cd "${DEST_PATH}" || exit 1

for apk in *.apk
do
	[ -L "${apk}" ] || continue
	[ -e "${apk}" ] && continue
	echo "Remove: ${apk}"
	rm "${apk}"
done 2>&1 > /dev/kmsg

mount -o remount,rw /system

for apk in *-[0-9].apk
do
	apk_name="${apk/-[0-9].apk/.apk}"
	apk_preload="${SRC_PATH}/${apk_name}"

	[ -e "${apk_preload}" ] || continue
	[ -L "${apk}" ] && continue

	if [ "${apk_preload}" -ot "${apk}" ]
	then
		echo "Move: ${apk} -> ${apk_preload}"
		rm "${apk_preload}" || continue
		cp -av "${apk}" "${apk_preload}" || continue
		rm "${apk}" || continue
	else
		echo "Remove: ${apk}"
		rm "${apk}" || continue
	fi

	echo "Symlink: ${apk_preload} -> ${apk_name}"
	ln -s "${apk_preload}" "${apk_name}"
done 2>&1 > /dev/kmsg

mount -o remount,ro /system
