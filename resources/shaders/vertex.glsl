#version 410 core

layout(location = 0) in vec3 position;      // Vertex position
layout(location = 1) in vec3 normal;        // Normals
layout(location = 2) in vec2 texCoord;      // Texture coordinates

out vec2 fragTexCoord;                      // Output texture coordinate to fragment shader
out vec3 fragNormal;                        // Output normal to fragment shader
out vec3 fragPosition;                      // Output world position to fragment shader

uniform mat4 model;                         // Model matrix
uniform mat4 view;                          // View mawtrix
uniform mat4 projection;                    // Projection matrix

void main() {
    fragTexCoord = texCoord;                // Pass texCoord to fragment shader
    fragNormal = mat3(transpose(inverse(model))) * normal; // Transform normal to world space
    fragPosition = vec3(model * vec4(position, 1.0)); // World space position

    // Apply MVP matrix transformation
    gl_Position = projection * view * model * vec4(position, 1.0); 
}