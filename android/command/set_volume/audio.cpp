/*
 * File:		audio.cpp
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

#include <media/AudioSystem.h>

extern "C" {

int setStreamVolume(int stream, float volume)
{
	return android::AudioSystem::setStreamVolume(stream, volume, 0);
}

int getStreamVolume(int stream, float *volume)
{
	return android::AudioSystem::getStreamVolume(stream, volume, 0);
}

}
