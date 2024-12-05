#version 410 core

layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
    vec4 pos = model * projection * view * vec4(aPos, 1.0);
    
    // Force the depth value to 1.0 to ensure the skybox renders behind all objects
    gl_Position = pos.xyww; // gl_Position's z and w components are used for depth
    TexCoords = aPos;  // Pass the modified position to fragment shader for texture lookup
}