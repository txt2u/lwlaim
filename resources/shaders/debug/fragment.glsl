#version 410 core

out vec4 fragColor;
uniform vec4 cubeColor;

void main() {
    fragColor = cubeColor;
}