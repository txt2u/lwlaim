#ifndef SOUND_H
#define SOUND_H

#include <stdbool.h>
#include <AL/al.h>
#include <AL/alc.h>

// Sound structure
typedef struct {
    ALuint buffer;       // Audio buffer
    ALuint source;       // Audio source
    bool is_playing;     // Playback state
    bool is_muted;       // Mute state
    float volume;        // Current volume
} Sound;

// Sound system initialization and cleanup
bool sound_initialize();
void sound_cleanup();
void sound_attach_buffer(Sound* sound);

// Play sound functions
void sound_play_once(Sound* sound);
void sound_play_repeat(Sound* sound);

// Mute and volume controls
void sound_mute(Sound* sound, bool mute);
void sound_set_volume(Sound* sound, float volume);

// Pause/resume controls
void sound_pause(Sound* sound);
void sound_resume(Sound* sound);

// Spatial audio: relative to world position (rtwp)
void sound_play_once_rtwp(Sound* sound, float sound_pos[3], float listener_pos[3], float listener_dir[3]);
void sound_play_repeat_rtwp(Sound* sound, float sound_pos[3], float listener_pos[3], float listener_dir[3]);

#endif // SOUND_H