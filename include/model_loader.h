#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <stdio.h>

// Structure to hold model data (vertices, normals, texcoords, indices)
typedef struct {
    float *vertices;      // Vertex positions (x, y, z)
    float *normals;       // Normal vectors (x, y, z)
    float *texcoords;     // Texture coordinates (u, v)
    unsigned int *indices; // Indices for drawing the model
    int vertex_count;      // Number of vertices
    int normal_count;      // Number of normals
    int texcoord_count;    // Number of texture coordinates
    int index_count;       // Number of indices
} Model;

// Function to load the OBJ file and return whether it was successful
int load_obj(const char *filename, Model *model);

// Function to free the model's allocated memory
void free_model(Model *model);

// Function to generate default UVs (for simple cube mapping)
void generate_default_uvs(float *vertices, int vertex_count, float *texcoords);

// Helper function to load the material file (MTL)
void load_mtl(const char *filename);

// Helper function to extract the directory part of a file path
void extract_directory(const char *path, char *dir);

#endif // MODEL_LOADER_H
