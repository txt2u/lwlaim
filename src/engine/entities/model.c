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

// Function to load a texture from the GLTF data
static Material *load_material_from_gltf(cgltf_material *gltf_material, const char* model_path) {
    // Allocate memory for the Material struct
    Material *material = malloc(sizeof(Material));
    if (!material) {
        printf("[GLTF_MODEL] Failed to allocate memory for material.\n");
        return NULL;
    }

	if (!gltf_material) {
        printf("[GLTF_MODEL] No GLTF Material was defined.\n");
        return NULL;
    }

    // Initialize material properties with default values
    memset(material, 0, sizeof(Material));
    glm_vec4_copy((vec4){1.0f, 1.0f, 1.0f, 1.0f}, material->diffuse_color); // Default diffuse color
    glm_vec3_copy((vec3){0.0f, 0.0f, 0.0f}, material->emissive_color);      // Default emissive color
    material->metallic = 0.0f;                                            // Default metallic
    material->roughness = 1.0f;                                           // Default roughness

	// Load diffuse texture
	if (gltf_material->pbr_metallic_roughness.base_color_texture.texture) {
		cgltf_texture *base_color_texture = gltf_material->pbr_metallic_roughness.base_color_texture.texture;
		material->diffuse_texture_id = load_texture_from_gltf(base_color_texture, model_path);
		if (!material->diffuse_texture_id) {
			printf("Warning: Failed to load diffuse texture.\n");
		} else {
			printf("Success: Made diffuse texture with the following id \"%i\"\n", material->diffuse_texture_id);
		}
	} else {
		printf("No diffuse texture defined in material.\n");
	}

	// Fallback default diffuse texture
	if (!material->diffuse_texture_id) {
		printf("Using default white texture as fallback for diffuse.\n");
		glGenTextures(1, &material->diffuse_texture_id);
		glBindTexture(GL_TEXTURE_2D, material->diffuse_texture_id);

		unsigned char white_pixel[4] = {255, 255, 255, 255};
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white_pixel);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

    // // Check if the material has a valid PBR metallic-roughness component
    // if (gltf_material->has_pbr_metallic_roughness) {
    //     // Diffuse color
    //     if (sizeof(gltf_material->pbr_metallic_roughness.base_color_factor)) {
    //         glm_vec4_copy(gltf_material->pbr_metallic_roughness.base_color_factor, material->diffuse_color);
    //     }

    //     // Metallic and roughness factors
    //     if (gltf_material->pbr_metallic_roughness.metallic_factor >= 0.0f) {
    //         material->metallic = gltf_material->pbr_metallic_roughness.metallic_factor;
    //     }
    //     if (gltf_material->pbr_metallic_roughness.roughness_factor >= 0.0f) {
    //         material->roughness = gltf_material->pbr_metallic_roughness.roughness_factor;
    //     }

    //     // Load diffuse texture
    //     // if (gltf_material->pbr_metallic_roughness.base_color_texture.texture) {
    //     //     cgltf_texture *base_color_texture = gltf_material->pbr_metallic_roughness.base_color_texture.texture;
    //     //     material->diffuse_texture_id = load_texture_from_gltf(base_color_texture, model_path);
    //     //     if (!material->diffuse_texture_id) {
    //     //         printf("Warning: Failed to load diffuse texture.\n");
    //     //     } else {
	// 	// 		printf("Success: Made diffuse texture with the following id \"%i\"\n", material->diffuse_texture_id);
	// 	// 	}
    //     // }

    //     // Load metallic-roughness texture
    //     if (gltf_material->pbr_metallic_roughness.metallic_roughness_texture.texture) {
    //         cgltf_texture *metallic_roughness_texture = gltf_material->pbr_metallic_roughness.metallic_roughness_texture.texture;
    //         material->metallic_roughness_texture_id = load_texture_from_gltf(metallic_roughness_texture, model_path);
    //         if (!material->metallic_roughness_texture_id) {
    //             printf("Warning: Failed to load metallic-roughness texture.\n");
    //         } else {
	// 			printf("Success: Made metallic with the following id \"%i\"\n", material->metallic_roughness_texture_id);
	// 		}
    //     }

    // }

    // Load normal texture
    if (gltf_material->normal_texture.texture) {
        cgltf_texture *normal_texture = gltf_material->normal_texture.texture;
        material->normal_texture_id = load_texture_from_gltf(normal_texture, model_path);
        if (!material->normal_texture_id) {
            printf("Warning: Failed to load normal texture.\n");
        } else {
			printf("Success: Made normal with the following id \"%i\"\n", material->normal_texture_id);
		}
    }

    // Load occlusion texture
    if (gltf_material->occlusion_texture.texture) {
        cgltf_texture *occlusion_texture = gltf_material->occlusion_texture.texture;
        material->occlusion_texture_id = load_texture_from_gltf(occlusion_texture, model_path);
        if (!material->occlusion_texture_id) {
            printf("Warning: Failed to load occlusion texture.\n");
        } else {
			printf("Success: Made occlusion with the following id \"%i\"\n", material->occlusion_texture_id);
		}
    }

    // Load emissive texture
    if (gltf_material->emissive_texture.texcoord) {
        cgltf_texture *emissive_texture = gltf_material->emissive_texture.texture;
        material->emissive_texture_id = load_texture_from_gltf(emissive_texture, model_path);
        if (!material->emissive_texture_id) {
            printf("Warning: Failed to load emissive texture.\n");
        }  else {
			printf("Success: Made emissive with the following id \"%i\"\n", material->emissive_texture_id);
		}
    }

    // Emissive color
    if (sizeof(gltf_material->emissive_factor)) {
        glm_vec3_copy(gltf_material->emissive_factor, material->emissive_color);
    }

    return material;
}

