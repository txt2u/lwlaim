#version 330 core

in vec3 fragNormal;  // Normal passed from vertex shader
in vec2 fragTexCoord;  // Texture coordinates passed from vertex shader
out vec4 fragColor;  // Final output color

uniform sampler2D texture1;

void main() {
    fragColor = texture(texture1, fragTexCoord);  // Simply output the color passed from the vertex shader
	// fragColor = vec4(fragTexCoord, 0.0, 1.0);  // Show UVs as color
}
