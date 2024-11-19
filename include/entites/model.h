#ifndef MODEL_H
#define MODEL_H

#include <cglm/cglm.h>
#include <entites/mesh.h>
#include <loaders/obj.h>

// Structure to hold model data (vertices, normals, texcoords, indices, and transformations)
typedef struct {
    Mesh *mesh;                  // Pointer to the mesh object
    float *vertices;             // Vertex positions (x, y, z)
    float *normals;              // Normal vectors (x, y, z)
    float *texcoords;            // Texture coordinates (u, v)
    unsigned int *indices;       // Indices for drawing the model
    int vertex_count;            // Number of vertices
    int normal_count;            // Number of normals
    int texcoord_count;          // Number of texture coordinates
    int index_count;             // Number of indices
    char *textureFile;           // Texture file path (if applicable)

    // Transformation data
    vec3 translation;            // Translation vector
    vec3 rotationAxis;           // Rotation axis
    float rotationAngle;         // Rotation angle (in degrees)
    vec3 scale;                  // Scale vector
} Model;

// Function to load the OBJ model from a file
int load_model(const char* filename, const char* texture_path, Model* model, int program_id);

// Function to free the memory allocated for the model
void free_model(Model *model);

// Function to render the model using the specified view and projection matrices
void render_model(Model *model, mat4 viewMatrix, mat4 projectionMatrix);

// Function to generate default UVs for a model (cube mapping or simple mapping)
void generate_default_uvs(float *vertices, int vertexCount, float *texcoords);

// Apply translation to the model
void model_translate(Model *model, vec3 translation);

// Apply rotation to the model
void model_rotate(Model *model, float angle, vec3 axis);

// Apply scaling to the model
void model_scale(Model *model, vec3 scale);

#endif // MODEL_H