#include <loaders/obj.h>
#include <entites/mesh.h>
#include <entites/model.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stb_image.h>
#include <cglm/cglm.h>
#include <math.h>

// Apply translation to the model
void model_translate(Model *model, vec3 translation) {
    glm_vec3_copy(translation, model->translation);
    // Recompute the model matrix
    glm_mat4_identity(model->mesh->modelMatrix);
    glm_translate(model->mesh->modelMatrix, model->translation); // Apply translation
}


// Apply rotation to the model (with conversion from degrees to radians)
void model_rotate(Model *model, float angle, vec3 axis) {
    model->rotationAngle = glm_rad(angle);  // Convert angle to radians
    glm_vec3_copy(axis, model->rotationAxis);

    // Recompute the model matrix
    glm_mat4_identity(model->mesh->modelMatrix); // Reset the model matrix
    glm_translate(model->mesh->modelMatrix, model->translation); // Apply translation
    glm_rotate(model->mesh->modelMatrix, model->rotationAngle, model->rotationAxis); // Apply rotation
}

// Apply scaling to the model
void model_scale(Model *model, vec3 scale) {
    glm_vec3_copy(scale, model->scale);

    // Recompute the model matrix
    glm_mat4_identity(model->mesh->modelMatrix); // Reset the model matrix
    glm_translate(model->mesh->modelMatrix, model->translation); // Apply translation
    glm_rotate(model->mesh->modelMatrix, model->rotationAngle, model->rotationAxis); // Apply rotation
    glm_scale(model->mesh->modelMatrix, model->scale); // Apply scaling
}

// Load the model data from an OBJ file
int load_model(const char* filename, const char* texture_path, Model* model, int program_id) {
    StaticModel sm;  // Declare StaticModel, not a pointer
    // Load the OBJ file
    printf("Loading model from %s\n", filename);
    if (load_obj(filename, &sm) == 0) {
        printf("Failed to load OBJ file: %s\n", filename);
        return 0;  // Failed to load
    }

    // Transfer data from StaticModel to Model
    model->vertex_count = sm.vertex_count;
    model->index_count = sm.index_count;
    model->vertices = sm.vertices;
    model->indices = sm.indices;
    model->texcoords = sm.texcoords;
    model->normal_count = sm.normal_count;

    // Check if index count is valid
    if (model->index_count == 0) {
        printf("[FAIL] model->index_count returned a 0!\n");
        return -1;
    }

    // Determine the texcoord count based on the texture coordinates array size
    if (model->texcoords != NULL) {
        model->texcoord_count = model->vertex_count;  // Assuming 2D UV coordinates, count is equal to vertex count
    } else {
        model->texcoord_count = 0;  // No texture coordinates found
    }

    // Apply default UVs if the model doesn't have any
    if (model->texcoords == NULL) {
        model->texcoords = (float*)malloc(model->vertex_count * 2 * sizeof(float));  // Allocate space for UVs
        if (model->texcoords == NULL) {
            printf("Failed to allocate memory for texcoords\n");
            return 0;
        }
        generate_default_uvs(model->vertices, model->vertex_count, model->texcoords);
        model->texcoord_count = model->vertex_count;  // Set texcoord_count after default UVs are generated
    }

    model->textureFile = (char*)texture_path;

    // Create the mesh from the loaded model data
    model->mesh = mesh_create(
        model->vertices, model->vertex_count, 
        model->indices, model->index_count, 
        model->texcoords, model->texcoord_count,
        model->normals, model->normal_count,
        model->textureFile, program_id
    );
    
    if (model->mesh == NULL) {
        printf("Failed to create mesh for model: %s\n", filename);
        return 0;  // Failed to create mesh
    }
    printf("Created mesh!\n");

    return 1;  // Successfully loaded
}

// Free the model data
void free_model(Model *model) {
    if (model) {
        // Free the mesh
        if (model->mesh) {
            mesh_destroy(model->mesh);
        }

        // Free the vertices, indices, texcoords, and other arrays
        if (model->vertices) free(model->vertices);
        if (model->normals) free(model->normals);
        if (model->texcoords) free(model->texcoords);
        if (model->indices) free(model->indices);
    }
}

// Render the model
void render_model(Model *model, mat4 viewMatrix, mat4 projectionMatrix) {
    // Apply transformations (if any)
    mat4 modelMatrix;
    glm_mat4_identity(modelMatrix);  // Reset to identity matrix

    // Apply model transformations (scale, rotate, translate)
    mesh_scale(model->mesh, model->scale);
    mesh_rotate(model->mesh, model->rotationAngle, model->rotationAxis);
    mesh_translate(model->mesh, model->translation);

    // Set the final model matrix to the one in the mesh
    glm_mat4_copy(model->mesh->modelMatrix, modelMatrix);

    // Render the mesh with the correct modelMatrix
    mesh_render(model->mesh, viewMatrix, projectionMatrix);
}

// Generate default UVs for the model (cube mapping or any other method)
void generate_default_uvs(float *vertices, int vertexCount, float *texcoords) {
    // Simple cube UV mapping (example, adjust as needed)
    for (int i = 0; i < vertexCount; i++) {
        texcoords[i * 2] = vertices[i * 3] * 0.5f + 0.5f;  // u
        texcoords[i * 2 + 1] = vertices[i * 3 + 1] * 0.5f + 0.5f;  // v
    }
}