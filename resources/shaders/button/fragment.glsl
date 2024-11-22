#version 330 core

in vec2 TexCoord;        // Texture coordinates from vertex shader

out vec4 FragColor;      // Final output color

uniform sampler2D texture_sampler; // Texture of the button background (if any)
uniform bool use_image;           // Flag to determine if we use an image or color background
uniform vec4 fragColor;

void main()
{
    if (use_image)
    {
        // When using an image as the background, sample the texture
        vec4 texColor = texture(texture_sampler, TexCoord);
        FragColor = texColor;
    }
    else
    {
        // When using a solid color as the background, use fragColor
        FragColor = vec4(fragColor);  // Set the background color
    }
}