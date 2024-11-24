#ifndef MATERIAL_H
#define MATERIAL_H

#include <glad/glad.h>

typedef struct {
    GLuint texture_id;
    float base_color[4];  // RGBA color
    float metallic;
    float roughness;
} Material;

// Function to create a material with texture
Material *material_create_with_texture(const char *texture_path, const float base_color[4]);

// Function to create a material with a color (no texture)
Material *material_create_with_color(const float base_color[4]);

// Function to load a texture from file
GLuint load_texture(const char *texture_path);

// Function to free a material
void material_free(Material *material);

// Function to apply material properties to a shader
void material_apply_to_shader(Material *material, GLuint shader_program);

#endif // MATERIAL_H