static Mesh *load_mesh_from_gltf(cgltf_mesh *gltf_mesh, const cgltf_data *gltf_data, cgltf_node *node, bool apply_parent_transform, Model *model, const char* model_path) {
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
    if (node && apply_parent_transform) {
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
    } else if (node && !apply_parent_transform) {
        mesh->position[0] = model->position[0];
        mesh->position[1] = model->position[1];
        mesh->position[2] = model->position[2];

        mesh->rotation[0] = model->rotation[1];
        mesh->rotation[1] = model->rotation[2];
        mesh->rotation[2] = model->rotation[3];
        mesh->rotation[3] = model->rotation[0];
        
        mesh->scale[0] = model->scale[0];
        mesh->scale[1] = model->scale[2];
        mesh->scale[2] = model->scale[1];
    }

    // Assign material to mesh (if the primitive has a material)
    if (primitive->material) {
        Material *material = load_material_from_gltf(primitive->material, model_path);
        mesh_set_material(mesh, material);
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
int model_load_gltf(Model *model, const char *file_path, bool apply_parent_transform) {
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
    model->materials = malloc(sizeof(Material *) * model->mesh_count);

    if (!model->meshes || !model->materials) {
        cgltf_free(gltf_data);
        printf("Failed to allocate memory for meshes or materials.\n");
        return -3;
    }

    for (size_t i = 0; i < gltf_data->meshes_count; i++) {
        cgltf_mesh *gltf_mesh = &gltf_data->meshes[i];
        cgltf_node *node = &gltf_data->nodes[i]; // Assume nodes are indexed like meshes for simplicity

        model->meshes[i] = load_mesh_from_gltf(gltf_mesh, gltf_data, node, apply_parent_transform, model, file_path);
        if (!model->meshes[i]) {
            printf("Failed to load mesh %zu.\n", i);
            for (size_t j = 0; j < i; j++) {
                mesh_free(model->meshes[j]);
            }
            free(model->meshes);
            cgltf_free(gltf_data);
            return -4;
        }

        // Load and assign the material to the model
        if (gltf_mesh->primitives[0].material) {
            model->materials[i] = load_material_from_gltf(gltf_mesh->primitives[0].material, file_path);
        }
    }

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