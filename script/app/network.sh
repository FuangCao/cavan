#!/bin/bash

function cavan-net-ipconfig()
{
	local dev address gateway mask dns value

	{
		echo "Select network device:"

		select dev in $(ls /sys/class/net)
		do
			[ "${dev}" ] && break
		done

		echo -n "Address: "
		read address

		gateway="${address%.*}.1"
		echo -n "Gateway (${gateway}): "
		read value
		[ "${value}" ] && gateway=${value}

		mask="255.255.255.0"
		echo -n "Mask (${mask}): "
		read value
		[ "${value}" ] && mask=${value}

		dns=${gateway}
		echo -n "Dns (${dns}): "
		read value
		[ "${value}" ] && dns=${value}

		echo
		echo "Dev: ${dev}"
		echo "Address: ${address}"
		echo "Gateway: ${gateway}"
		echo "Mask: ${mask}"
		echo "Dns: ${dns}"
	} >&2

	if [ "$(which netplan)" ]
	then
		echo "network:"
		echo "    version: 2"
		echo "    ethernets:"
		echo "        ${dev}:"
		echo "            dhcp4: no"
		echo "            addresses: [${address}/24]"
		echo "            gateway4: ${gateway}"
		echo "            nameservers:"
		echo "                addresses: [${dns}, 8.8.8.8]"
	else
		echo "auto ${dev}"
		echo "iface ${dev} inet static"
		echo "address ${address}"
		echo "gateway ${gateway}"
		echo "netmask ${mask}"
		echo "dns-nameservers ${dns}"
	fi
}
