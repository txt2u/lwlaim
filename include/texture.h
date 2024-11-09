#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/glew.h>

// Struct to hold texture information
typedef struct {
    GLuint id;
    int width;
    int height;
} Texture;

// Initialize a texture with raw data (e.g., font bitmap or image pixel data)
Texture texture_create(const unsigned char* data, int width, int height, GLenum format);

// Bind a texture to a specified texture unit
void texture_bind(const Texture* texture, GLenum textureUnit);

// Free the OpenGL texture resources
void texture_destroy(Texture* texture);

#endif // TEXTURE_H
