#include "crosshair.h"

#include <GL/glew.h>
#include "qreader.h"
#include "shader.h"

// Vertex data for a plus-shaped crosshair
static const float crosshairVertices[] = {
    -0.5f, 0.0f, 0.0f,
     0.5f, 0.0f, 0.0f,
     0.0f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
};

// Initialize the crosshair and shader
void crosshair_init(Crosshair *crosshair, float size, float thickness, vec3 color) {
    glm_vec3_copy(color, crosshair->color);
    crosshair->size = size;
	crosshair->thickness = thickness;

    crosshair->buffers = buffers_create_empty();
    buffers_bind_vao(crosshair->buffers.VAO);

    crosshair->buffers.VBO = buffers_create_vbo(crosshairVertices, sizeof(crosshairVertices));
    buffers_bind_vbo(crosshair->buffers.VBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    buffers_unbind_vbo();
    buffers_unbind_vao();

    // Initialize the shader once during crosshair setup
    char* vertexShaderSource = read_file("resources/shaders/crosshair/vertex.glsl");
    char* fragmentShaderSource = read_file("resources/shaders/crosshair/fragment.glsl");
    if (!vertexShaderSource || !fragmentShaderSource) {
        fprintf(stderr, "Failed to load crosshair shader sources!\n");
        return;
    }
    crosshair->shader = shader_create(vertexShaderSource, fragmentShaderSource);

    // Free shader source memory after creating the shader
    free(vertexShaderSource);
    free(fragmentShaderSource);
}

void crosshair_render(Crosshair *crosshair, int screenWidth, int screenHeight) {
    if (!crosshair->shader.id) return;  // Ensure shader is initialized

    shader_use(&crosshair->shader);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    mat4 projection;
    glm_ortho(0.0f, (float)screenWidth, (float)screenHeight, 0.0f, -1.0f, 1.0f, projection);

    glUniformMatrix4fv(glGetUniformLocation(crosshair->shader.id, "projection"), 1, GL_FALSE, (const GLfloat*)projection);
    glUniform3fv(glGetUniformLocation(crosshair->shader.id, "color"), 1, crosshair->color);
    glUniform1f(glGetUniformLocation(crosshair->shader.id, "size"), crosshair->size);

    mat4 model = GLM_MAT4_IDENTITY_INIT;
	glm_translate(model, (vec3){screenWidth / 2.0f, screenHeight / 2.0f, 0.0f});
    glUniformMatrix4fv(glGetUniformLocation(crosshair->shader.id, "model"), 1, GL_FALSE, (const GLfloat*)model);

    buffers_bind_vao(crosshair->buffers.VAO);
	glLineWidth(crosshair->thickness);
    glDrawArrays(GL_LINES, 0, 4);
	glLineWidth(0.25f);
    buffers_unbind_vao();

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

// Clean up crosshair resources, including the shader
void crosshair_destroy(Crosshair *crosshair) {
    buffers_destroy(&crosshair->buffers);
    shader_destroy(&crosshair->shader); // Properly release the shader here
}