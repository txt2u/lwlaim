#version 330 core

layout (location = 0) in vec2 aPos;            // Vertex position
layout (location = 1) in vec2 aTexCoord;       // Texture coordinates

out vec2 TexCoord;                             // Texture coordinates to pass to fragment shader

uniform mat4 model;                            // Model matrix for transformation
uniform mat4 projection;                       // Projection matrix

void main() {
    gl_Position = projection * model * vec4(aPos, 0.0, 1.0);  // Apply transformations
    TexCoord = aTexCoord;                      // Pass texture coordinates to fragment shader
}
