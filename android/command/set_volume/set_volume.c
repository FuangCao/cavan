/*
 * File:		set_volume.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2014-11-25 11:55:09
 *
 * Copyright (c) 2014 Fuang.Cao <cavan.cfa@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#define pr_std_info(fmt, args ...) \
	printf("%s[%d]: " fmt "\n", __FUNCTION__, __LINE__, ##args)

#define pr_red_info(fmt, args ...) \
	pr_std_info("\033[31m" fmt "\033[0m", ##args)

#define pr_green_info(fmt, args ...) \
	pr_std_info("\033[32m" fmt "\033[0m", ##args)

#define pr_bold_info(fmt, args ...) \
	pr_std_info("\033[1m" fmt "\033[0m", ##args)

#ifndef NELEM
#define NELEM(a) \
	((int)(sizeof(a) / sizeof((a)[0])))
#endif

int setStreamVolume(int stream, float volume);
int getStreamVolume(int stream, float *volume);

static const char *stream_name_list[] =
{
	"voice_call",
	"system",
	"ring",
	"music",
	"alarm",
	"notification",
	"bluetooth_sco",
	"fm",
	"dtmf",
	"tts",
	"enforced_audible",
	"num_stream_types"
};

static int getStreamByName(const char *name)
{
	int i;

	for (i = 0; i < NELEM(stream_name_list); i++)
	{
		if (strcmp(stream_name_list[i], name) == 0)
		{
			return i;
		}
	}

	return -1;
}

int main(int argc, char *argv[])
{
	int ret;
	int stream;
	float volume;

	assert(argc > 1);

	stream = getStreamByName(argv[1]);
	if (stream < 0)
	{
		pr_red_info("getStreamByName");
		return stream;
	}

	if (argc > 2)
	{
		volume = atoi(argv[2]);

		ret = setStreamVolume(stream, volume / 100);
		if (ret < 0)
		{
			pr_red_info("setStreamVolume");
		}
		else
		{
			pr_green_info("setStreamVolume");
		}
	}
	else
	{
		ret = getStreamVolume(stream, &volume);
		if (ret < 0)
		{
			pr_red_info("getStreamVolume");
		}
		else
		{
			pr_green_info("volume = %f", volume);
		}
	}

	return 0;
}
