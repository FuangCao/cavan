#!/bin/bash

function umount_directory()
{
	while umount $1
	do
		echo "umount $1 successfull"
	done
}
