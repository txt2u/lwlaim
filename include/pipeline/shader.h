#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

// Struct to hold shader program information
typedef struct {
    GLuint id;
} ShaderProgram;

// Create a shader program from vertex and fragment shader source strings
ShaderProgram shader_create(const char* vertexSrc, const char* fragmentSrc);

// Use the shader program
void shader_use(const ShaderProgram* shader);

// Free the shader program resources
void shader_destroy(ShaderProgram* shader);

#endif // SHADER_H