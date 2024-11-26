#include <entities/mesh.h>
#include <cglm/cglm.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Constructor for creating a new mesh with a name
Mesh *mesh_create(const char *name) {
    Mesh *new_mesh = (Mesh *)malloc(sizeof(Mesh));
    if (!new_mesh) {
        printf("Error: Unable to allocate memory for mesh.\n");
        return NULL;
    }

    // Initialize the mesh with default values
    new_mesh->name = name ? strdup(name) : NULL;  // Duplicate name or set to NULL
    new_mesh->vertices = NULL;
    new_mesh->normals = NULL;
    new_mesh->texcoords = NULL;
    new_mesh->vertex_count = 0;
    new_mesh->indices = NULL;
    new_mesh->index_count = 0;

    glm_vec3_zero(new_mesh->min_bound);  // Default bounding box
    glm_vec3_zero(new_mesh->max_bound);

    glm_vec3_zero(new_mesh->position);   // Default position (0, 0, 0)
    glm_vec3_one(new_mesh->scale);      // Default scale (1, 1, 1)
    glm_quat_identity(new_mesh->rotation); // Default rotation (identity quaternion)

    glm_mat4_identity(new_mesh->transform_matrix); // Default transform matrix

    // Initialize the material pointer to NULL (set from outside when used)
    new_mesh->material = NULL;

    return new_mesh;
}

// Function to set the material for the mesh
void mesh_set_material(Mesh *mesh, Material *material) {
    if (mesh) {
        mesh->material = material;
    }
}

// Function to update the mesh's transform matrix
void mesh_update_transform_matrix(Mesh *mesh) {
    if (!mesh) return;

    mat4 translation_matrix, rotation_matrix, scale_matrix;

    // Create translation matrix
    glm_translate_make(translation_matrix, mesh->position);

    // Create rotation matrix from quaternion
    glm_quat_mat4(mesh->rotation, rotation_matrix);

    // Create scale matrix
    glm_scale_make(scale_matrix, mesh->scale);

    // Combine the transformation matrices in the correct order (scale -> rotate -> translate)
    glm_mat4_identity(mesh->transform_matrix);  // Start with identity matrix
    glm_mat4_mul(scale_matrix, mesh->transform_matrix, mesh->transform_matrix);
    glm_mat4_mul(rotation_matrix, mesh->transform_matrix, mesh->transform_matrix);
    glm_mat4_mul(translation_matrix, mesh->transform_matrix, mesh->transform_matrix);
}

// Function to free a mesh
void mesh_free(Mesh *mesh) {
    if (mesh) {
        if (mesh->name) free(mesh->name);
        if (mesh->vertices) free(mesh->vertices);
        if (mesh->normals) free(mesh->normals);
        if (mesh->texcoords) free(mesh->texcoords);
        if (mesh->indices) free(mesh->indices);
        
        // Free material if it exists
        if (mesh->material) {
            material_free(mesh->material);  // Free the material resources
            free(mesh->material);           // Free the material pointer if dynamically allocated
        }
        
        free(mesh);
    }
}
