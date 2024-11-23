#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <AL/al.h>

bool load_wav(const char* filename, ALenum* format, ALvoid** data, ALsizei* size, ALsizei* freq);