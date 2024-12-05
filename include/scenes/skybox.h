#include <pipeline/shader.h>
#include <pipeline/buffers.h>

#include <cglm/cglm.h>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef SKYBOX_H
#define SKYBOX_H

typedef struct {
	Buffers buffers;

	size_t program_id;
	size_t texture_id;
	
	const char* faces[6]; // Array of 6 faces for the skybox
} Skybox;

void skybox_init(Skybox* skybox, const char* source[6], size_t program_id);
void skybox_use(Skybox* skybox, mat4 projection, mat4 view, mat4 model);
void skybox_destroy(Skybox* skybox);

#endif // SKYBOX_H