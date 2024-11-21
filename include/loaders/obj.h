#ifndef OBJ_H
#define OBJ_H

#include <cglm/cglm.h>

typedef struct {
    float *positions;   // Vertex positions (x, y, z)
    float *normals;     // Normals (x, y, z)
    float *texcoords;   // Texture coordinates (u, v)
    unsigned int *indices;  // Indices (for faces)
    size_t vertex_count;
    size_t index_count;
    size_t texcoords_count;
    size_t normals_count;
} StaticMesh;

int load_obj(const char *filename, StaticMesh *mesh);

#endif // OBJ_H