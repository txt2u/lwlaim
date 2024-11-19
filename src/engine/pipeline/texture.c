#include <pipeline/texture.h>
#include <stdio.h>

// Create a texture with raw pixel data (e.g., grayscale or RGB data)
Texture texture_create(const unsigned char* data, int width, int height, GLenum format) {
    Texture texture = {0};
    texture.width = width;
    texture.height = height;

    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload pixel data to the GPU
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

    return texture;
}

// Bind the texture to a specified texture unit (e.g., GL_TEXTURE0)
void texture_bind(const Texture* texture, GLenum textureUnit) {
    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_2D, texture->id);
}

// Destroy the texture and free resources
void texture_destroy(Texture* texture) {
    if (texture->id) {
        glDeleteTextures(1, &texture->id);
        texture->id = 0;
        texture->width = 0;
        texture->height = 0;
    }
}
