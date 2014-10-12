#!/system/bin/sh

SRC_PATH="${1-/data/app}"
DEST_PATH="${2-/system/app/preload}"

echo "SRC_PATH = ${SRC_PATH}"
echo "DEST_PATH = ${DEST_PATH}"

(
	cd "${SRC_PATH}" && for apk in *-[0-9].apk
	do
		[ -f "${apk}" ] || continue
		[ -L "${apk}" ] && continue

		raw_apk=${apk/-[0-9].apk/.apk}
		dest_apk="${DEST_PATH}/${raw_apk}"

		[ -f "${dest_apk}" ] || continue
		cp -av "${apk}" "${dest_apk}" && rm "${apk}" || continue
		ln -s "${dest_apk}" "${raw_apk}" || continue
	done
)
