#!/bin/bash

function cavan-ffmpeg-u16-8000()
{
	local target_fn

	for fn in $@
	do
		target_fn="${fn}-8000.u16"
		rm -rf $target_fn* || return 1
		ffmpeg -i $fn -ac 1 -ar 8000 -f u16le $target_fn || return 1
		ffmpeg -ac 1 -ar 8000 -f u16le -i $target_fn ${target_fn}.wav || return 1
	done
}

function cavan-ffmpeg-u16-16000()
{
	local target_fn

	for fn in $@
	do
		target_fn="${fn}-16000.u16"
		rm -rf $target_fn* || return 1
		ffmpeg -i $fn -ac 1 -ar 16000 -f u16le $target_fn || return 1
		ffmpeg -ac 1 -ar 16000 -f u16le -i $target_fn ${target_fn}.wav || return 1
	done
}

function cavan-ffmpeg-u16-22050()
{
	local target_fn

	for fn in $@
	do
		target_fn="${fn}-22050.u16"
		rm -rf $target_fn* || return 1
		ffmpeg -i $fn -ac 1 -ar 22050 -f u16le $target_fn || return 1
		ffmpeg -ac 1 -ar 22050 -f u16le -i $target_fn ${target_fn}.wav || return 1
	done
}

function cavan-ffmpeg-u16-44100()
{
	local target_fn

	for fn in $@
	do
		target_fn="${fn}-44100.u16"
		rm -rf $target_fn* || return 1
		ffmpeg -i $fn -ac 1 -ar 44100 -f u16le $target_fn || return 1
		ffmpeg -ac 1 -ar 44100 -f u16le -i $target_fn ${target_fn}.wav || return 1
	done
}
