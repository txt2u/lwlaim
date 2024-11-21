#ifndef MESH_H
#define MESH_H

#include <cglm/cglm.h>
#include <stdint.h>

typedef struct {
    float *vertices;         // Vertex positions
    float *normals;          // Vertex normals
    float *texcoords;        // Texture coordinates
    uint32_t vertex_count;   // Number of vertices
    uint32_t *indices;       // Indices for drawing
    uint32_t index_count;    // Number of indices
    vec3 min_bound;          // Bounding box minimum
    vec3 max_bound;          // Bounding box maximum
} Mesh;

Mesh *mesh_create();
void mesh_free(Mesh *mesh);

#endif // MESH_H
