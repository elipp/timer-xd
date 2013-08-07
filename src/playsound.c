#include <stdio.h>
#include <stdlib.h>

#include <AL/al.h>
#include <AL/alc.h>

#include "playsound.h"

static ALuint source, buffer;

static ALCdevice* device = NULL;
static ALCcontext* context = NULL;

static const ALuint sampleRate = 44100;

int createALContext() {

	device = alcOpenDevice(NULL);
	context = alcCreateContext(device, NULL);
	alcMakeContextCurrent(context);

	if (!device || !context) { 
		return 0;
	}

	//ALchar* deviceString = (ALchar*)alGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);

	alListener3f(AL_POSITION, 0, 0, 0);
	alListener3f(AL_VELOCITY, 0, 0, 0);
//	alListenerfv(AL_ORIENTATION, orientation);

	alGenSources(1, &source);

	ALenum err = alGetError();
	if (err != AL_NO_ERROR) {
		printf("Creating OpenAL context failed.\n");
		return 0;
	}

	const float position[3] = { 0.0, 0.0, 0.0 };
//	const float velocity[3] = { 0.0, 0.0, 0.0 };
//	const float orientation[6] = { 0.0, 0.0, -1.0, 0.0, 1.0, 0.0 };

	alSourcef(source, AL_PITCH, 1.0);
	alSourcef(source, AL_GAIN, 1.0);
	alSource3f(source, AL_POSITION, position[0], position[1], position[2]);
	alSource3f(source, AL_VELOCITY, 0, 0, 0);
	alSourcei(source, AL_LOOPING, AL_FALSE); 	// no looping

	err = alGetError();

	if (err != AL_NO_ERROR) {

		printf("alGetError: error (%x)", (int)err);
		return 0;
				
	}
	
	return 1; 

}


static int createBufferFromData_int16(void *data, size_t samplecount, ALuint format) {

	alGenBuffers(1, &buffer);
	alBufferData(buffer, format, data, samplecount*sizeof(short), sampleRate);
	alSourcei(source, AL_BUFFER, buffer);

	ALenum err = alGetError();
	if (err != AL_NO_ERROR) {
		alSourcei(source, AL_BUFFER, 0);
		alDeleteBuffers(1, &buffer);
		printf("buffer creation failed.\n");
		return 0;
	}

	return 1;

}

int load_raw(char* buf_beg, size_t filesize) {
	// assuming mono 44100, we're using a builtin file (alarmsound.o)
	const size_t sample_count = (filesize - 44)/sizeof(short); // WAV header = 44 bytes
	return createBufferFromData_int16(buf_beg+44, sample_count, AL_FORMAT_MONO16);
}

void al_cleanup() {

	alDeleteSources(1, &source);
	alDeleteBuffers(1, &buffer);
	alcDestroyContext(context);
	alcCloseDevice(device);

}


void playSound() {
	// assuming valid context
	alSourcePlay(source);
}

void printDeviceString() {

	const ALchar* devString = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);

	printf("Audio device string: %s\n", devString);

}

int is_still_playing() {
	ALint state;
	alGetSourcei(source, AL_SOURCE_STATE, &state);
	
	return state == AL_PLAYING ? 1 : 0;
}

