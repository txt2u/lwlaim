#include <entities/material.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stb_image.h>
#include <cgltf.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define PATH_SEPARATOR '/'

#ifdef _WIN32
	#include <win_dirent.h>
#else
	#include <dirent.h>
#endif

// Helper function to check if a path is a directory
int is_directory(const char *path) {
    struct stat path_stat;
    if (stat(path, &path_stat) != 0) {
        return 0; // Cannot stat path, treat as not a directory
    }
    return S_ISDIR(path_stat.st_mode);
}

// Helper function to get the parent directory of a file
void get_parent_directory(const char *file_path, char *parent_directory) {
    const char *last_slash = strrchr(file_path, '/');

    if (last_slash) {
        size_t length = last_slash - file_path;
        strncpy(parent_directory, file_path, length);
        parent_directory[length] = '\0'; // Null-terminate the string
    } else {
        strcpy(parent_directory, "."); // Current directory as fallback
    }
}

// Recursive file search in a directory
char *find_file_in_directory(const char *base_dir, const char *target_file) {
    DIR *dir;
    struct dirent *entry;

    if ((dir = opendir(base_dir)) == NULL) {
        return NULL; // Cannot open directory
    }

    while ((entry = readdir(dir)) != NULL) {
        // Skip the current and parent directory entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Build the full path for the current entry
        char path[1024];
        snprintf(path, sizeof(path), "%s%c%s", base_dir, PATH_SEPARATOR, entry->d_name);

        if (is_directory(path)) {
            // Recurse into subdirectories
            char *found_path = find_file_in_directory(path, target_file);
            if (found_path) {
                closedir(dir);
                return found_path; // Return the found path if the target is located
            }
        } else if (strcmp(entry->d_name, target_file) == 0) {
            // Target file found
            closedir(dir);
            return strdup(path); // Return a dynamically allocated copy of the file path
        }
    }

    closedir(dir);
    return NULL; // File not found in this directory or its subdirectories
}

// Function to load a texture from a GLTF texture structure
GLuint load_texture_from_gltf(cgltf_texture *gltf_texture, const char *model_path) {
    printf("[load_texture_from_gltf] called.\n");

    // Get parent directory from the model path
    char parent_dir[1024];
    get_parent_directory(model_path, parent_dir);

    printf("Parent directory: %s\n", parent_dir);

    // Get the texture URI from the GLTF texture structure
    cgltf_image *gltf_image = gltf_texture->image;
    if (!gltf_image || !gltf_image->uri) {
        printf("Invalid GLTF image or missing URI.\n");
        return 0;
    }

    // Find the texture file in the directory
    char *texture_path = find_file_in_directory(parent_dir, gltf_image->uri);
    if (!texture_path) {
        printf("Texture file not found: %s\n", gltf_image->uri);
        return 0;
    }

    // Load texture data
    int width, height, channels;
    unsigned char *data = stbi_load(texture_path, &width, &height, &channels, 0);
    free(texture_path); // Free the path string after use

    if (!data) {
        printf("Failed to load texture data: %s\n", gltf_image->uri);
        return 0; // Return 0 if texture loading fails
    }

    // Generate OpenGL texture
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);

    printf("Successfully loaded texture: %s\n", gltf_image->uri);
    return texture_id;
}

// Function to create a GL texture from a file path and assigns it to texture_id_dest
int material_create_gl_texture(cgltf_texture* texture, MaterialTextureType type, GLuint* texture_id_dest, const char* model_path) {
    // Attempt to load the texture and assign it to the destination
    GLuint texture_id = load_texture_from_gltf(texture, model_path);  // For simplicity, skipping the GLTF data

    if (texture_id == 0) {
        printf("Failed to create texture: %s\n", texture->image->name);
        return -1;  // Return error code if texture creation fails
    }

    *texture_id_dest = texture_id;
    return 0;
}

// Function to apply the material (bind its textures) to a shader program
void material_apply(const Material* material, GLuint shader_program_id) {
    glUseProgram(shader_program_id);

	if (material->diffuse_texture_id <= 0) {
		printf("[material_apply->fn] MATERIAL WAS SET TO NULL OR DIDN'T EXIST!\n");
		return;
	}

    // Apply diffuse texture if it exists
    if (material->diffuse_texture_id) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, material->diffuse_texture_id);
        glUniform1i(glGetUniformLocation(shader_program_id, "diffuseTexture"), 0);
		glUniform4fv(glGetUniformLocation(shader_program_id, "diffuseColor"), 1, material->diffuse_color);
    }

    // Apply normal texture if it exists
    if (material->normal_texture_id) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, material->normal_texture_id);
        glUniform1i(glGetUniformLocation(shader_program_id, "normalTexture"), 1);
    }

    // Apply metallic-roughness texture if it exists
    if (material->metallic_roughness_texture_id) {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, material->metallic_roughness_texture_id);
        glUniform1i(glGetUniformLocation(shader_program_id, "metallicRoughnessTexture"), 2);
    }

    // Apply occlusion texture if it exists
    if (material->occlusion_texture_id) {
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, material->occlusion_texture_id);
        glUniform1i(glGetUniformLocation(shader_program_id, "occlusionTexture"), 3);
    }

    // Apply emissive texture if it exists
    if (material->emissive_texture_id) {
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, material->emissive_texture_id);
        glUniform1i(glGetUniformLocation(shader_program_id, "emissiveTexture"), 4);
		glUniform3fv(glGetUniformLocation(shader_program_id, "emissiveColor"), 1, material->emissive_color);
    }

    // Set material properties for diffuse color, metallic, roughness, and emissive color
	if (material->metallic) glUniform1f(glGetUniformLocation(shader_program_id, "metallic"), material->metallic);
    if (material->roughness) glUniform1f(glGetUniformLocation(shader_program_id, "roughness"), material->roughness);
}

// Function to free material resources, including GPU textures
void material_free(Material* material) {
    if (material->diffuse_texture_id) {
        glDeleteTextures(1, &material->diffuse_texture_id);
    }
    if (material->normal_texture_id) {
        glDeleteTextures(1, &material->normal_texture_id);
    }
    if (material->metallic_roughness_texture_id) {
        glDeleteTextures(1, &material->metallic_roughness_texture_id);
    }
    if (material->occlusion_texture_id) {
        glDeleteTextures(1, &material->occlusion_texture_id);
    }
    if (material->emissive_texture_id) {
        glDeleteTextures(1, &material->emissive_texture_id);
    }

    free(material);
}