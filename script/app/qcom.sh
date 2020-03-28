#!/bin/bash

alias cavan-lunch-CP10="cavan-android-choosecombo 1 CP10 userdebug 32"
alias cavan-build-CP10="cavan-lunch-CP10 && cavan-qcom-build-android"

alias cavan-lunch-APH7="cavan-android-choosecombo 1 APH7 userdebug 32"
alias cavan-build-APH7="cavan-lunch-APH7 && cavan-qcom-build-android"

alias cavan-qcom-build-bootloader="cavan-android-make aboot"
alias cavan-qcom-build-kernel="cavan-android-make bootimage"
alias cavan-qcom-build-system="cavan-android-make systemimage"

function cavan-qcom-mkdir()
{
	[ -d "$1" ] && return 0
	[ -e "$1" ] && rm -f "$1" || return 1
	mkdir -pv "$1" || return 1
}

function cavan-qcom-build-android()
{
	local target_system="out/target/product/msm8953_64/system"

	cavan-android-croot || return 1

	cavan-qcom-mkdir "${target_system}/lib" || return 1
	cavan-qcom-mkdir "${target_system}/system/lib64" || return 1
	cavan-android-make || return 1
}

function cavan-qcom-build-modem-msm8953()
{
	cavan-android-croot && cd modem || return 1
	rm -rf ADSP.8953.2.8.2/adsp_proc/obj || return 1
	source factory/build/envsetup_sm55c72.sh || return 1
	./build_all_8953.sh || return 1
	./gen_firehose_8953.sh || return 1
	./gen_symbols_8953.sh || return 1
}
