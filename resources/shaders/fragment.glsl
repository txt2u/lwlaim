#version 330 core

// Input from vertex shader
in vec2 fragTexCoord;

// Output color
out vec4 color;

// Texture sampler
uniform sampler2D texture1;

void main()
{
    // Sample the texture at the fragment's texture coordinates
    color = texture(texture1, fragTexCoord);
}
