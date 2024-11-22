#include <entities/drawable.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void draw_manager_init_from_mesh(Drawable* p_drawable, Mesh* mesh) {
    if (!p_drawable || !mesh) return;  // Ensure the drawable and mesh pointers are valid

    // Initialize the buffers for the drawable
    p_drawable->buffers = buffers_create_empty();
    buffers_bind_vao(p_drawable->buffers.VAO);

    // Load vertex positions (VBO)
    p_drawable->buffers.VBO = buffers_create_vbo(mesh->vertices, mesh->vertex_count * 3);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    // Load normals (if available)
    if (mesh->normals) {
        p_drawable->buffers.NormalVBO = buffers_create_vbo(mesh->normals, mesh->vertex_count * 3);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(1);
    }

    // Load texture coordinates (if available)
    if (mesh->texcoords) {
        p_drawable->buffers.TexCoordVBO = buffers_create_vbo(mesh->texcoords, mesh->vertex_count * 2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(2);
    }

    // Load indices (if available)
    if (mesh->indices) {
        p_drawable->buffers.EBO = buffers_create_ebo(mesh->indices, mesh->index_count);
    }

    // Set the mesh pointer
    p_drawable->mesh = mesh;

    // Unbind VAO after setup
    glBindVertexArray(0);

    // Initialize model matrix to identity
    glm_mat4_identity(p_drawable->model_matrix);
}

void draw_manager_draw(Drawable* drawable) {
    if (!drawable) return;  // Check if drawable is valid

    // Start with the identity matrix
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

    buffers_bind_vao(drawable->buffers.VAO);

    // Bind VBO
    buffers_bind_vbo(drawable->buffers.VBO);
    
    // Bind normal VBO if it exists
    if (drawable->buffers.NormalVBO) {
        buffers_bind_vbo(drawable->buffers.NormalVBO);
    }
    
    // Bind texture coordinates VBO if it exists
    if (drawable->buffers.TexCoordVBO) {
        buffers_bind_vbo(drawable->buffers.TexCoordVBO);
    }
    
    // Bind EBO if it exists
    if (drawable->buffers.EBO) {
        buffers_bind_ebo(drawable->buffers.EBO);
        glDrawElements(GL_TRIANGLES, drawable->mesh->index_count, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, drawable->mesh->vertex_count);
    }

    // Unbind VAO after drawing
    buffers_unbind_vao();
}

// Applies translation to the drawable object
void draw_manager_translate(Drawable* drawable, vec3 translation) {
    if (!drawable) return;

    // Update translation in the drawable struct
    glm_vec3_copy(translation, drawable->translation);
}

// Applies scaling to the drawable object
void draw_manager_scale(Drawable* drawable, vec3 scale) {
    if (!drawable) return;

    // Update scale in the drawable struct
    glm_vec3_copy(scale, drawable->scale);
}

// Applies rotation to the drawable object
void draw_manager_rotate(Drawable* drawable, vec3 rotation) {
    if (!drawable) return;

    // Update rotation (Euler angles) in the drawable struct
    glm_vec3_copy(rotation, drawable->rotation);
}

void draw_manager_destroy(Drawable* drawable) {
    if (!drawable) return;
    buffers_destroy(&drawable->buffers);
}
