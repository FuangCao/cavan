function cavan-do-command()
{
	echo "Execute: $@"

	eval "$@" && return 0

	echo "Failed to run command: $@"

	return 1
}

function cavan-read-choise()
{
	local def_choise choise message

	if [ "$2" = "" ] || [ "$2" = "y" ] || [ "$2" = "Y" ]
	then
		def_choise="Y"
		message="$1 [Y/n]? "
	else
		def_choise="N"
		message="$1 [y/N]? "
	fi

	while :
	do
		echo -n "${message}"
		read choise
		[ "${choise}" ] || choise=${def_choise}
		case ${choise} in
			Y | y)
				return 0 ;;
			n | N)
				return 1 ;;
			*)
				echo "Please input y/Y/n/N/Enter" ;;
		esac
	done
}

function cavan-execute-ack()
{
	cavan-read-choise "$*" "Y" || return 1
	$* || return 1
	return 0
}

function cavan-configure()
{
	local command

	command="./configure --prefix=/usr --build=$(gcc -dumpmachine) --host=arm-cavan-linux-gnueabi --target=arm-cavan-linux-gnueabi $*"

	cavan-execute-ack ${command} || return 1
	return 0
}

function cavan-install()
{
	if [ "$1" ]
	then
		command="make DESTDIR=\"$1\" install"
	else
		command="make install"
	fi

	cavan-execute-ack ${command} || return 1
	return 0
}

function cavan-get-cpu-core-num()
{
	if [ "${CAVAN_OS_MAC}" = "true" ]
	then
		sysctl -n machdep.cpu.core_count
	else
		cat /proc/cpuinfo | grep "^processor\s*:" | wc -l
	fi
}

CPU_CORE_NUM="$(cavan-get-cpu-core-num)"
((${CPU_CORE_NUM} > 0)) || CPU_CORE_NUM="1"

MAKE_JOBS="${CPU_CORE_NUM}"
CMD_MAKE="make -j${MAKE_JOBS}"

alias make="${CMD_MAKE}"

export CPU_CORE_NUM MAKE_JOBS CMD_MAKE
