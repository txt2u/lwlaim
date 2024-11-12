#include "buffers.h"
#include <stdlib.h>

// Creates and returns a VAO
GLuint buffers_create_vao() {
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    return VAO;
}

// Creates and returns a VBO with the specified vertex data
GLuint buffers_create_vbo(const float* vertices, size_t vertex_count) {
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(float), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return VBO;
}

// Creates and returns an EBO with the specified index data
GLuint buffers_create_ebo(const unsigned int* indices, size_t index_count) {
    GLuint EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(unsigned int), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    return EBO;
}

// Initializes an empty VAO, VBO, and optionally an EBO for your own data
Buffers buffers_create_empty() {
    Buffers buffers = {0};

    // Create VAO
    buffers.VAO = buffers_create_vao();
    buffers_bind_vao(buffers.VAO);

    // Unbind buffers to prevent accidental modification
    buffers_unbind_vao(0);

    return buffers;
}

// Binding and unbinding functions for VAO, VBO, and EBO
void buffers_bind_vao(GLuint VAO) { glBindVertexArray(VAO); }
void buffers_bind_vbo(GLuint VBO) { glBindBuffer(GL_ARRAY_BUFFER, VBO); }
void buffers_bind_ebo(GLuint EBO) { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); }

void buffers_unbind_vao() { glBindVertexArray(0); }
void buffers_unbind_vbo() { glBindBuffer(GL_ARRAY_BUFFER, 0); }
void buffers_unbind_ebo() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

// Deletes all buffers in the Buffers struct
void buffers_destroy(Buffers* buffers) {
    if (buffers->EBO) {
        glDeleteBuffers(1, &buffers->EBO);
        buffers->EBO = 0;
    }

    if (buffers->TexCoordVBO) {
        glDeleteBuffers(1, &buffers->TexCoordVBO);  // Delete the TexCoordVBO
        buffers->TexCoordVBO = 0;
    }

    if (buffers->VBO) {
        glDeleteBuffers(1, &buffers->VBO);
        buffers->VBO = 0;
    }

    if (buffers->VAO) {
        glDeleteVertexArrays(1, &buffers->VAO);
        buffers->VAO = 0;
    }
}