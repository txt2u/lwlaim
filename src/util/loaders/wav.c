#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <AL/al.h>
#include <wav.h>
#include <string.h>

bool load_wav(const char* filename, ALenum* format, ALvoid** data, ALsizei* size, ALsizei* freq) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Failed to open WAV file: %s\n", filename);
        return false;
    }

    // Read WAV header
    char chunk_id[4];
    uint32_t chunk_size;
    char format_type[4];

    fread(chunk_id, 1, 4, file);
    fread(&chunk_size, 4, 1, file);
    fread(format_type, 1, 4, file);

    if (strncmp(chunk_id, "RIFF", 4) != 0 || strncmp(format_type, "WAVE", 4) != 0) {
        fprintf(stderr, "Invalid WAV file format\n");
        fclose(file);
        return false;
    }

    // Read "fmt " subchunk
    char subchunk1_id[4];
    uint32_t subchunk1_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;

    fread(subchunk1_id, 1, 4, file);
    fread(&subchunk1_size, 4, 1, file);
    fread(&audio_format, 2, 1, file);
    fread(&num_channels, 2, 1, file);
    fread(&sample_rate, 4, 1, file);
    fread(&byte_rate, 4, 1, file);
    fread(&block_align, 2, 1, file);
    fread(&bits_per_sample, 2, 1, file);

    if (strncmp(subchunk1_id, "fmt ", 4) != 0) {
        fprintf(stderr, "Invalid fmt chunk in WAV file\n");
        fclose(file);
        return false;
    }

    // Check for PCM format
    if (audio_format != 1) {
        fprintf(stderr, "Non-PCM WAV files are not supported\n");
        fclose(file);
        return false;
    }

    // Determine OpenAL format
    if (num_channels == 1) {
        *format = (bits_per_sample == 8) ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;
    } else if (num_channels == 2) {
        *format = (bits_per_sample == 8) ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;
    } else {
        fprintf(stderr, "Unsupported channel count: %d\n", num_channels);
        fclose(file);
        return false;
    }

    *freq = sample_rate;

    // Read "data" subchunk
    char subchunk2_id[4];
    uint32_t subchunk2_size;

    while (fread(subchunk2_id, 1, 4, file)) {
        fread(&subchunk2_size, 4, 1, file);
        if (strncmp(subchunk2_id, "data", 4) == 0) {
            break;
        }
        fseek(file, subchunk2_size, SEEK_CUR); // Skip non-"data" chunks
    }

    if (strncmp(subchunk2_id, "data", 4) != 0) {
        fprintf(stderr, "No data chunk found in WAV file\n");
        fclose(file);
        return false;
    }

    // Allocate memory for audio data and read it
    *size = subchunk2_size;
    *data = malloc(subchunk2_size);
    if (!*data) {
        fprintf(stderr, "Failed to allocate memory for WAV data\n");
        fclose(file);
        return false;
    }

    fread(*data, 1, subchunk2_size, file);
    fclose(file);

    return true;
}