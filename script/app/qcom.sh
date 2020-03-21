#!/bin/bash

alias cavan-qcom-lunch-APH7="cavan-android-choosecombo 1 APH7 userdebug 32"
alias cavan-qcom-build-android-APH7="cavan-qcom-lunch-APH7 && cavan-qcom-build-android"
alias cavan-qcom-build-bootloader="make aboot -j8"
alias cavan-qcom-build-kernel="make bootimage -j8"
alias cavan-qcom-build-system="make systemimage -j8"

function cavan-qcom-build-android()
{
	cavan-android-croot || return 1
	mkdir -pv out/target/product/msm8953_64/system/lib || return 1
	mkdir -pv out/target/product/msm8953_64/system/lib64 || return 1
	make -j8 || return 1
}

function cavan-qcom-build-modem-msm8953()
{
	cavan-android-croot && cd modem || return 1
	source factory/build/envsetup_sm55c72.sh || return 1
	./build_all_8953.sh || return 1
	./gen_firehose_8953.sh || return 1
	./gen_symbols_8953.sh || return 1
}
