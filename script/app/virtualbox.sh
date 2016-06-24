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

	VBoxManage modifyvm "${vm_name}" --cpuidset 00000001 000306a9 04100800 7fbae3ff bfebfbff || return 1
	${cmd_set_data} "VBoxInternal/Devices/efi/0/Config/DmiSystemProduct" "MacBookPro11,3" || return 1
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

	VBoxManage setextradata "$1" "VBoxInternal2/EfiGopMode" "$2"
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
