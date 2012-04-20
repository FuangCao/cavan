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
