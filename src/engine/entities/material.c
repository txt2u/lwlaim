#include <entities/material.h>
#include <glad/glad.h>

#include <stb_image.h>
#include <stdio.h>
#include <stdlib.h>

Material *material_create_with_texture(const char *texture_path, const float base_color[4]) {
    Material *material = (Material *)malloc(sizeof(Material));
    if (!material) {
        printf("Failed to allocate memory for material.\n");
        return NULL;
    }

    // Load the texture
    material->texture_id = load_texture(texture_path);

    // Set the base color (used when there is no texture)
    material->base_color[0] = base_color[0];
    material->base_color[1] = base_color[1];
    material->base_color[2] = base_color[2];
    material->base_color[3] = base_color[3];

    material->metallic = 0.0f;
    material->roughness = 0.5f;  // Default value

    return material;
}

Material *material_create_with_color(const float base_color[4]) {
    Material *material = (Material *)malloc(sizeof(Material));
    if (!material) {
        printf("Failed to allocate memory for material.\n");
        return NULL;
    }

    material->texture_id = 0;  // No texture
    material->base_color[0] = base_color[0];
    material->base_color[1] = base_color[1];
    material->base_color[2] = base_color[2];
    material->base_color[3] = base_color[3];

    material->metallic = 0.0f;
    material->roughness = 0.5f;  // Default value

    return material;
}

GLuint load_texture(const char *texture_path) {
    int width, height, channels;
    unsigned char *data = stbi_load(texture_path, &width, &height, &channels, 0);

    if (!data) {
        printf("Failed to load texture: %s\n", texture_path);
        return 0;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);

    return texture;
}

void material_free(Material *material) {
    if (material) {
        if (material->texture_id) {
            glDeleteTextures(1, &material->texture_id);
        }
        free(material);
    }
}

void material_apply_to_shader(Material *material, GLuint shader_program) {
    // Apply texture if present
    if (material->texture_id) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, material->texture_id);
        glUniform1i(glGetUniformLocation(shader_program, "texture1"), 0);  // Bind texture1 to texture unit 0
    } else {
        glUniform1i(glGetUniformLocation(shader_program, "texture1"), -1);  // No texture
    }

    // Apply base color
    glUniform4fv(glGetUniformLocation(shader_program, "material_base_color"), 1, material->base_color);
    
    // Apply metallic and roughness values
    glUniform1f(glGetUniformLocation(shader_program, "material_metallic"), material->metallic);
    glUniform1f(glGetUniformLocation(shader_program, "material_roughness"), material->roughness);
}