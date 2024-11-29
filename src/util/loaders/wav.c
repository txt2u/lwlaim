#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <AL/al.h>
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

    // Set OpenAL format to mono, regardless of the original number of channels
    *format = (bits_per_sample == 8) ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;

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

    // If the file was stereo, downmix it to mono
    if (num_channels == 2) {
        int16_t* stereo_data = (int16_t*)*data;
        int16_t* mono_data = (int16_t*)malloc(*size / 2); // Mono data will be half the size of stereo data

        if (!mono_data) {
            fprintf(stderr, "Failed to allocate memory for downmixed data\n");
            free(*data);
            return false;
        }

        size_t num_samples = *size / sizeof(int16_t);
        for (size_t i = 0, j = 0; i < num_samples; i += 2, ++j) {
            // Average left and right channels to create mono data
            mono_data[j] = (stereo_data[i] + stereo_data[i + 1]) / 2;
        }

        // Free the original stereo data and point to the mono data
        free(*data);
        *data = mono_data;
        *size = *size / 2;  // Update size for mono data
    }

    return true;
}