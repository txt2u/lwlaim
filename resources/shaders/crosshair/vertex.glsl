#version 410 core
layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 model;
uniform float size;

void main() {
    vec3 scaledPos = aPos * size; // Scale the vertex position by the size
    gl_Position = projection * model * vec4(scaledPos, 1.0);
}