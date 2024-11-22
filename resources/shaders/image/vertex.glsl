#version 410 core

layout(location = 0) in vec2 position;    // Vertex position (x, y)
layout(location = 1) in vec2 texCoord;    // Texture coordinates (u, v)

out vec2 fragTexCoord;                    // Texture coordinates to pass to fragment shader

uniform mat4 model;                       // Model matrix for transformations (scaling, translation)
uniform mat4 projection;                  // Projection matrix for 2D rendering

void main()
{
    // Apply the model transformation to the vertex position
    gl_Position = projection * model * vec4(position, 0.0, 1.0);

    // Pass the texture coordinates to the fragment shader
    fragTexCoord = texCoord;
}