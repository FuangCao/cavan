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
