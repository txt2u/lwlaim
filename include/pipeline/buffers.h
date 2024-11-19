#ifndef BUFFERS_H
#define BUFFERS_H

#include <glad/glad.h>

typedef struct {
    GLuint VAO;
    GLuint VBO;
    GLuint TexCoordVBO;  // Added for texture coordinates
    GLuint NormalVBO;  // Added for texture coordinates
    GLuint EBO;
} Buffers;

// Initializes a VAO
GLuint buffers_create_vao();

// Initializes a VBO with vertex data
GLuint buffers_create_vbo(const float* vertices, size_t vertex_count);

// Initializes an EBO with index data
GLuint buffers_create_ebo(const unsigned int* indices, size_t index_count);

// Initializes a VAO, VBO, and optionally an EBO for existing data
Buffers buffers_create_empty();

// Binds a specific buffer
void buffers_bind_vao(GLuint VAO);
void buffers_bind_vbo(GLuint VBO);
void buffers_bind_ebo(GLuint EBO);

// Unbinds the VAO, VBO, or EBO
void buffers_unbind_vao();
void buffers_unbind_vbo();
void buffers_unbind_ebo();

// Deletes the buffers to free resources
void buffers_destroy(Buffers* buffers);

#endif // BUFFERS_H