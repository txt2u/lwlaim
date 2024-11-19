#ifndef CROSSHAIR_H
#define CROSSHAIR_H

#include <glad/glad.h>
#include <pipeline/shader.h>
#include <pipeline/buffers.h>
#include "cglm/cglm.h"

typedef struct {
    // Buffers for the crosshair (VBO, VAO, etc.)
    Buffers buffers;
    
    // Shader for rendering the crosshair
    ShaderProgram shader;

    // Crosshair properties
    float size;
    float thickness;
    vec4 color;
} Crosshair;

// Initialize the crosshair and shader
void crosshair_init(Crosshair *crosshair, float size, float thickness, vec4 color);

// Render the crosshair
void crosshair_render(Crosshair *crosshair, int screenWidth, int screenHeight);

// Clean up crosshair resources
void crosshair_destroy(Crosshair *crosshair);

#endif // CROSSHAIR_H
