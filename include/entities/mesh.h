#ifndef MESH_H
#define MESH_H

#include <entities/material.h>
#include <cglm/cglm.h>
#include <stdint.h>


typedef struct {
    char *name;              // Name of the mesh
    float *vertices;         // Vertex positions
    float *normals;          // Vertex normals
    float *texcoords;        // Texture coordinates
    uint32_t vertex_count;   // Number of vertices
    uint32_t *indices;       // Indices for drawing
    uint32_t index_count;    // Number of indices
    vec3 min_bound;          // Bounding box minimum
    vec3 max_bound;          // Bounding box maximum

    vec3 position;           // Position of the mesh
    vec3 scale;              // Scale of the mesh
    versor rotation;           // Rotation of the mesh in Euler angles (x, y, z)

    mat4 transform_matrix;   // Combined transformation matrix (Position, Rotation, Scale)

	Material *material;
} Mesh;

Mesh *mesh_create(const char *name); // Constructor with name initialization
void mesh_free(Mesh *mesh);          // Cleanup function
void mesh_set_material(Mesh *mesh, Material *material);
void mesh_update_transform_matrix(Mesh *mesh);

#endif // MESH_H