#!/bin/bash

function cavan-virtualbox-osx-fixup()
{
	local vm_name cmd_set_data

	[ "$1" ] ||
	{
		echo "cavan-virtualbox-osx-fixup <VM>"
		return 1
	}

	vm_name="$1"
	cmd_set_data="VBoxManage setextradata ${vm_name}"

	VBoxManage modifyvm "${vm_name}" --cpuidset 00000001 000106e5 00100800 0098e3fd bfebfbff || return 1
	${cmd_set_data} "VBoxInternal/Devices/efi/0/Config/DmiSystemProduct" "iMac11,3" || return 1
	${cmd_set_data} "VBoxInternal/Devices/efi/0/Config/DmiSystemVersion" "1.0" || return 1
	${cmd_set_data} "VBoxInternal/Devices/efi/0/Config/DmiBoardProduct" "Iloveapple" || return 1
	${cmd_set_data} "VBoxInternal/Devices/smc/0/Config/DeviceKey" "ourhardworkbythesewordsguardedpleasedontsteal(c)AppleComputerInc" || return 1
	${cmd_set_data} "VBoxInternal/Devices/smc/0/Config/GetKeyFromRealSMC" 1 || return 1
	${cmd_set_data} "VBoxInternal2/EfiGopMode" "4" || return 1

	echo "fixup ${vm_name} successfull"
}

function cavan-virtualbox-set-resolution()
{
	[ "$2" ] ||
	{
		echo "cavan-virtualbox-set-resolution <VM> <RESOLUTION>"
		return 1
	}

	if [ "$3" ]
	then
		VBoxManage setextradata "$1" "VBoxInternal2/UgaHorizontalResolution" "$2" || return 1
		VBoxManage setextradata "$1" "VBoxInternal2/UgaVerticalResolution" "$3" || return 1
	else
		VBoxManage setextradata "$1" "VBoxInternal2/EfiGopMode" "$2" || return 1
	fi
}

function cavan-virtualbox-set-bootarg()
{
	[ "$#" -lt "2" ] &&
	{
		echo "cavan-virtualbox-set-bootarg <VM> <ARG>"
		return 1
	}

	VBoxManage setextradata "$1" "VBoxInternal2/EfiBootArgs" "$2"
}

function cavan-virtualbox-osx-set-safemode()
{
	local bootarg

	if [ "$2" = "1" ]
	then
		bootarg="-x"
	else
		bootarg=""
	fi

	cavan-virtualbox-set-bootarg "$1" "${bootarg}"
}

function cavan-virtualbox-add-resolution()
{
	[ "$2" ] ||
	{
		echo "cavan-virtualbox-add-resolution <VM> <RESOLUTION>"
		return 1
	}

	VBoxManage setextradata "$1" CustomVideoMode1 "${2}x32"
}

function cavan-virtualbox-setup()
{
	for fn in /sbin/rcvboxdrv /usr/lib/virtualbox/vboxdrv.sh /etc/init.d/vboxdrv
	do
		[ -x "${fn}" ] &&
		{
			echo "Try ${fn}"
			sudo "${fn}" "setup" && return 0
			break
		}
	done

	return 1
}

alias cavan-virtualbox-set-uuid="VBoxManage internalcommands sethduuid"
