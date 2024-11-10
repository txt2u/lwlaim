#version 330 core

// Vertex attributes
layout(location = 0) in vec3 position;  // Vertex position
layout(location = 1) in vec2 texCoord;  // Texture coordinates

// Uniforms
uniform mat4 model;        // Model transformation matrix
uniform mat4 view;         // View transformation matrix
uniform mat4 projection;   // Projection matrix

// Outputs to fragment shader
out vec2 fragTexCoord; // Texture coordinates

void main()
{
    // Apply transformations: Model -> View -> Projection
    gl_Position = projection * view * model * vec4(position, 1.0);

    // Pass texture coordinates to the fragment shader
    fragTexCoord = texCoord;
}
