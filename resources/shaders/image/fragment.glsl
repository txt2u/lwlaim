#version 330 core

in vec2 fragTexCoord;            // Texture coordinates from vertex shader
out vec4 FragColor;               // Output fragment color

uniform sampler2D texture_sampler; // Texture sampler

void main()
{
    vec4 texColor = texture(texture_sampler, fragTexCoord); // Sample color from texture

    // Discard the fragment if alpha is less than 0.1
    if (texColor.a < 0.1)
        discard;

    FragColor = texColor;
}