#include <entities/model.h>
#include <entities/mesh.h>
#include <glad/glad.h>
#include <cgltf.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <stb_image.h>

static Mesh *load_mesh_from_gltf(cgltf_mesh *gltf_mesh, const cgltf_data *gltf_data) {
    Mesh *mesh = mesh_create();
    // Assume single primitive
    cgltf_primitive *primitive = &gltf_mesh->primitives[0];

    // Load vertex data
    for (size_t i = 0; i < primitive->attributes_count; i++) {
        cgltf_attribute *attribute = &primitive->attributes[i];
        cgltf_accessor *accessor = attribute->data;
        
        size_t num_components = cgltf_num_components(accessor->type);
        
        if (attribute->type == cgltf_attribute_type_position) {
            mesh->vertex_count = accessor->count;
            mesh->vertices = malloc(sizeof(float) * num_components * accessor->count);
            for (size_t j = 0; j < accessor->count; j++) {
                float temp[3];
                cgltf_accessor_read_float(accessor, j, temp, num_components);
                for (size_t k = 0; k < num_components; k++) {
                    mesh->vertices[j * num_components + k] = temp[k];
                }
            }
        } else if (attribute->type == cgltf_attribute_type_normal) {
            mesh->normals = malloc(sizeof(float) * num_components * accessor->count);
            for (size_t j = 0; j < accessor->count; j++) {
                float temp[3];
                cgltf_accessor_read_float(accessor, j, temp, num_components);
                for (size_t k = 0; k < num_components; k++) {
                    mesh->normals[j * num_components + k] = temp[k];
                }
            }
        } else if (attribute->type == cgltf_attribute_type_texcoord) {
            mesh->texcoords = malloc(sizeof(float) * 2 * accessor->count);
            for (size_t j = 0; j < accessor->count; j++) {
                float temp[2];
                cgltf_accessor_read_float(accessor, j, temp, 2);
                mesh->texcoords[j * 2 + 0] = temp[0];
                mesh->texcoords[j * 2 + 1] = temp[1];
            }
        }
    }

    // Load indices
    if (primitive->indices) {
        cgltf_accessor *accessor = primitive->indices;
        mesh->index_count = accessor->count;
        mesh->indices = malloc(sizeof(uint32_t) * accessor->count);
        for (size_t i = 0; i < accessor->count; i++) {
            mesh->indices[i] = (uint32_t)cgltf_accessor_read_index(accessor, i);
        }
    }

    // Compute bounding box
    for (uint32_t i = 0; i < mesh->vertex_count; i++) {
        vec3 vertex = {
            mesh->vertices[i * 3 + 0],
            mesh->vertices[i * 3 + 1],
            mesh->vertices[i * 3 + 2]
        };
        glm_vec3_minv(vertex, mesh->min_bound, mesh->min_bound);
        glm_vec3_maxv(vertex, mesh->max_bound, mesh->max_bound);
    }

    return mesh;
}


int model_load_gltf(Model *model, const char *texture_path, const char *file_path) {
    if (!model || !file_path) {
        printf("Invalid parameters: model or file_path is NULL.\n");
        return -1;
    }

    cgltf_data *gltf_data = NULL;
    cgltf_options options = {0};
    cgltf_result result = cgltf_parse_file(&options, file_path, &gltf_data);
    if (result != cgltf_result_success) {
        printf("Failed to parse GLTF file: %s\n", file_path);
        return -1;
    }

    result = cgltf_load_buffers(&options, gltf_data, file_path);
    if (result != cgltf_result_success) {
        cgltf_free(gltf_data);
        printf("Failed to load GLTF buffers: %s\n", file_path);
        return -2;
    }

    // Populate the model
    model->mesh_count = gltf_data->meshes_count;
    model->meshes = malloc(sizeof(Mesh *) * model->mesh_count);
    if (!model->meshes) {
        cgltf_free(gltf_data);
        printf("Failed to allocate memory for meshes.\n");
        return -3;
    }

    for (size_t i = 0; i < gltf_data->meshes_count; i++) {
        model->meshes[i] = load_mesh_from_gltf(&gltf_data->meshes[i], gltf_data);
        if (!model->meshes[i]) {
            printf("Failed to load mesh %zu.\n", i);
            // Clean up already-loaded meshes
            for (size_t j = 0; j < i; j++) {
                mesh_free(model->meshes[j]);
            }
            free(model->meshes);
            cgltf_free(gltf_data);
            return -4;
        }
    }

	if (texture_path) {
		int width, height, channels;
		unsigned char* data = stbi_load(texture_path, &width, &height, &channels, 0);

		if (data) {
			GLuint texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);

			// Set texture parameters
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			// Load texture data into OpenGL
			GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			// Unbind texture and free image data
			glBindTexture(GL_TEXTURE_2D, 0);
			stbi_image_free(data);

			model->texture_id = texture;

			printf("Created texture with ID: %u\n", texture);
		} else {
			printf("Failed to load texture: %s\n", texture_path);
		}
	}

    cgltf_free(gltf_data);
    return 1; // Success
}

void model_free(Model *model) {
    if (model) {
        for (uint32_t i = 0; i < model->mesh_count; i++) {
            mesh_free(model->meshes[i]);
        }
        free(model->meshes);
        free(model);
    }
}
