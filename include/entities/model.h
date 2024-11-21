#ifndef MODEL_H
#define MODEL_H

#include <entities/mesh.h>
#include <stdint.h>

// Model stores multiple meshes
typedef struct {
    Mesh **meshes;       // Array of meshes
    uint32_t mesh_count; // Number of meshes
} Model;

int model_load_gltf(Model* model, const char *file_path);
void model_free(Model *model);

#endif // MODEL_H
