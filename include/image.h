#ifndef IMAGE_H
#define IMAGE_H

#include <GL/glew.h>
#include <stdio.h>
#include <stdbool.h>

#include "buffers.h"

typedef struct {
    GLuint texture_id;
    GLuint shader_program;
    Buffers buffers;
    int width, height;
	float rotation;
	bool model_dirty;
} Image;

// Function prototypes
void image_init(Image *image, const char *image_path, GLuint shader_program);
void image_set_dimensions(Image *image, int new_width, int new_height);
void image_set_dimensions_by_shader(Image *image, float new_width, float new_height);
void image_set_rotation_by_shader_dirty(Image *image, float angle_degrees);
void image_set_rotation_by_shader(Image *image, float angle_degrees);
void image_render(Image *image, float x, float y);
void image_cleanup(Image *image);

#endif // IMAGE_H
