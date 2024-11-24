#include <entities/model.h>
#include <entities/mesh.h>
#include <glad/glad.h>
#include <cgltf.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <stb_image.h>
#include <cglm/cglm.h>
#include <cglm/struct.h>

// Function to load mesh from GLTF
static Mesh *load_mesh_from_gltf(cgltf_mesh *gltf_mesh, const cgltf_data *gltf_data, cgltf_node *node, bool apply_parent_transform) {
    Mesh *mesh = mesh_create(gltf_mesh->name ? gltf_mesh->name : "unknown_or_singular_mesh_type");

    // Assume single primitive for simplicity
    cgltf_primitive *primitive = &gltf_mesh->primitives[0];

    // Load vertex data (positions, normals, texcoords)
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

    // Set position, scale, and rotation from the node
    if (node) {
        if (node->has_translation) {
            mesh->position[0] = node->translation[0];
            mesh->position[1] = -node->translation[2];
            mesh->position[2] = node->translation[1];
        }
        if (node->has_scale) {
            mesh->scale[0] = node->scale[0];
            mesh->scale[1] = node->scale[2];
            mesh->scale[2] = node->scale[1];
        }
        if (node->has_rotation) {
            mesh->rotation[0] = node->rotation[1];
            mesh->rotation[1] = node->rotation[2];
            mesh->rotation[2] = node->rotation[3];
            mesh->rotation[3] = node->rotation[0];
        }
    } else {
        glm_vec3_zero(mesh->position);
        glm_vec3_one(mesh->scale);
        glm_quat_identity(mesh->rotation);
    }

    // Apply parent transformation if requested
    if (apply_parent_transform && node && node->parent) {
        cgltf_node *parent_node = node->parent;
        if (parent_node->has_translation) {
            glm_vec3_mul(mesh->position, parent_node->translation, mesh->position);
        }
        if (parent_node->has_scale) {
            glm_vec3_mul(mesh->scale, parent_node->scale, mesh->scale);
        }
        if (parent_node->has_rotation) {
            glm_quat_mul(mesh->rotation, parent_node->rotation, mesh->rotation);
        }
    }

    // Update transformation matrix
    mesh_update_transform_matrix(mesh);
    return mesh;
}

// Model transformation functions
void model_set_position(Model *model, vec3 new_position) {
    glm_vec3_copy(new_position, model->position);  // Copy the new position into the model
}

void model_set_scale(Model *model, vec3 new_scale) {
    glm_vec3_copy(new_scale, model->scale);  // Copy the new scale into the model
}

void model_set_rotation(Model *model, versor rotation) {
    glm_quat_copy(rotation, model->rotation);  // Copy the quaternion into the model
}

// Rotation quat
void create_rotation_quaternion(versor q, vec3 axis, float angle) {
    glm_quatv(q, angle, axis);  // Create a quaternion from the axis and angle
}

void model_apply_transform(Model *model) {
    // Ensure scale has safe default values
    if (model->scale[0] == 0.0f && model->scale[1] == 0.0f && model->scale[2] == 0.0f) {
        model->scale[0] = 1.0f;
        model->scale[1] = 1.0f;
        model->scale[2] = 1.0f;
    }

    // Start with the identity matrix
    glm_mat4_identity(model->transform_matrix);

    // Combine transformations in the correct order: scale -> rotate -> translate

    // Apply scale
    glm_scale(model->transform_matrix, model->scale);

    // Apply rotation
    glm_rotate(model->transform_matrix, glm_rad(model->rotation[0]), (vec3){1.0f, 0.0f, 0.0f});
    glm_rotate(model->transform_matrix, glm_rad(model->rotation[1]), (vec3){0.0f, 1.0f, 0.0f});
    glm_rotate(model->transform_matrix, glm_rad(model->rotation[2]), (vec3){0.0f, 0.0f, 1.0f});

    // Apply translation
    glm_translate(model->transform_matrix, model->position);
}

// GLTF loading function
int model_load_gltf(Model *model, const char *texture_path, const char *file_path, bool apply_parent_transform) {
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

    model->mesh_count = gltf_data->meshes_count;
    model->meshes = malloc(sizeof(Mesh *) * model->mesh_count);
    if (!model->meshes) {
        cgltf_free(gltf_data);
        printf("Failed to allocate memory for meshes.\n");
        return -3;
    }

    for (size_t i = 0; i < gltf_data->meshes_count; i++) {
        cgltf_mesh *gltf_mesh = &gltf_data->meshes[i];
        model->meshes[i] = load_mesh_from_gltf(gltf_mesh, gltf_data, &gltf_data->nodes[i], apply_parent_transform);
        if (!model->meshes[i]) {
            printf("Failed to load mesh %zu.\n", i);
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
        unsigned char *data = stbi_load(texture_path, &width, &height, &channels, 0);

        if (data) {
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

            model->texture_id = texture;
            printf("Created texture with ID: %u\n", texture);
        } else {
            printf("Failed to load texture: %s\n", texture_path);
        }
    }

    model_set_position(model, GLM_VEC3_ZERO);
    model_set_scale(model, GLM_VEC3_ONE);
    model_set_rotation(model, (vec4){1.0f, 0.0f, 0.0f, 0.0f});
    model_apply_transform(model);

    cgltf_free(gltf_data);
    return 1;
}

// Free model resources
void model_free(Model *model) {
    if (model) {
        for (uint32_t i = 0; i < model->mesh_count; i++) {
            mesh_free(model->meshes[i]);
        }
        free(model->meshes);
        free(model);
    }
}