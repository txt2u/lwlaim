#ifndef MODEL_H
#define MODEL_H

#include <entities/mesh.h>
#include <stdint.h>
#include <cglm/cglm.h>

// Model stores multiple meshes and their transformations
typedef struct {
    Mesh **meshes;            // Array of meshes
    uint32_t mesh_count;      // Number of meshes
    int texture_id;           // Texture ID for the model

    vec3 position;            // Position of the model
    vec3 scale;               // Scale of the model
    versor rotation;          // Rotation of the model as a quaternion (w, x, y, z)
    mat4 transform_matrix;    // Final transformation matrix (position + scale + rotation)
} Model;

// Load a model from a glTF file and set the texture
int model_load_gltf(Model *model, const char *texture_path, const char *file_path, bool apply_parent_transform);

// Set the position of the model
void model_set_position(Model *model, vec3 new_position);

// Set the scale of the model
void model_set_scale(Model *model, vec3 new_scale);

// Set the rotation of the model using a quaternion (w, x, y, z)
void model_set_rotation(Model *model, versor rotation);

// Create a rotation quaternion based on axis and angle
void create_rotation_quaternion(versor q, vec3 axis, float angle);

// Apply the current transformation matrix to the model (position + scale + rotation)
void model_apply_transform(Model *model);

// Free the model and all its meshes
void model_free(Model *model);

#endif // MODEL_H
