#include <entities/drawable.h>
#include <glad/glad.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Initializes a drawable object with a mesh
void draw_manager_init_from_mesh(Drawable* p_drawable, Mesh* mesh, const char* name) {
    if (!p_drawable || !mesh || p_drawable->mesh_count >= MAX_MESHES) return;

    DrawableMesh* new_mesh = &p_drawable->meshes[p_drawable->mesh_count++];
    new_mesh->mesh = mesh;
    new_mesh->name = name;  // Set the name of the mesh

    // Initialize the buffers for the mesh
    new_mesh->buffers = buffers_create_empty();
    buffers_bind_vao(new_mesh->buffers.VAO);

    // Load vertex positions (VBO)
    new_mesh->buffers.VBO = buffers_create_vbo(mesh->vertices, mesh->vertex_count * 3);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    // Load normals (if available)
    if (mesh->normals) {
        new_mesh->buffers.NormalVBO = buffers_create_vbo(mesh->normals, mesh->vertex_count * 3);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(1);
    }

    // Load texture coordinates (if available)
    if (mesh->texcoords) {
        new_mesh->buffers.TexCoordVBO = buffers_create_vbo(mesh->texcoords, mesh->vertex_count * 2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(2);
    }

    // Load indices (if available)
    if (mesh->indices) {
        new_mesh->buffers.EBO = buffers_create_ebo(mesh->indices, mesh->index_count);
    }

    // Unbind VAO after setup

    // Initialize model matrix to identity
    glm_mat4_identity(p_drawable->model_matrix);

    // Initialize translation, scaling, and rotation to defaults
    glm_vec3_zero(p_drawable->translation);
    glm_vec3_one(p_drawable->scale);
    glm_vec3_zero(p_drawable->rotation);
}

// Draws a specific mesh in the drawable object by its name
void draw_manager_draw(Drawable* drawable, const char* mesh_name) {
    if (!drawable || !mesh_name) return;

    // Find the mesh by name
    DrawableMesh* mesh_to_draw = NULL;
    for (int i = 0; i < drawable->mesh_count; ++i) {
        if (strcmp(drawable->meshes[i].name, mesh_name) == 0) {
            mesh_to_draw = &drawable->meshes[i];
            break;
        }
    }

    if (!mesh_to_draw) {
        // printf("Mesh with name %s not found!\n", mesh_name);
        return;  // Mesh with the specified name not found
    }

    // Apply transformations to the model matrix
    mat4 modelMatrix;
    glm_mat4_identity(modelMatrix);

    // Apply translation
    mat4 translation_matrix;
    glm_translate_make(translation_matrix, drawable->translation);  // Create translation matrix
    glm_mat4_mul(modelMatrix, translation_matrix, modelMatrix);      // Apply translation

    // Apply scaling
    mat4 scale_matrix;
    glm_scale_make(scale_matrix, drawable->scale);  // Create scaling matrix
    glm_mat4_mul(modelMatrix, scale_matrix, modelMatrix);  // Apply scaling

    // Apply rotation (assuming rotation is in Euler angles)
    mat4 rotation_matrix;

    // Apply rotation on X-axis (Pitch)
    glm_rotate_make(rotation_matrix, glm_rad(drawable->rotation[0]), (vec3){1.0f, 0.0f, 0.0f});
    glm_mat4_mul(modelMatrix, rotation_matrix, modelMatrix);

    // Apply rotation on Y-axis (Yaw)
    glm_rotate_make(rotation_matrix, glm_rad(drawable->rotation[1]), (vec3){0.0f, 1.0f, 0.0f});
    glm_mat4_mul(modelMatrix, rotation_matrix, modelMatrix);

    // Apply rotation on Z-axis (Roll)
    glm_rotate_make(rotation_matrix, glm_rad(drawable->rotation[2]), (vec3){0.0f, 0.0f, 1.0f});
    glm_mat4_mul(modelMatrix, rotation_matrix, modelMatrix);

    // Now copy the combined modelMatrix to drawable->model_matrix
    glm_mat4_copy(modelMatrix, drawable->model_matrix);

    buffers_bind_vao(mesh_to_draw->buffers.VAO);

    // Bind VBO
    buffers_bind_vbo(mesh_to_draw->buffers.VBO);

    // Bind normal VBO if it exists
    if (mesh_to_draw->buffers.NormalVBO) {
        buffers_bind_vbo(mesh_to_draw->buffers.NormalVBO);
    }

    // Bind texture coordinates VBO if it exists
    if (mesh_to_draw->buffers.TexCoordVBO) {
        buffers_bind_vbo(mesh_to_draw->buffers.TexCoordVBO);
    }

    // Bind EBO if it exists
    if (mesh_to_draw->buffers.EBO) {
        buffers_bind_ebo(mesh_to_draw->buffers.EBO);
        glDrawElements(GL_TRIANGLES, mesh_to_draw->mesh->index_count, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, mesh_to_draw->mesh->vertex_count);
    }

    // Unbind VAO after drawing
    buffers_unbind_vao();
}

// Applies translation to the drawable object
void draw_manager_translate(Drawable* drawable, vec3 translation) {
    if (!drawable) return;

    glm_vec3_copy(translation, drawable->translation);
}

// Applies scaling to the drawable object
void draw_manager_scale(Drawable* drawable, vec3 scale) {
    if (!drawable) return;

    glm_vec3_copy(scale, drawable->scale);
}

// Applies rotation to the drawable object
void draw_manager_rotate(Drawable* drawable, vec3 rotation) {
    if (!drawable) return;

    glm_vec3_copy(rotation, drawable->rotation);
}

// Cleans up drawable buffers
void draw_manager_destroy(Drawable* drawable) {
    if (!drawable) return;
    
    for (int i = 0; i < drawable->mesh_count; ++i) {
        buffers_destroy(&drawable->meshes[i].buffers);
    }
}
