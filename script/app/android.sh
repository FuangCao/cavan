#!/bin/bash

function cavan-android-croot()
{
	[ "${ANDROID_BUILD_TOP}" ] &&
	{
		cd "${ANDROID_BUILD_TOP}" && return 0
	}
}

function cavan-android-lunch()
{
	source build/envsetup.sh
	lunch $1
}

alias cavan-lunch-rk3288="cavan-android-lunch rk3288-userdebug"
alias cavan-lunch-ms600="cavan-android-lunch imx6ms600-user"
