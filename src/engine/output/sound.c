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

void sound_attach_buffer(Sound* sound) {
	alSourcei(sound->source, AL_BUFFER, sound->buffer);
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

void sound_play_once_rtwp(Sound* sound, float sound_pos[3], float listener_pos[3], float listener_dir[3], float listener_up[3]) {
    // Update listener position
    alListener3f(AL_POSITION, listener_pos[0], listener_pos[1], listener_pos[2]);

    // Correct listener orientation: forward direction (listener_dir) and up direction (assumed Y axis: 0,1,0)
    float listener_orientation[] = { 
        listener_dir[0], listener_dir[1], listener_dir[2], // forward direction
        listener_up[0], listener_up[1], listener_up[2]  // up direction (Y-axis)
    };
    alListenerfv(AL_ORIENTATION, listener_orientation); // Set listener orientation

    // Set attenuation for the source (this controls how the sound fades with distance)
    // AL_ROLLOFF_FACTOR: 1.0 means the sound will gradually fade with distance
    alSourcef(sound->source, AL_ROLLOFF_FACTOR, 1.0f);  // Attenuate sound over distance
    alSourcef(sound->source, AL_REFERENCE_DISTANCE, 5.0f);  // Sound at full volume within 5 units
    alSourcef(sound->source, AL_MAX_DISTANCE, 50.0f);      // Maximum audible distance is 50 units

    // Update sound source position (this moves the sound in 3D space)
    alSourcefv(sound->source, AL_POSITION, sound_pos);

    // Check if the sound is already playing
    ALint state;
    alGetSourcei(sound->source, AL_SOURCE_STATE, &state);

    // Play the sound only if it's not already playing
    if (state != AL_PLAYING && !sound->played) {
        sound_play_once(sound);
		sound->played = true;
    }
}

void sound_play_repeat_rtwp(Sound* sound, float sound_pos[3], float listener_pos[3], float listener_dir[3], float listener_up[3]) {
    // Update listener position
    alListener3f(AL_POSITION, listener_pos[0], listener_pos[1], listener_pos[2]);

    // Correct listener orientation: forward direction (listener_dir) and up direction (assumed Y axis: 0,1,0)
    float listener_orientation[] = { 
        listener_dir[0], listener_dir[1], listener_dir[2], // forward direction
        listener_up[0], listener_up[1], listener_up[2]  // up direction (Y-axis)
    };
    alListenerfv(AL_ORIENTATION, listener_orientation); // Set listener orientation

    // Set attenuation for the source
    alSourcef(sound->source, AL_ROLLOFF_FACTOR, 1.0f);  // Attenuate sound over distance
    alSourcef(sound->source, AL_REFERENCE_DISTANCE, 5.0f);  // Sound at full volume within 5 units
    alSourcef(sound->source, AL_MAX_DISTANCE, 50.0f);      // Maximum audible distance is 50 units

    // Set sound source position
    alSourcefv(sound->source, AL_POSITION, sound_pos);

    // Play the sound on repeat (looping)
    sound_play_repeat(sound); // This already handles looping
}
