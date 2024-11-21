#include <entities/model.h>
#include <entities/mesh.h>
#include <cgltf.h> // Use cgltf for GLTF parsing
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ! ###########################################################
// ! ###########################################################
// */ 
// */ The problem of this function is that the vertices, texcoords
// */ are way too off and wrong. For example:
// */ . ________________________.
// */ | gl_Position | texCoords |
// */ | x, y, z     | u, v      |
// */ | 600.0...... | -4.0, 1.4 |
// */ ._________________________.
// */ SO MAKE SURE TO FIX THIS BEFORE USING AND TURNING INTO SICKO MODE!
// */ SO MAKE SURE TO FIX THIS BEFORE USING AND TURNING INTO SICKO MODE!
// */ SO MAKE SURE TO FIX THIS BEFORE USING AND TURNING INTO SICKO MODE!
// */ SO MAKE SURE TO FIX THIS BEFORE USING AND TURNING INTO SICKO MODE!
// */ SO MAKE SURE TO FIX THIS BEFORE USING AND TURNING INTO SICKO MODE!
// */ SO MAKE SURE TO FIX THIS BEFORE USING AND TURNING INTO SICKO MODE!
// */ 
// ! ###########################################################
// ! ###########################################################
static Mesh *load_mesh_from_gltf(cgltf_mesh *gltf_mesh, const cgltf_data *gltf_data) {
    Mesh *mesh = mesh_create();
    // Iterate through primitives (assuming triangles)
    cgltf_primitive *primitive = &gltf_mesh->primitives[0];
    
    // Load vertex data
    for (size_t i = 0; i < primitive->attributes_count; i++) {
        cgltf_attribute *attribute = &primitive->attributes[i];
        cgltf_accessor *accessor = attribute->data;
        if (attribute->type == cgltf_attribute_type_position) {
            mesh->vertex_count = accessor->count;
            mesh->vertices = malloc(sizeof(float) * 3 * accessor->count);
            cgltf_accessor_read_float(accessor, 0, mesh->vertices, 3 * accessor->count);
        } else if (attribute->type == cgltf_attribute_type_normal) {
            mesh->normals = malloc(sizeof(float) * 3 * accessor->count);
            cgltf_accessor_read_float(accessor, 0, mesh->normals, 3 * accessor->count);
        } else if (attribute->type == cgltf_attribute_type_texcoord) {
            mesh->texcoords = malloc(sizeof(float) * 2 * accessor->count);
            cgltf_accessor_read_float(accessor, 0, mesh->texcoords, 2 * accessor->count);
        }
    }

    // Load indices
    if (primitive->indices) {
        mesh->index_count = primitive->indices->count;
        mesh->indices = malloc(sizeof(uint32_t) * primitive->indices->count);
        for (size_t i = 0; i < primitive->indices->count; i++) {
            mesh->indices[i] = cgltf_accessor_read_index(primitive->indices, i);
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

int model_load_gltf(Model *model, const char *file_path) {
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
