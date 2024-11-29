#include <pipeline/shader.h>
#include <stdio.h>
#include <stdlib.h>

// Helper function to compile a shader and check for errors
static GLuint compile_shader(const char* source, GLenum shaderType) {
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    // Check for compilation errors
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, sizeof(infoLog), NULL, infoLog);
        fprintf(stderr, "Shader compilation failed: %s\n", infoLog);
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

// Create a shader program from vertex and fragment shader sources
ShaderProgram shader_create(const char* vertexSrc, const char* fragmentSrc) {
    ShaderProgram shaderProgram = {0};

    GLuint vertexShader = compile_shader(vertexSrc, GL_VERTEX_SHADER);
    if (vertexShader == 0) return shaderProgram;

    GLuint fragmentShader = compile_shader(fragmentSrc, GL_FRAGMENT_SHADER);
    if (fragmentShader == 0) {
        glDeleteShader(vertexShader);
        return shaderProgram;
    }

    // Link shaders into a program
    shaderProgram.id = glCreateProgram();
    glAttachShader(shaderProgram.id, vertexShader);
    glAttachShader(shaderProgram.id, fragmentShader);
    glLinkProgram(shaderProgram.id);

    // Check for linking errors
    GLint success;
    glGetProgramiv(shaderProgram.id, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram.id, sizeof(infoLog), NULL, infoLog);
        fprintf(stderr, "Shader program linking failed: %s\n", infoLog);
        glDeleteProgram(shaderProgram.id);
        shaderProgram.id = 0;
    }

    // Clean up shaders as they're no longer needed after linking
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

// Use the shader program
void shader_use(const ShaderProgram* shader) {
	if (shader->id == 0) {
		fprintf(stderr, "[fn shader_use] Trying to use a shader program with the id of 0.\n");
		return;
	}

    if (shader && shader->id) {
		// Todo: vvvvvvvvvvv -> Enable later
		// shader_disband();
		
        glUseProgram(shader->id);
    }
}

void shader_disband() {
	glUseProgram(0);
}

// Free the shader program resources
void shader_destroy(ShaderProgram* shader) {
    if (shader && shader->id) {
        glDeleteProgram(shader->id);
        shader->id = 0;
    }
}