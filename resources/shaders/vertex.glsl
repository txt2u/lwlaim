#version 330 core

layout(location = 0) in vec3 position;      // Vertex position
layout(location = 1) in vec3 normal;        // Normals
layout(location = 2) in vec2 texCoord;      // Texture coordinates

out vec2 fragTexCoord;                      // Output texture coordinate to fragment shader
out vec3 fragNormal;                        // Output normal to fragment shader

uniform mat4 model;                         // Model matrix
uniform mat4 view;                          // View matrix
uniform mat4 projection;                    // Projection matrix

void main() {
    fragTexCoord = texCoord;                // Pass texCoord to fragment shader
    fragNormal = normal;                    // Pass normal to fragment shader

    // Apply MVP matrix transformation
    gl_Position = projection * view * model * vec4(position, 1.0); 
}