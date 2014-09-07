#!/bin/bash

TEMP_DOWNLOAD_PATH="/tmp"

function wget_download()
{
	wget -P ${TEMP_DOWNLOAD_PATH} -t 5 -w 10 -c $1 || return 1

	return 0
}

function tftp_download()
{
	cavan-tftp_copy --ip=${SERVER_IP} --port=${SERVER_PORT} -r $1 ${TEMP_DOWNLOAD_PATH} || return 1

	return 0
}

function tcp_copy_download()
{
	cavan-tcp_copy --ip=${SERVER_IP} --port=${SERVER_PORT} -r $1 ${TEMP_DOWNLOAD_PATH} || return 1

	return 0
}

function download_main()
{
	local pkg_url pkg_name pkg_path mime_type

	[ "$1" ] || return 1

	pkg_url="$1"
	pkg_name="$(basename ${pkg_url})"
	pkg_path="${TEMP_DOWNLOAD_PATH}/${pkg_name}"

	echo "pkg_url = ${pkg_url}"
	echo "pkg_name = ${pkg_name}"
	echo "pkg_path = ${pkg_path}"

	case "${DOWNLOAD_WAY}" in
		tftp)
			tftp_download ${SERVER_SOURCE}/${pkg_name} ||
			{
				rm ${pkg_path} -rfv
				return 1
			}
			;;
		tcp_copy)
			tcp_copy_download ${SERVER_SOURCE}/${pkg_name} ||
			{
				rm ${pkg_path} -rfv
				return 1
			}
			;;
		*)
			wget_download "${pkg_url}" ||
			{
				rm ${pkg_path} -rfv
				return 1
			}
			;;
	esac

	mime_type=$(file -b --mime-type "${pkg_path}")

	echo "mime_type = ${mime_type}"

	case "${pkg_name}" in
		*.xz | *.txz)
			[ "${mime_type}" = "application/x-xz" ] || return 1
			;;
		*.bz2 | *.tbz)
			[ "${mime_type}" = "application/x-bzip2" ] || return 1
			;;
		*.gz | *.tgz)
			[ "${mime_type}" = "application/gzip" ] || return 1
			;;
		*.zip)
			[ "${mime_type}" = "application/zip" ] || return 1
			;;
		*.rar)
			[ "${mime_type}" = "application/x-rar" ] || return 1
			;;
		*)
			echo "package ${pkg_name} type unknown"
			;;
	esac

	mv -v "${pkg_path}" . || return 1

	return 0
}

download_main $* || exit 1
