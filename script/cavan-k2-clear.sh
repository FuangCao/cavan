#!/bin/sh

for svc in adpush capp collect lc schemeupgrade tr069 update_up
do
	config_file="/etc/init.d/${svc}"

	[ -f "${config_file}" ] ||
	{
		echo "${config_file} skip"
		continue
	}

	${config_file} disable && echo "${config_file} disable"
	${config_file} stop && echo "${config_file} stop"
done
