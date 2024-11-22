#include <entities/mesh.h>
#include <stdlib.h>
#include <string.h>

Mesh *mesh_create() {
    Mesh *mesh = (Mesh *)malloc(sizeof(Mesh));
    mesh->vertices = NULL;
    mesh->normals = NULL;
    mesh->texcoords = NULL;
    mesh->vertex_count = 0;
    mesh->indices = NULL;
    mesh->index_count = 0;
    glm_vec3_zero(mesh->min_bound);
    glm_vec3_zero(mesh->max_bound);
    return mesh;
}

void mesh_free(Mesh *mesh) {
    if (mesh) {
        free(mesh->vertices);
        free(mesh->normals);
        free(mesh->texcoords);
        free(mesh->indices);
        free(mesh);
    }
}