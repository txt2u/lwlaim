#version 440 core

in vec2 fragTexCoord;                      // Input texture coordinates from vertex shader
out vec4 fragColor;                        // Output color

uniform sampler2D texture1;                // Texture

void main() {
    // Sample the texture using the texture coordinates
    fragColor = texture(texture1, fragTexCoord);  // Texture sampling using fragTexCoord
}