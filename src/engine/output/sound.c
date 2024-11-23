#include <AL/al.h>
#include <AL/alc.h>
#include <output/sound.h>
#include <stdlib.h>
#include <stdio.h>

static ALCdevice* device = NULL;
static ALCcontext* context = NULL;

bool sound_initialize() {
    // Retrieve the default device specifier
    const ALchar* deviceName = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
    if (!deviceName) {
        fprintf(stderr, "Failed to retrieve default device specifier.\n");
        return false;
    }

    // Open the audio device
    device = alcOpenDevice(deviceName);
    if (!device) {
        fprintf(stderr, "Failed to open audio device.\n");
        return false;
    }

    // Create the audio context
    context = alcCreateContext(device, NULL);
    if (!context || !alcMakeContextCurrent(context)) {
        fprintf(stderr, "Failed to create or set audio context.\n");
        alcCloseDevice(device);
        return false;
    }

    fprintf(stdout, "OpenAL initialized successfully. Device: %s\n", deviceName);
    return true;
}

void sound_cleanup() {
    alcMakeContextCurrent(NULL); // Detach current context
    if (context) alcDestroyContext(context);
    if (device) alcCloseDevice(device);
}

void sound_play_once(Sound* sound) {
    if (sound->is_muted) return;
    alSourcei(sound->source, AL_LOOPING, AL_FALSE); // Disable looping
    alSourcePlay(sound->source);
    sound->is_playing = true;
}

void sound_play_repeat(Sound* sound) {
    if (sound->is_muted) return;
    alSourcei(sound->source, AL_LOOPING, AL_TRUE); // Enable looping
    alSourcePlay(sound->source);
    sound->is_playing = true;
}

void sound_mute(Sound* sound, bool mute) {
    sound->is_muted = mute;
    alSourcef(sound->source, AL_GAIN, mute ? 0.0f : sound->volume);
}

void sound_set_volume(Sound* sound, float volume) {
    sound->volume = volume;
    if (!sound->is_muted) {
        alSourcef(sound->source, AL_GAIN, volume);
    }
}

void sound_pause(Sound* sound) {
    if (sound->is_playing) {
        alSourcePause(sound->source);
        sound->is_playing = false;
    }
}

void sound_resume(Sound* sound) {
    if (!sound->is_playing) {
        alSourcePlay(sound->source);
        sound->is_playing = true;
    }
}

void sound_play_once_rtwp(Sound* sound, float sound_pos[3], float listener_pos[3], float listener_dir[3]) {
    alListener3f(AL_POSITION, listener_pos[0], listener_pos[1], listener_pos[2]);  // Set listener position
    alListener3f(AL_ORIENTATION, listener_dir[0], listener_dir[1], listener_dir[2]); // Set listener direction

    alSource3f(sound->source, AL_POSITION, sound_pos[0], sound_pos[1], sound_pos[2]); // Set sound source position
    sound_play_once(sound); // Play the sound once
}

void sound_play_repeat_rtwp(Sound* sound, float sound_pos[3], float listener_pos[3], float listener_dir[3]) {
    alListener3f(AL_POSITION, listener_pos[0], listener_pos[1], listener_pos[2]);  // Set listener position
    alListener3f(AL_ORIENTATION, listener_dir[0], listener_dir[1], listener_dir[2]); // Set listener direction

    alSource3f(sound->source, AL_POSITION, sound_pos[0], sound_pos[1], sound_pos[2]); // Set sound source position
    sound_play_repeat(sound); // Play the sound on repeat
}