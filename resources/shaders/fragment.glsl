#version 330 core

in vec2 fragTexCoord;                      // Input texture coordinates from vertex shader
out vec4 fragColor;                        // Output color

uniform sampler2D texture1;                // Texture

void main() {
    fragColor = texture(texture1, fragTexCoord); // Sample the texture
}