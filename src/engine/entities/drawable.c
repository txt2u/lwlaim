#include <entities/drawable.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void draw_manager_init_from_mesh(Drawable* p_drawable, Mesh* mesh) {
    // Initialize the drawable object
    Drawable drawable;
    glm_mat4_identity(drawable.model_matrix);

    // Create empty buffers for the drawable
    drawable.buffers = buffers_create_empty();
    buffers_bind_vao(drawable.buffers.VAO);

    // Load vertex positions (VBO)
    drawable.buffers.VBO = buffers_create_vbo(mesh->vertices, mesh->vertex_count * 3);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    // Load normals (if available)
    if (mesh->normals) {
        drawable.buffers.NormalVBO = buffers_create_vbo(mesh->normals, mesh->vertex_count * 3);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(1);
    }

    // Load texture coordinates (if available)
    if (mesh->texcoords) {
        drawable.buffers.TexCoordVBO = buffers_create_vbo(mesh->texcoords, mesh->vertex_count * 2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(2);
    }

    // Load indices (if available)
    if (mesh->indices) {
        drawable.buffers.EBO = buffers_create_ebo(mesh->indices, mesh->index_count);
    }

	drawable.mesh = mesh;

    // Unbind VAO after setup
    glBindVertexArray(0);

    // Set the drawable to the pointer passed in
    *p_drawable = drawable;
}

void draw_manager_draw(Drawable* drawable) {
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

void draw_manager_translate(Drawable* drawable, vec3 translation) {
    mat4 translation_matrix;
    glm_translate_make(translation_matrix, translation);
    glm_mat4_mul(translation_matrix, drawable->model_matrix, drawable->model_matrix);
}

void draw_manager_scale(Drawable* drawable, vec3 scale) {
    mat4 scale_matrix;
    glm_scale_make(scale_matrix, scale);
    glm_mat4_mul(scale_matrix, drawable->model_matrix, drawable->model_matrix);
}

void draw_manager_rotate(Drawable* drawable, float angle, vec3 axis) {
    mat4 rotation_matrix;
    glm_rotate_make(rotation_matrix, glm_rad(angle), axis);
    glm_mat4_mul(rotation_matrix, drawable->model_matrix, drawable->model_matrix);
}

void draw_manager_destroy(Drawable* drawable) {
    buffers_destroy(&drawable->buffers);
}
