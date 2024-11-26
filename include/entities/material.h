#ifndef MATERIAL_H
#define MATERIAL_H

#include <glad/glad.h>
#include <cglm/cglm.h>
#include <cgltf.h>

typedef struct {
    GLuint diffuse_texture_id;      // Diffuse (base color) texture
    GLuint normal_texture_id;       // Normal map
    GLuint metallic_roughness_texture_id; // Metallic-roughness texture
    GLuint occlusion_texture_id;    // Occlusion map
    GLuint emissive_texture_id;     // Emissive texture

    char* diffuse_texture;          // Path to diffuse texture
    char* normal_texture;           // Path to normal texture
    char* metallic_roughness_texture; // Path to metallic-roughness texture
    char* occlusion_texture;        // Path to occlusion texture
    char* emissive_texture;         // Path to emissive texture

    float diffuse_color[4];         // Diffuse color (RGBA)
    float emissive_color[3];        // Emissive color (RGB)
    float metallic;                 // Metallic factor
    float roughness;                // Roughness factor
} Material;

typedef enum {
    MATERIAL_BASE,
    MATERIAL_NORMAL,
    MATERIAL_METALLIC,
    MATERIAL_EMISSIVE,   // New type for emissive texture
    MATERIAL_OCCLUSION   // New type for occlusion texture
} MaterialTextureType;

GLuint load_texture_from_gltf(cgltf_texture *gltf_texture, const char* model_path);

int material_create_gl_texture(cgltf_texture* texture, MaterialTextureType type, GLuint* texture_id_dest, const char* model_path);

// int material_create(Material* material, cgltf_texture* base_texture, cgltf_texture* normal_texture, cgltf_texture* metallic_texture, cgltf_texture* emissive_texture, cgltf_texture* occlusion_texture);

// Function to apply the material (binds its textures) to a shader program
void material_apply(const Material* material, GLuint shader_program_id);

// Function to free material resources, including GPU textures
void material_free(Material* material);

#endif // MATERIAL_H
